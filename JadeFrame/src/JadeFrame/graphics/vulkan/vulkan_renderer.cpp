#include "pch.h"

#include "vulkan_renderer.h"
#if defined(_WIN32)
#include "JadeFrame/platform/windows/windows_window.h"
#elif defined(__linux__)
#include "JadeFrame/platform/linux/linux_window.h"
#endif

#include "vulkan_shader.h"

#include "../graphics_shared.h"
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {


static const i32 MAX_FRAMES_IN_FLIGHT = 1;

Vulkan_Renderer::Vulkan_Renderer(RenderSystem& system, const IWindow* window)
    : m_context(window) {
    m_system = &system;
    m_logical_device = &m_context.m_instance.m_logical_device;

    // Swapchain stuff
    m_swapchain = m_logical_device->create_swapchain(m_context.m_instance.m_surface);
    const u32 swapchain_image_amount = static_cast<u32>(m_swapchain.m_images.size());

    m_render_pass = m_logical_device->create_render_pass(m_swapchain.m_image_format);

    m_framebuffers.resize(swapchain_image_amount);
    for (size_t i = 0; i < swapchain_image_amount; i++) {
        m_framebuffers[i] =
            m_logical_device->create_framebuffer(m_swapchain.m_image_views[i], m_render_pass, m_swapchain.m_extent);
    }

    /*
        The part below should probably be somewhere else, as they are highly dependent on the shader code.
        One has to find a way to make it more dynamic.
    */

    // Uniform stuff
    m_ub_tran = m_logical_device->create_buffer(vulkan::Buffer::TYPE::UNIFORM, nullptr, sizeof(Matrix4x4));
    m_ub_cam = m_logical_device->create_buffer(vulkan::Buffer::TYPE::UNIFORM, nullptr, sizeof(Matrix4x4));

    m_frames.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) { m_frames[i].init(m_logical_device); }
}
auto Vulkan_Renderer::set_clear_color(const RGBAColor& color) -> void { m_clear_color = color; }



auto Vulkan_Renderer::clear_background() -> void {}

auto Vulkan_Renderer::submit(const Object& obj) -> void {
    const vulkan::LogicalDevice& d = *m_logical_device;

    const Vulkan_RenderCommand command = {
        .transform = &obj.m_transform,
        .vertex_data = obj.m_vertex_data,
        .material_handle = obj.m_material_handle,
        .m_GPU_mesh_data_id = obj.m_vertex_data_id,
    };
    m_render_commands.push_back(command);
}


static auto get_aligned_block_size(const u64 block_size, const u64 alignment) -> u64 {
#if 0 // more efficient
        const u64 new_val = (block_size + alignment - 1) & ~(alignment - 1);
        const u64 aligned_block_size = alignment > 0 ? new_val : block_size;
#else // more readable
    const u64 new_val = (block_size + alignment - (block_size % alignment));
    const u64 aligned_block_size = (block_size % alignment == 0) ? block_size : new_val;
#endif
    return aligned_block_size;
}

auto Vulkan_Renderer::recreate_swapchain() -> void {
    vkDeviceWaitIdle(m_logical_device->m_handle);
    m_swapchain.deinit();

    m_swapchain.init(*m_logical_device, m_logical_device->m_instance->m_surface);
}
auto Vulkan_Renderer::cleanup_swapchain() -> void { m_swapchain.deinit(); }


auto Vulkan_Renderer::render(const Matrix4x4& view_projection) -> void {
    vulkan::LogicalDevice&        d = *m_logical_device;
    const vulkan::PhysicalDevice* pd = d.m_physical_device;

    m_frames[m_frame_index].acquire_image(m_swapchain);


    if (m_swapchain.m_is_recreated == true) {
        m_swapchain.m_is_recreated = false;
        return;
    }

    // Per Frame ubo
    m_ub_cam->write(view_projection, 0);

    const u64 dyn_alignment =
        get_aligned_block_size(sizeof(Matrix4x4), pd->query_limits().minUniformBufferOffsetAlignment);

    vulkan::CommandBuffer& cb = m_frames[m_frame_index].m_cmd;
    cb.record([&] {
        vulkan::Framebuffer& framebuffer = m_framebuffers[m_frames[m_frame_index].m_index];
        const RGBAColor      c = m_clear_color;
        const VkClearValue   clear_value = VkClearValue{c.r, c.g, c.b, c.a};

        cb.render_pass(framebuffer, m_render_pass, m_swapchain.m_extent, clear_value, [&] {
            for (u64 i = 0; i < m_render_commands.size(); i++) {
                const auto&          cmd = m_render_commands[i];
                const ShaderHandle&  shader_handle = m_system->m_registered_shaders[cmd.material_handle.m_shader_id];
                Vulkan_Shader*       shader = static_cast<Vulkan_Shader*>(shader_handle.m_handle);
                vulkan::GPUMeshData& gpu_data = m_registered_meshes[cmd.m_GPU_mesh_data_id];

                // Update ubo buffer and descriptor set when the amount of render commands changes
                const size_t num_commands = m_render_commands.size();
                if (num_commands * dyn_alignment != m_ub_tran->m_size) {
                    if (num_commands != 0) {
                        m_ub_tran->resize(num_commands * dyn_alignment);
                        shader->rebind_buffer(3, 0, *m_ub_tran);
                    }
                }

                const u32 dyn_offset = static_cast<u32>(dyn_alignment * i);

                // Per DrawCall ubo
                m_ub_tran->write(*cmd.transform, dyn_offset);


                const VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
                VkDeviceSize              offsets[] = {0, 0};
                cb.bind_pipeline(bind_point, shader->m_pipeline);
                cb.bind_vertex_buffers(0, 1, &gpu_data.m_vertex_buffer->m_handle, offsets);
                cb.bind_descriptor_sets(bind_point, shader->m_pipeline, 0, shader->m_sets[0], &dyn_offset);
                // cb.bind_descriptor_sets(bind_point, shader->m_pipeline, 1, shader->m_sets[1], &dyn_offset);
                cb.bind_descriptor_sets(bind_point, shader->m_pipeline, 2, shader->m_sets[2], &dyn_offset);
                cb.bind_descriptor_sets(bind_point, shader->m_pipeline, 3, shader->m_sets[3], &dyn_offset);


                if (cmd.vertex_data->m_indices.size() > 0) {
                    cb.bind_index_buffer(*gpu_data.m_index_buffer, 0);
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

    VkResult result = m_frames[m_frame_index].present(d.m_present_queue, m_swapchain);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebuffer_resized) {
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


auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {}

auto Vulkan_Renderer::take_screenshot(const char* /*filename*/) -> void { assert(false); }
} // namespace JadeFrame