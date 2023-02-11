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



    // TODO: The below part has to be moved!
    // The descriptor set pool creation has to be done a renderer.
    // The descriptor set layout creation has to be done in a shader.



    // Create main descriptor pool, which should have all kinds of types. In the future maybe make it more specific.
    u32 descriptor_count = 1000;
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_SAMPLER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptor_count});
    m_main_descriptor_pool.init(*m_logical_device, 4);


    /*
        The part below should probably be somewhere else, as they are highly dependent on the shader code.
        One has to find a way to make it more dynamic.
    */

    // Uniform stuff
    m_ub_cam.init(*m_logical_device, vulkan::Buffer::TYPE::UNIFORM, nullptr, sizeof(Matrix4x4));
    m_ub_tran.init(*m_logical_device, vulkan::Buffer::TYPE::UNIFORM, nullptr, sizeof(Matrix4x4));

    m_descriptor_set_layout_global.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    m_descriptor_set_layout_global.init(*m_logical_device);

    m_descriptor_set_layout_draw_call.add_binding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT);
    m_descriptor_set_layout_draw_call.init(*m_logical_device);

    // TODO: Maybe move the descriptor code to `vulkan_renderer.cpp`?


    m_descriptor_sets.resize(2);
    m_descriptor_sets[0] = m_main_descriptor_pool.allocate_descriptor_set(m_descriptor_set_layout_global);
    m_descriptor_sets[0].add_uniform_buffer(0, m_ub_cam, 0, sizeof(Matrix4x4));
    m_descriptor_sets[1] = m_main_descriptor_pool.allocate_descriptor_set(m_descriptor_set_layout_draw_call);
    m_descriptor_sets[1].add_uniform_buffer(0, m_ub_tran, 0, sizeof(Matrix4x4));
    for (int i = 0; i < m_descriptor_sets.size(); i++) { m_descriptor_sets[i].update(); }
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
    m_ub_cam.send(view_projection, 0);

    const u64 aligned_block_size = [&]() {
        const u64 alignment = m_logical_device->m_physical_device->m_properties.limits.minUniformBufferOffsetAlignment;
        const u64 block_size = sizeof(Matrix4x4);
        const u64 aligned_block_size = alignment > 0 ? (block_size + alignment - 1) & ~(alignment - 1) : block_size;
        return aligned_block_size;
    }();

    // Update ubo buffer and descriptor set when the amount of render commands changes
    if (m_render_commands.size() != 0 && m_render_commands.size() * aligned_block_size != m_ub_tran.m_size) {
        m_ub_tran.resize(m_render_commands.size() * aligned_block_size);
        m_descriptor_sets[1].readd_uniform_buffer(0, m_ub_tran);
        for (int i = 0; i < m_descriptor_sets.size(); i++) { m_descriptor_sets[i].update(); }
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
                m_ub_tran.send(transform, offset);


                cb.bind_pipeline(bind_point, pipeline);
                cb.bind_vertex_buffers(0, 1, vertex_buffers, offsets);
                cb.bind_descriptor_sets(bind_point, pipeline, 0, m_descriptor_sets[0], &offset);
                cb.bind_descriptor_sets(bind_point, pipeline, 3, m_descriptor_sets[1], &offset);


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