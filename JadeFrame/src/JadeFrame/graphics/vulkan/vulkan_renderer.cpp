#include "pch.h"

#include "vulkan_renderer.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "vulkan_shader.h"

#include "../graphics_shared.h"
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {


Vulkan_Renderer::Vulkan_Renderer(const Window& window)
    : m_context(window) {}
auto Vulkan_Renderer::set_clear_color(const RGBAColor& color) -> void { m_clear_color = color; }



auto Vulkan_Renderer::clear_background() -> void {}

auto Vulkan_Renderer::submit(const Object& obj) -> void {
    const vulkan::LogicalDevice& d = m_context.m_instance.m_logical_device;

    if (false == obj.m_GPU_mesh_data.m_is_initialized) {
        assert(obj.m_vertex_format.m_attributes.size() > 0);

        obj.m_GPU_mesh_data.m_handle = new vulkan::Vulkan_GPUMeshData(d, *obj.m_vertex_data, obj.m_vertex_format);
        obj.m_GPU_mesh_data.m_is_initialized = true;
    }

    MaterialHandle* mh = obj.m_material_handle;
    ShaderHandle*   sh = mh->m_shader_handle;
    if (mh->m_is_initialized == false) {
        sh->m_api = GRAPHICS_API::VULKAN;

        Vulkan_Shader::DESC shader_desc;
        shader_desc.code = sh->m_code;
        shader_desc.vertex_format = sh->m_vertex_format;
        sh->m_handle = new Vulkan_Shader(d, shader_desc);

        if (mh->m_texture_handle != nullptr) {
            mh->m_texture_handle->m_api = GRAPHICS_API::VULKAN;
            // mh->m_texture_handle->init();
            vulkan::Vulkan_Texture* texture = new vulkan::Vulkan_Texture();
            texture->init(
                d, mh->m_texture_handle->m_data,
                {static_cast<u32>(mh->m_texture_handle->m_size.x), static_cast<u32>(mh->m_texture_handle->m_size.y)},
                VK_FORMAT_R8G8B8A8_SRGB);
            mh->m_texture_handle->m_handle = texture;
        }
        mh->m_is_initialized = true;
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
    vulkan::LogicalDevice& d = m_context.m_instance.m_logical_device;
    const RGBAColor        c = m_clear_color;
    const VkClearValue     clear_value = VkClearValue{c.r, c.g, c.b, c.a};

    vulkan::Fence&     curr_fence = d.m_in_flight_fences[d.m_current_frame];
    vulkan::Semaphore& available_semaphore = d.m_image_available_semaphores[d.m_current_frame];
    vulkan::Semaphore& finished_semaphore = d.m_render_finished_semaphores[d.m_current_frame];

    vulkan::RenderPass& render_pass = d.m_swapchain.m_render_pass;

    const u64 alignment = m_context.m_instance.m_physical_device.m_properties.limits.minUniformBufferOffsetAlignment;
    const u64 block_size = sizeof(Matrix4x4);
    const u64 aligned_block_size = alignment > 0 ? (block_size + alignment - 1) & ~(alignment - 1) : block_size;


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
                const auto& cmd = m_render_commands[i];
                const auto& shader = *static_cast<Vulkan_Shader*>(cmd.material_handle->m_shader_handle->m_handle);
                const auto& gpu_data = *static_cast<vulkan::Vulkan_GPUMeshData*>(cmd.m_GPU_mesh_data->m_handle);
                const auto& vertex_data = *cmd.vertex_data;
                const auto& transform = *cmd.transform;
                VkBuffer    vertex_buffers[] = {gpu_data.m_vertex_buffer.m_handle};
                VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;


                const u32    offset = static_cast<u32>(aligned_block_size * i);
                VkDeviceSize offsets[] = {0, 0};

                // Per DrawCall ubo
                d.m_ub_tran.send(transform, offset);

                vkCmdBindPipeline(
                    cb.m_handle,               // commandBuffer
                    bind_point,                // pipelineBindPoint
                    shader.m_pipeline.m_handle // pipeline
                );

                vkCmdBindVertexBuffers(
                    cb.m_handle,    // commandBuffer
                    0,              // firstBinding
                    1,              // bindingCount
                    vertex_buffers, // pBuffers
                    offsets         // pOffsets
                );

                vkCmdBindDescriptorSets(
                    cb.m_handle,                                      // commandBuffer
                    bind_point,                                       // pipelineBindPoint
                    shader.m_pipeline.m_layout,                       // layout
                    0,                                                // firstSet
                    1,                                                // descriptorSetCount
                    &d.m_descriptor_sets[0].m_handle,                 // pDescriptorSets
                    d.m_descriptor_sets[0].m_layout->m_dynamic_count, // dynamicOffsetCount
                    &offset                                           // pDynamicOffsets
                );

                vkCmdBindDescriptorSets(
                    cb.m_handle,                                      // commandBuffer
                    bind_point,                                       // pipelineBindPoint
                    shader.m_pipeline.m_layout,                       // layout
                    3,                                                // firstSet
                    1,                                                // descriptorSetCount
                    &d.m_descriptor_sets[1].m_handle,                 // pDescriptorSets
                    d.m_descriptor_sets[1].m_layout->m_dynamic_count, // dynamicOffsetCount
                    &offset                                           // pDynamicOffsets
                );




                if (vertex_data.m_indices.size() > 0) {
                    vkCmdBindIndexBuffer(cb.m_handle, gpu_data.m_index_buffer.m_handle, 0, VK_INDEX_TYPE_UINT32);
                    vkCmdDrawIndexed(cb.m_handle, static_cast<u32>(vertex_data.m_indices.size()), 1, 0, 0, 0);
                } else {
                    vkCmdDraw(cb.m_handle, static_cast<u32>(vertex_data.m_positions.size()), 1, 0, 0);
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
    vulkan::LogicalDevice& d = m_context.m_instance.m_logical_device;
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