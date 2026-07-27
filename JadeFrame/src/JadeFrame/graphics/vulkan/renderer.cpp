#include "renderer.h"
#if defined(_WIN32)
    #include "JadeFrame/platform/windows/windows_window.h"
#elif defined(__linux__)
    #include "JadeFrame/platform/linux/linux_window.h"
#endif

#include "JadeFrame/utils/logger.h"
#include "../graphics_shared.h"
#include "shader.h"

#include <limits>

namespace JadeFrame {
// prepare shaders and its dynamic uniform buffers
// TODO: Find a better way to do this, but for now it works
static auto prepare_shaders(const std::deque<RenderCommand>& commands) -> void {
    for (u64 i = 0; i < commands.size(); i++) {
        const auto&           cmd = commands[i];
        const MaterialHandle& mh = *cmd.material;
        auto*                 material = static_cast<Vulkan_Material*>(mh.m_handle.get());
        material->set_dynamic_ub_num(commands.size());
    }
}

static const i32 MAX_FRAMES_IN_FLIGHT = 4;

Vulkan_Renderer::Vulkan_Renderer(RenderSystem& system, Window* window)
    : m_context(window)
    , m_logical_device(&m_context.m_instance.m_logical_device)
    , m_system(&system) {

    // Swapchain stuff
    m_swapchain = m_logical_device->create_swapchain(window);
    const u32 swapchain_image_amount = static_cast<u32>(m_swapchain.m_images.size());

    m_render_pass = m_logical_device->create_render_pass(m_swapchain.m_image_format);

    m_framebuffers.resize(swapchain_image_amount);
    for (size_t i = 0; i < swapchain_image_amount; i++) {
        m_framebuffers[i] = m_logical_device->create_framebuffer(
            m_swapchain.m_image_views[i],
            m_swapchain.m_depth_image_view,
            m_render_pass,
            m_swapchain.m_extent
        );
    }

    m_frames.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_frames[i].init(m_logical_device);
    }
}

Vulkan_Renderer::~Vulkan_Renderer() { this->wait_until_idle(); }

auto Vulkan_Renderer::wait_until_idle() -> void {
    if (m_logical_device == nullptr) { return; }
    m_logical_device->wait_until_idle();
}

auto Vulkan_Renderer::recreate_swapchain() -> void {
    m_logical_device->wait_until_idle();

    m_framebuffers.clear();
    m_swapchain.recreate();
    m_render_pass = m_logical_device->create_render_pass(m_swapchain.m_image_format);

    const u32 swapchain_image_amount = static_cast<u32>(m_swapchain.m_images.size());
    m_framebuffers.resize(swapchain_image_amount);
    for (size_t i = 0; i < swapchain_image_amount; i++) {
        m_framebuffers[i] = m_logical_device->create_framebuffer(
            m_swapchain.m_image_views[i],
            m_swapchain.m_depth_image_view,
            m_render_pass,
            m_swapchain.m_extent
        );
    }
}

auto Vulkan_Renderer::set_clear_color(const RGBAColor& color) -> void {
    m_clear_color = color;
}

auto Vulkan_Renderer::clear_background() -> void {}

auto Vulkan_Renderer::render(const Camera& camera) -> void {
    vulkan::LogicalDevice&        d = *m_logical_device;

    auto& curr_frame = m_frames[m_frame_index];
    curr_frame.acquire_image(m_swapchain);

    if (m_swapchain.m_is_recreated) {
        m_swapchain.m_is_recreated = false;
        this->recreate_swapchain();
        m_skip_present = true;
        return;
    }

    std::deque<RenderCommand>& render_commands = m_system->m_render_commands;
    // prepare shaders and its dynamic uniform buffers
    // TODO: Find a better way to do this, but for now it works
    for (u64 i = 0; i < render_commands.size(); i++) {
        const auto&           cmd = render_commands[i];
        const MaterialHandle& mh = *cmd.material;
        auto*                 material = static_cast<Vulkan_Material*>(mh.m_handle.get());
        material->set_dynamic_ub_num(render_commands.size());
    }

    vulkan::CommandBuffer& cb = curr_frame.m_cmd;
    // cb.record([&] {
    cb.record_begin();
    vulkan::Framebuffer& framebuffer = m_framebuffers[curr_frame.m_index];
    const RGBAColor      c = m_clear_color;
    const VkClearValue   clear_value = VkClearValue{{{c.r, c.g, c.b, c.a}}};

    // cb.render_pass(framebuffer, m_render_pass, m_swapchain.m_extent, clear_value, [&] {
    cb.render_pass_begin(framebuffer, m_render_pass, m_swapchain.m_extent, clear_value);
    for (u64 i = 0; i < render_commands.size(); i++) {
        using namespace vulkan;
        const RenderCommand& cmd = render_commands[i];
        MaterialHandle&      mh = *cmd.material;
        auto*                material = static_cast<Vulkan_Material*>(mh.m_handle.get());

        const VkPipelineBindPoint bp = VK_PIPELINE_BIND_POINT_GRAPHICS;
        vulkan::Pipeline&         pl = material->m_shader->m_pipeline;
        auto&                     sets = material->m_sets;
        if (!pl.m_is_compiled) {
            Logger::err("Skipping a render command with an uncompiled Vulkan pipeline");
            continue;
        }
        cb.bind_pipeline(bp, pl);

        const auto* bg_cam = mh.m_info.get_bind_group_by_name("Camera");
        const auto* bg_tran = mh.m_info.get_bind_group_by_name("Transform");
        if (bg_cam == nullptr || bg_tran == nullptr) {
            Logger::err("Material is missing required bind groups.");
            continue;
        }
        if (bg_cam->m_set != static_cast<u32>(vulkan::FREQUENCY::PER_FRAME) ||
            bg_tran->m_set != static_cast<u32>(vulkan::FREQUENCY::PER_OBJECT)) {
            Logger::err("Camera and Transform bind groups use incompatible descriptor sets");
            continue;
        }

        mat4x4 cam = camera.get_view_projection("Vulkan");
        auto   cam_set = static_cast<FREQUENCY>(bg_cam->m_set);
        auto   cam_binding = bg_cam->m_binding;
        material->write_ub(cam_set, cam_binding, &cam, sizeof(cam), 0);

        const auto& tran = cmd.transform;
        auto        tran_set = static_cast<FREQUENCY>(bg_tran->m_set);
        auto        tran_binding = bg_tran->m_binding;
        const VkDeviceSize transform_stride =
            material->get_dynamic_uniform_stride(bg_tran->m_set, bg_tran->m_binding);
        const u64 transform_offset = transform_stride * i;
        if (transform_stride == 0 || transform_offset > std::numeric_limits<u32>::max()) {
            Logger::err("Transform binding is not a valid dynamic uniform buffer");
            continue;
        }
        const u32 dyn_offset = static_cast<u32>(transform_offset);
        material->write_ub(tran_set, tran_binding, &tran, sizeof(tran), dyn_offset);

        for (u32 set_index = 0; set_index < pl.m_set_layouts.size(); ++set_index) {
            if (set_index >= sets.size()) {
                Logger::err("Material is missing descriptor set {}", set_index);
                continue;
            }

            std::vector<u32> dynamic_offsets;
            const auto&      set_layout = pl.m_set_layouts[set_index];
            dynamic_offsets.reserve(set_layout.m_dynamic_count);
            bool valid_dynamic_bindings = true;
            for (const auto& binding : set_layout.m_bindings) {
                if (binding.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC &&
                    binding.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
                    continue;
                }

                const VkDeviceSize stride =
                    material->get_dynamic_uniform_stride(set_index, binding.binding);
                const u64 offset = stride * i;
                if (stride == 0 || offset > std::numeric_limits<u32>::max()) {
                    Logger::err(
                        "Missing or invalid dynamic uniform binding {} in descriptor set {}",
                        binding.binding,
                        set_index
                    );
                    valid_dynamic_bindings = false;
                    break;
                }
                dynamic_offsets.push_back(static_cast<u32>(offset));
            }
            if (!valid_dynamic_bindings) { continue; }
            cb.bind_descriptor_set(bp, pl, set_index, sets[set_index], dynamic_offsets);
        }

        this->render_mesh(cmd.vertex_data, cmd.m_mesh);
    }
    //});
    cb.render_pass_end();
    //});
    cb.record_end();

    curr_frame.submit(d.m_graphics_queue);

    render_commands.clear();
}

auto Vulkan_Renderer::render_mesh(const Mesh* vertex_data, const GPUMeshData* gpu_data)
    -> void {
    // const auto& s = vertex_data->m_attributes.at(Mesh::POSITION.m_id);
    const auto& position_attribute = vertex_data->m_attributes.at(Mesh::POSITION.m_id);
    const u32   num_vertices = static_cast<u32>(
        position_attribute.m_data.size() / component_count(Mesh::POSITION.m_format)
    );
    const auto& num_indices = static_cast<u32>(vertex_data->m_indices.size());

    const vulkan::Buffer* vertex_buffer =
        static_cast<vulkan::Buffer*>(gpu_data->m_vertex_buffer->m_handle);

    vulkan::CommandBuffer& cb = m_frames[m_frame_index].m_cmd;
    cb.bind_vertex_buffer(0, *vertex_buffer, 0);

    if (!vertex_data->m_indices.empty()) {
        const vulkan::Buffer* index_buffer =
            static_cast<vulkan::Buffer*>(gpu_data->m_index_buffer->m_handle);
        cb.bind_index_buffer(*index_buffer, 0);
        cb.draw_indexed(num_indices, 1, 0, 0, 0);
    } else {
        cb.draw(num_vertices, 1, 0, 0);
    }
}

auto Vulkan_Renderer::present() -> void {
    if (m_skip_present) {
        m_skip_present = false;
        return;
    }

    vulkan::LogicalDevice& d = *m_logical_device;
    (void)d;

    VkResult result =
        m_frames[m_frame_index].present(m_swapchain.m_present_queue, m_swapchain);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_framebuffer_resized) {
        m_framebuffer_resized = false;
        Logger::debug("recreate because of vkQueuePresentKHR");
        this->recreate_swapchain();
    } else if (result != VK_SUCCESS) {
        Logger::err("failed to present swap chain image!");
        assert(false);
    }

    // auto max_frames_in_flight = d.m_swapchain.m_images.size() - 1;
    m_frame_index = (m_frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

auto Vulkan_Renderer::
    set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {}

auto Vulkan_Renderer::take_screenshot(const char* /*filename*/) -> Image {
    assert(false);
    return Image();
}
} // namespace JadeFrame
