#include "pch.h"
#include "vulkan_logical_device.h"
#include "vulkan_command_buffers.h"
#include "vulkan_command_pool.h"
#include "vulkan_swapchain.h"
#include "vulkan_render_pass.h"
#include "vulkan_pipeline.h"
#include "vulkan_descriptor_set.h"

namespace JadeFrame {
auto VulkanCommandBuffers::init(
	const VulkanLogicalDevice& device,
	const VulkanCommandPool& command_pool,
	const size_t amount
) -> void {
	VkResult result;
	m_device = &device;
	m_command_pool = &command_pool;
	m_handles.resize(amount);

	const VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = command_pool.m_handle,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<u32>(m_handles.size()),
	};
	result = vkAllocateCommandBuffers(device.m_handle, &alloc_info, m_handles.data());
	if (result != VK_SUCCESS) __debugbreak();
}

auto VulkanCommandBuffers::deinit() -> void {
	vkFreeCommandBuffers(m_device->m_handle, m_command_pool->m_handle, static_cast<uint32_t>(m_handles.size()), m_handles.data());

}

auto VulkanCommandBuffers::record(size_t index, std::function<void()> func) -> void {
	VkResult result;
	const VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = {},
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = {},
	};

	result = vkBeginCommandBuffer(m_handles[index], &begin_info);
	if (result != VK_SUCCESS) __debugbreak();

	func();


	result = vkEndCommandBuffer(m_handles[index]);
	if (result != VK_SUCCESS) __debugbreak();



}

auto VulkanCommandBuffers::draw_into(
	const VulkanRenderPass& render_pass, 
	const VulkanSwapchain& swapchain, 
	const VulkanPipeline& pipeline, 
	const VulkanDescriptorSets& descriptor_sets, 
	const VulkanBuffer& vertex_buffer, 
	const VulkanBuffer& index_buffer, 
	const std::vector<u32>& indices, 
	const VkClearValue color_value
) -> void {
	VkResult result;

	for (size_t i = 0; i < m_handles.size(); i++) {

		this->record(i,
			[&]() {
				//const VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
				const VkClearValue clear_color = color_value;
				const VkRenderPassBeginInfo render_pass_info = {
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass = render_pass.m_handle,
					.framebuffer = swapchain.m_framebuffers[i],
					.renderArea = {
						.offset = {0, 0},
						.extent = swapchain.m_extent
					},
					.clearValueCount = 1,
					.pClearValues = &clear_color,
				};

				vkCmdBeginRenderPass(m_handles[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
				{
					vkCmdBindPipeline(m_handles[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_graphics_pipeline);
					VkBuffer vertex_buffers[] = { vertex_buffer.m_buffer };
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(m_handles[i], 0, 1, vertex_buffers, offsets);
					//vkCmdDraw(m_command_buffers[i], static_cast<u32>(g_vertices.size()), 1, 0, 0);

					vkCmdBindIndexBuffer(m_handles[i], index_buffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
					vkCmdBindDescriptorSets(m_handles[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_pipeline_layout, 0, 1, &descriptor_sets.m_descriptor_sets[i], 0, nullptr);
					vkCmdDrawIndexed(m_handles[i], indices.size(), 1, 0, 0, 0);
					if(indices.size() > 0) {
					
					} else {
						//vkCmdDraw(m_handles[i], static_cast<u32>(g_vertices.size()), 1, 0, 0);
					}
				}
				vkCmdEndRenderPass(m_handles[i]);
			}
		);
	}
}

}
