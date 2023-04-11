#include "pch.h"

#include "vulkan_renderer.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "vulkan_shader.h"

#include "../graphics_shared.h"
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {


static const i32 MAX_FRAMES_IN_FLIGHT = 1;

Vulkan_Renderer::Vulkan_Renderer(RenderSystem& system, const IWindow* window)
    : m_context(window) {
    m_system = &system;
    m_logical_device = &m_context.m_instance.m_logical_device;


    // Create main descriptor pool, which should have all kinds of types. In the future maybe make it more specific.
    u32                               descriptor_count = 1000;
    std::vector<VkDescriptorPoolSize> pool_sizes = {
        {        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptor_count},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptor_count},
        {  VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptor_count},
        {        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptor_count},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descriptor_count},
        {  VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptor_count},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor_count},
        {               VK_DESCRIPTOR_TYPE_SAMPLER, descriptor_count},
        {         VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptor_count},
        {         VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptor_count},
    };
    m_set_pool = m_logical_device->create_descriptor_pool(4, pool_sizes);


    /*
        The part below should probably be somewhere else, as they are highly dependent on the shader code.
        One has to find a way to make it more dynamic.
    */

    // Uniform stuff
    m_ub_tran = m_logical_device->create_buffer(vulkan::Buffer::TYPE::UNIFORM, nullptr, sizeof(Matrix4x4));
    m_ub_cam = m_logical_device->create_buffer(vulkan::Buffer::TYPE::UNIFORM, nullptr, sizeof(Matrix4x4));

    // Sync objects stuff
    m_frames.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) { m_frames[i].init(m_logical_device); }
}
auto Vulkan_Renderer::set_clear_color(const RGBAColor& color) -> void { m_clear_color = color; }



auto Vulkan_Renderer::clear_background() -> void {}

auto Vulkan_Renderer::submit(const Object& obj) -> void {
    const vulkan::LogicalDevice& d = *m_logical_device;

    if (false == obj.m_GPU_mesh_data.m_is_initialized) {
        assert(obj.m_vertex_format.m_attributes.size() > 0);

        obj.m_GPU_mesh_data.m_handle = new vulkan::GPUMeshData(d, *obj.m_vertex_data, obj.m_vertex_format);
        obj.m_GPU_mesh_data.m_is_initialized = true;
    }

    const Vulkan_RenderCommand command = {
        .transform = &obj.m_transform,
        .vertex_data = obj.m_vertex_data,
        .material_handle = obj.m_material_handle,
        .m_GPU_mesh_data = &obj.m_GPU_mesh_data,
    };
    m_render_commands.push_back(command);
}


auto get_aligned_block_size(const u64 block_size, const u64 alignment) -> u64 {
#if 0 // more efficient
        const u64 new_val = (block_size + alignment - 1) & ~(alignment - 1);
        const u64 aligned_block_size = alignment > 0 ? new_val : block_size;
#else // more readable
    const u64 new_val = (block_size + alignment - (block_size % alignment));
    const u64 aligned_block_size = (block_size % alignment == 0) ? block_size : new_val;
#endif
    return aligned_block_size;
}

auto Vulkan_Renderer::render(const Matrix4x4& view_projection) -> void {
    vulkan::LogicalDevice&        d = *m_logical_device;
    const vulkan::PhysicalDevice* pd = d.m_physical_device;

    m_frames[m_frame_index].acquire_image(d.m_swapchain);


    if (d.m_swapchain.m_is_recreated == true) {
        d.m_swapchain.m_is_recreated = false;
        return;
    }

    // Per Frame ubo
    m_ub_cam.write(view_projection, 0);

    const u64 dyn_alignment =
        get_aligned_block_size(sizeof(Matrix4x4), pd->query_limits().minUniformBufferOffsetAlignment);

    vulkan::CommandBuffer& cb = m_frames[m_frame_index].m_cmd;
    cb.record([&] {
        vulkan::Framebuffer& framebuffer = d.m_framebuffers[m_frames[m_frame_index].m_index];
        const RGBAColor      c = m_clear_color;
        const VkClearValue   clear_value = VkClearValue{c.r, c.g, c.b, c.a};

        cb.render_pass(framebuffer, d.m_render_pass, d.m_swapchain.m_extent, clear_value, [&] {
            for (u64 i = 0; i < m_render_commands.size(); i++) {
                const auto&         cmd = m_render_commands[i];
                const ShaderHandle& shader_handle = m_system->m_registered_shaders[cmd.material_handle.m_shader_id];
                Vulkan_Shader*      shader = static_cast<Vulkan_Shader*>(shader_handle.m_handle);
                const auto&         gpu_data = *static_cast<vulkan::GPUMeshData*>(cmd.m_GPU_mesh_data->m_handle);


                VkBuffer                  vertex_buffers[] = {gpu_data.m_vertex_buffer.m_handle};
                const VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
                const u32                 offset = static_cast<u32>(dyn_alignment * i);
                VkDeviceSize              offsets[] = {0, 0};

                // Update ubo buffer and descriptor set when the amount of render commands changes
                const size_t num_commands = m_render_commands.size();
                if (num_commands * dyn_alignment != m_ub_tran.m_size) {
                    if (num_commands != 0) {
                        m_ub_tran.resize(num_commands * dyn_alignment);
                        shader->rebind_buffer(3, 0, m_ub_tran);
                    }
                }

                // Per DrawCall ubo
                m_ub_tran.write(*cmd.transform, offset);


                cb.bind_pipeline(bind_point, shader->m_pipeline);
                cb.bind_vertex_buffers(0, 1, vertex_buffers, offsets);
                cb.bind_descriptor_sets(bind_point, shader->m_pipeline, 0, shader->m_sets[0], &offset);
                // cb.bind_descriptor_sets(bind_point, shader->m_pipeline, 1, shader->m_sets[1], &offset);
                // cb.bind_descriptor_sets(bind_point, shader->m_pipeline, 2, shader->m_sets[2], &offset);
                cb.bind_descriptor_sets(bind_point, shader->m_pipeline, 3, shader->m_sets[3], &offset);


                if (cmd.vertex_data->m_indices.size() > 0) {
                    cb.bind_index_buffer(gpu_data.m_index_buffer, 0);
                    cb.draw_indexed(static_cast<u32>(cmd.vertex_data->m_indices.size()), 1, 0, 0, 0);
                } else {
                    cb.draw(static_cast<u32>(cmd.vertex_data->m_positions.size()), 1, 0, 0);
                }
            }
        });
    });

    m_frames[m_frame_index].submit(d.m_graphics_queue);


    m_render_commands.clear();
}

auto Vulkan_Renderer::present() -> void {
    vulkan::LogicalDevice& d = *m_logical_device;

    VkResult result = m_frames[m_frame_index].present(d.m_present_queue, d.m_swapchain);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || d.m_framebuffer_resized) {
        d.m_framebuffer_resized = false;
        Logger::debug("recreate because of vkQueuePresentKHR");
        d.m_swapchain.recreate();
    } else if (result != VK_SUCCESS) {
        Logger::err("failed to present swap chain image!");
        assert(false);
    }

    // auto max_frames_in_flight = d.m_swapchain.m_images.size() - 1;
    m_frame_index = (m_frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}


auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {}

auto Vulkan_Renderer::take_screenshot(const char* /*filename*/) -> void { assert(false); }
} // namespace JadeFrame