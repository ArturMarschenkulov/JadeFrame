#include "pch.h"
#include "vulkan_render_pass.h"

#include "vulkan_logical_device.h"

namespace JadeFrame {

auto VulkanRenderPass::init(const VulkanLogicalDevice& device, VkFormat image_format) -> void {
	m_device = &device;
	VkResult result;

	const VkAttachmentDescription color_attachment = {
		.flags = {},
		.format = image_format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	const VkAttachmentReference color_attachment_ref = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	const VkSubpassDescription subpass = {
		.flags = {},
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount = {},
		.pInputAttachments = {},
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_ref,
		.pResolveAttachments = {},
		.pDepthStencilAttachment = {},
		.preserveAttachmentCount = {},
		.pPreserveAttachments = {},
	};

	const VkRenderPassCreateInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.attachmentCount = 1,
		.pAttachments = &color_attachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = {},
		.pDependencies = {},
	};

	result = vkCreateRenderPass(device.m_handle, &render_pass_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();

}
auto VulkanRenderPass::deinit() -> void {
	vkDestroyRenderPass(m_device->m_handle, m_handle, nullptr);
}
}