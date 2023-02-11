#include "pch.h"

#include "vulkan_renderer.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "vulkan_shader.h"

#include "../graphics_shared.h"
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {


Vulkan_Renderer::Vulkan_Renderer(RenderSystem& system, const IWindow* window)
    : m_context(window) {
    m_system = &system;
    m_logical_device = &m_context.m_instance.m_logical_device;
}
auto Vulkan_Renderer::set_clear_color(const RGBAColor& color) -> void { m_clear_color = color; }



auto Vulkan_Renderer::clear_background() -> void {}

auto Vulkan_Renderer::submit(const Object& obj) -> void {
    const vulkan::LogicalDevice& d = *m_logical_device;

    if (false == obj.m_GPU_mesh_data.m_is_initialized) {
        assert(obj.m_vertex_format.m_attributes.size() > 0);

        obj.m_GPU_mesh_data.m_handle = new vulkan::Vulkan_GPUMeshData(d, *obj.m_vertex_data, obj.m_vertex_format);
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

auto Vulkan_Renderer::render(const Matrix4x4& view_projection) -> void {

    Logger::info("start of frame rendering");
    vulkan::LogicalDevice& d = *m_logical_device;
    const RGBAColor        c = m_clear_color;
    const VkClearValue     clear_value = VkClearValue{c.r, c.g, c.b, c.a};

    vulkan::Fence&     curr_fence = d.m_in_flight_fences[d.m_current_frame];
    vulkan::Semaphore& available_semaphore = d.m_image_available_semaphores[d.m_current_frame];
    vulkan::Semaphore& finished_semaphore = d.m_render_finished_semaphores[d.m_current_frame];

    vulkan::RenderPass& render_pass = d.m_swapchain.m_render_pass;

    d.wait_for_fence(curr_fence, VK_TRUE, UINT64_MAX);
    d.m_present_image_index = d.m_swapchain.acquire_next_image(&available_semaphore, nullptr);

    if (d.m_swapchain.m_is_recreated == true) {
        d.m_swapchain.m_is_recreated = false;
        return;
    }
    // const VulkanImage& image = d.m_swapchain.m_images[image_index];
    vulkan::CommandBuffer& cb = d.m_command_buffers[d.m_present_image_index];
    vulkan::Framebuffer&   framebuffer = d.m_swapchain.m_framebuffers[d.m_present_image_index];

    // Per Frame ubo
    d.m_ub_cam.send(view_projection, 0);

    const u64 aligned_block_size = [&]() {
        const u64 alignment = m_logical_device->m_physical_device->m_properties.limits.minUniformBufferOffsetAlignment;
        const u64 block_size = sizeof(Matrix4x4);
        const u64 aligned_block_size = alignment > 0 ? (block_size + alignment - 1) & ~(alignment - 1) : block_size;
        return aligned_block_size;
    }();
    
    // Update ubo buffer and descriptor set when the amount of render commands changes
    if (m_render_commands.size() != 0 && m_render_commands.size() * aligned_block_size != d.m_ub_tran.m_size) {
        d.m_ub_tran.resize(m_render_commands.size() * aligned_block_size);
        d.m_descriptor_sets[1].readd_uniform_buffer(0, d.m_ub_tran);
        for (int i = 0; i < d.m_descriptor_sets.size(); i++) { d.m_descriptor_sets[i].update(); }
    }

    // cb.record_begin();
    cb.record([&] {
        cb.render_pass(framebuffer, render_pass, d.m_swapchain.m_extent, clear_value, [&] {
            for (u64 i = 0; i < m_render_commands.size(); i++) {
                const auto&           cmd = m_render_commands[i];
                const MaterialHandle& mh = *cmd.material_handle;
                auto&                 sh = m_system->m_registered_shaders[mh.m_shader_id];

                const auto& pipeline = static_cast<Vulkan_Shader*>(sh.m_handle)->m_pipeline;
                const auto& gpu_data = *static_cast<vulkan::Vulkan_GPUMeshData*>(cmd.m_GPU_mesh_data->m_handle);
                const auto& vertex_data = *cmd.vertex_data;
                const auto& transform = *cmd.transform;
                VkBuffer    vertex_buffers[] = {gpu_data.m_vertex_buffer.m_handle};
                const VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
                const u32                 offset = static_cast<u32>(aligned_block_size * i);
                VkDeviceSize              offsets[] = {0, 0};

                // Per DrawCall ubo
                d.m_ub_tran.send(transform, offset);


                cb.bind_pipeline(bind_point, pipeline);
                cb.bind_vertex_buffers(0, 1, vertex_buffers, offsets);
                cb.bind_descriptor_sets(bind_point, pipeline, 0, d.m_descriptor_sets[0], &offset);
                cb.bind_descriptor_sets(bind_point, pipeline, 3, d.m_descriptor_sets[1], &offset);


                if (vertex_data.m_indices.size() > 0) {
                    cb.bind_index_buffer(gpu_data.m_index_buffer, 0);
                    cb.draw_indexed(static_cast<u32>(vertex_data.m_indices.size()), 1, 0, 0, 0);
                } else {
                    cb.draw(static_cast<u32>(vertex_data.m_positions.size()), 1, 0, 0);
                }
            } //
        });
        // cb.render_pass_end();
    });
    // cb.record_end();

    curr_fence.reset();
    d.m_graphics_queue.submit(cb, &available_semaphore, &finished_semaphore, &curr_fence);

    m_render_commands.clear();
    std::cout << "Rendered frame" << std::endl;
}

auto Vulkan_Renderer::present() -> void {
    vulkan::LogicalDevice& d = *m_logical_device;
    vulkan::Semaphore&     finished_semaphore = d.m_render_finished_semaphores[d.m_current_frame];

    VkResult result = d.m_present_queue.present(d.m_present_image_index, d.m_swapchain, &finished_semaphore);
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || d.m_framebuffer_resized) {
            d.m_framebuffer_resized = false;
            std::cout << "recreate because of vkQueuePresentKHR" << std::endl;
            //__debugbreak();
            // d.recreate_swapchain();
            d.m_swapchain.recreate();
        } else if (result != VK_SUCCESS) {
            std::cout << "failed to present swap chain image!" << std::endl;
            assert(false);
        }
    }

    d.m_current_frame = (d.m_current_frame + 1) % 2 /*MAX_FRAMES_IN_FLIGHT*/;
}


auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {}

auto Vulkan_Renderer::take_screenshot(const char* /*filename*/) -> void { assert(false); }
} // namespace JadeFrame