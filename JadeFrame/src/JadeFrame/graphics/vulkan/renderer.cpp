#include "renderer.h"
#if defined(_WIN32)
    #include "JadeFrame/platform/windows/windows_window.h"
#elif defined(__linux__)
    #include "JadeFrame/platform/linux/linux_window.h"
#endif

#include "JadeFrame/utils/logger.h"
#include "../graphics_shared.h"
#include "shader.h"


namespace JadeFrame {
// prepare shaders and its dynamic uniform buffers
// TODO: Find a better way to do this, but for now it works
static auto prepare_shaders(const std::deque<RenderCommand>& commands) -> void {
    for (u64 i = 0; i < commands.size(); i++) {
        const auto&           cmd = commands[i];
        const MaterialHandle& mh = *cmd.material;
        auto*                 material = static_cast<Vulkan_Material*>(mh.m_handle);
        material->set_dynamic_ub_num(commands.size());
    }
}

static const i32 MAX_FRAMES_IN_FLIGHT = 1;

Vulkan_Renderer::Vulkan_Renderer(RenderSystem& system, const Window* window)
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

auto Vulkan_Renderer::set_clear_color(const RGBAColor& color) -> void {
    m_clear_color = color;
}

auto Vulkan_Renderer::clear_background() -> void {}

auto Vulkan_Renderer::render(const mat4x4& view_projection) -> void {
    vulkan::LogicalDevice&        d = *m_logical_device;
    const vulkan::PhysicalDevice* pd = d.m_physical_device;

    auto& curr_frame = m_frames[m_frame_index];
    curr_frame.acquire_image(m_swapchain);

    if (m_swapchain.m_is_recreated) {
        m_swapchain.m_is_recreated = false;
        return;
    }

    const u64 dyn_alignment = math::ceil_to_aligned(
        sizeof(mat4x4), pd->limits().minUniformBufferOffsetAlignment
    );

    std::deque<RenderCommand>& render_commands = m_system->m_render_commands;
    // prepare shaders and its dynamic uniform buffers
    // TODO: Find a better way to do this, but for now it works
    for (u64 i = 0; i < render_commands.size(); i++) {
        const auto&           cmd = render_commands[i];
        const MaterialHandle& mh = *cmd.material;
        auto*                 material = static_cast<Vulkan_Material*>(mh.m_handle);
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
        auto*                material = static_cast<Vulkan_Material*>(mh.m_handle);

        const VkPipelineBindPoint bp = VK_PIPELINE_BIND_POINT_GRAPHICS;
        vulkan::Pipeline&         pipeline = material->m_shader->m_pipeline;
        auto&                     sets = material->m_sets;
        cb.bind_pipeline(bp, pipeline);

        // Per Frame ubo
        // vulkan::FREQUENCY::PER_FRAME == 0
        // Per DrawCall ubo
        // vulkan::FREQUENCY::PER_OBJECT == 3

        material->write_ub(
            FREQUENCY::PER_FRAME, 0, &view_projection, sizeof(view_projection), 0
        );

        const u32 dyn_offset = static_cast<u32>(dyn_alignment * i);
        material->write_ub(
            FREQUENCY::PER_OBJECT, 0, &cmd.transform, sizeof(cmd.transform), dyn_offset
        );

        cb.bind_descriptor_sets(
            bp, pipeline, FREQUENCY::PER_FRAME, sets[FREQUENCY::PER_FRAME], nullptr
        );
        cb.bind_descriptor_sets(
            bp, pipeline, FREQUENCY::PER_PASS, sets[FREQUENCY::PER_PASS], nullptr
        );
        if (mh.m_texture != nullptr) {
            cb.bind_descriptor_sets(
                bp,
                pipeline,
                FREQUENCY::PER_MATERIAL,
                sets[FREQUENCY::PER_MATERIAL],
                nullptr
            );
        }
        cb.bind_descriptor_sets(
            bp, pipeline, FREQUENCY::PER_OBJECT, sets[FREQUENCY::PER_OBJECT], &dyn_offset
        );

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
    const auto& num_vertices =
        static_cast<u32>(vertex_data->m_attributes.at(Mesh::POSITION.m_id).m_data.size());
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
    vulkan::LogicalDevice& d = *m_logical_device;

    VkResult result =
        m_frames[m_frame_index].present(m_swapchain.m_present_queue, m_swapchain);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_framebuffer_resized) {
        m_framebuffer_resized = false;
        Logger::debug("recreate because of vkQueuePresentKHR");
        m_swapchain.recreate();
    } else if (result != VK_SUCCESS) {
        Logger::err("failed to present swap chain image!");
        assert(false);
    }

    // auto max_frames_in_flight = d.m_swapchain.m_images.size() - 1;
    m_frame_index = (m_frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/)
    const -> void {}

auto Vulkan_Renderer::take_screenshot(const char* /*filename*/) -> Image {
    assert(false);
    return Image();
}
} // namespace JadeFrame