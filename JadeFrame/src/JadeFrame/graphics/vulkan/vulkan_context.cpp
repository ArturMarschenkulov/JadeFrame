
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan_context.h"
#include "JadeFrame/defines.h"
#undef min
#undef max


#include <iostream>

#include <set>
#include <JadeFrame/graphics/vulkan/vulkan_debug.h>

const std::vector<const char*> validation_layers = {
	"VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
static const i32 MAX_FRAMES_IN_FLIGHT = 2;
#ifdef NDEBUG
const bool enable_validation_layers = false;
#else
const bool enable_validation_layers = true;
#endif

static auto check_validation_layer_support(const std::vector<VkLayerProperties>& available_layers) -> bool {
	for (u32 i = 0; i < validation_layers.size(); i++) {
		bool layer_found = false;
		for (u32 j = 0; j < available_layers.size(); j++) {
			if (strcmp(validation_layers[i], available_layers[j].layerName) == 0) {
				layer_found = true;
				break;
			}
		}
		if (!layer_found) {
			return false;
		}
	}
	return true;
}


static auto query_swapchain_support(VulkanPhysicalDevice physical_device, VulkanSurface surface) -> SwapChainSupportDetails {
	SwapChainSupportDetails details;
	auto res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device.m_physical_device, surface.m_surface, &details.m_capabilities);
	if (VK_SUCCESS != res) {
		__debugbreak();
	}

	uint32_t format_count;
	if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device.m_physical_device, surface.m_surface, &format_count, nullptr)) {
		__debugbreak();
	}
	if (format_count != 0) {
		details.m_formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device.m_physical_device, surface.m_surface, &format_count, details.m_formats.data());
	}

	uint32_t present_mode_count;
	if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device.m_physical_device, surface.m_surface, &present_mode_count, nullptr)) {
		__debugbreak();
	}
	if (present_mode_count != 0) {
		details.m_present_modes.resize(present_mode_count);
		if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device.m_physical_device, surface.m_surface, &present_mode_count, details.m_present_modes.data())) {
			__debugbreak();

		}
	}
	return details;
}

static auto find_queue_families(VulkanPhysicalDevice physical_device, VulkanSurface surface) -> QueueFamilyIndices {
	QueueFamilyIndices indices;

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device.m_physical_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families;
	queue_families.resize(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device.m_physical_device, &queue_family_count, queue_families.data());

	for (u32 i = 0; i < queue_families.size(); i++) {
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}
		VkBool32 present_support = false;
		if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceSupportKHR(physical_device.m_physical_device, i, surface.m_surface, &present_support)) {
			__debugbreak();
		}

		if (present_support) {
			indices.present_family = i;
		}
		if (indices.is_complete()) {
			break;
		}
	}
	return indices;
}

static auto is_device_suitable(VulkanPhysicalDevice physical_device) -> bool {
	bool swapchain_adequate = false;
	if (physical_device.m_extension_support) {
		swapchain_adequate = 
			!physical_device.m_swapchain_support_details.m_formats.empty() && 
			!physical_device.m_swapchain_support_details.m_present_modes.empty()
		;
	}
	return 
		physical_device.m_queue_family_indices.is_complete() && 
		physical_device.m_extension_support && 
		swapchain_adequate
	;
}

//static auto get_required_instance_extensions(u32* count) -> const char* {
//    const char* extensions[2];
//    extensions[0] = "VK_KHR_surface";
//    extensions[1] = "VK_KHR_win32_surface";
//    return *extensions;
//}

struct VulkanVersion {
	u32 variant;
	u32 major;
	u32 minor;
	u32 patch;
};
static auto vulkan_get_api_version(u32 version) -> VulkanVersion {
	VulkanVersion result;
	result.variant = VK_API_VERSION_VARIANT(version);
	result.major = VK_API_VERSION_MAJOR(version);
	result.minor = VK_API_VERSION_MINOR(version);
	result.patch = VK_API_VERSION_PATCH(version);
	return result;
}
static auto vulkan_get_device_type_string(const VkPhysicalDeviceType& device_type) -> const char* {
	const char* result = "";
	switch (device_type) {
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			result = "VK_PHYSICAL_DEVICE_TYPE_OTHER"; break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_CPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
			__debugbreak();
			result = ""; break;
	}
	return result;
}


static auto choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) -> VkSurfaceFormatKHR {
	for (u32 i = 0; i < available_formats.size(); i++) {
		if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return available_formats[i];
		}
	}

	return available_formats[0];
}
static auto choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) -> VkPresentModeKHR {
	for (u32 i = 0; i < available_present_modes.size(); i++) {
		if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return available_present_modes[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

static auto choose_swap_extent(HWND window_handle, const VkSurfaceCapabilitiesKHR& capabilities) -> VkExtent2D {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		int width, height;
		RECT area;
		GetClientRect(window_handle, &area);
		width = area.right;
		height = area.bottom;
		//glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actual_extent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

		return actual_extent;
	}
}




Vulkan_Context::Vulkan_Context(Windows_Window* window) {
	std::cout << __FUNCTION__ << std::endl;
	if (window->m_is_graphics_api_init == true) {
		if (window->m_graphics_api != Windows_Window::GRAPHICS_API::VULKAN) {
			window->recreate();
		}
	}
	m_window_handle = window->m_window_handle;
	m_instance.init(window->m_window_handle);
	window->m_graphics_api = Windows_Window::GRAPHICS_API::VULKAN;
	this->main_loop();
}

Vulkan_Context::~Vulkan_Context() {
	m_instance.m_device.deinit();
	m_instance.deinit();
}

auto get_queue_families_info(VulkanPhysicalDevice physical_device) -> void {
	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device.m_physical_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families;
	queue_families.resize(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device.m_physical_device, &queue_family_count, queue_families.data());

	for (u32 i = 0; i < queue_families.size(); i++) {


		std::string str = "{ ";
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			str += "Graphics ";
		}
		if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			str += "Compute ";
		}
		if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
			str += "Transfer ";
		}
		if (queue_families[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
			str += "SparseBinding ";
		}
		if (queue_families[i].queueFlags & VK_QUEUE_PROTECTED_BIT) {
			str += "Protected ";
		}
		str += "}";

		std::cout << "Queue familiy " << i << " has " << queue_families[i].queueCount << " queues capable of " << str << std::endl;

	}
}

const char* vs =
R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
)";

const char* fs =
R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
)";

static auto create_shader_module_from_spirv(VkDevice device, const std::vector<u32>& spirv) -> VkShaderModule {
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = spirv.size() * 4; // NOTE: It has to be multiplied by 4!
	create_info.pCode = spirv.data();
	VkShaderModule shader_module;
	if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
		__debugbreak();
	}
	return shader_module;
}
#include <shaderc/shaderc.hpp>

static auto string_to_SPIRV(const char* code, u32 i) -> std::vector<u32> {
	using namespace shaderc;
	Compiler compiler;
	CompileOptions options;
	options.SetOptimizationLevel(shaderc_optimization_level_size);
	SpvCompilationResult result = compiler.CompileGlslToSpv(std::string(code), (shaderc_shader_kind)i /*shaderc_vertex_shader*/, "", options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		__debugbreak();
		return std::vector<u32>();
	}

	std::vector<u32> res = { result.cbegin(), result.cend() };
	return res;
}
#include <thread>
#include <future>
#include <JadeFrame/base_app.h>


auto Vulkan_Context::main_loop() -> void {
	std::cout << __FUNCTION__ << " start" << std::endl;
	while (true) {
		JadeFrame::get_singleton()->m_apps[0]->poll_events();
		m_instance.m_device.draw_frame();
	}
	std::cout << __FUNCTION__ << " pre-end" << std::endl;
	vkDeviceWaitIdle(m_instance.m_device.m_device);
	std::cout << __FUNCTION__ << " end" << std::endl;
}


/*
Vulkan Scratch


struct Instance {
	physical_devices: PhysicalDevice[*],
	surface : Surface,
};

struct PhysicalDevice {
	queue_families: QueueFamily[*],
	logical_devices: Device[*],
};

struct QueueFamily {
	type: from {graphics, present, ...}
	queues: type[*],
};

*/

auto VulkanInstance::query_layers() -> std::vector<VkLayerProperties> {
	u32 layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> available_layers;
	available_layers.resize(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
	//std::cout << "available layers:\n";
	//for (u32 i = 0; i < available_layers.size(); i++) {
	//	std::cout << '\t' << available_layers[i].layerName << '\n';
	//}
	if (!check_validation_layer_support(available_layers)) {
		__debugbreak();
	}
	return available_layers;
}

auto VulkanInstance::query_extensions() -> std::vector<VkExtensionProperties> {
	u32 extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> m_extensions;
	m_extensions.resize(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, m_extensions.data());
	//std::cout << "available extensions:\n";
	//for (u32 i = 0; i < m_extensions.size(); i++) {
	//	std::cout << '\t' << m_extensions[i].extensionName << '\n';
	//}
	return m_extensions;
}

auto VulkanInstance::query_physical_devices() -> std::vector<VulkanPhysicalDevice> {
	u32 device_count = 0;
	vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
	if (device_count == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> physical_devices;
	physical_devices.resize(device_count);

	vkEnumeratePhysicalDevices(m_instance, &device_count, physical_devices.data());
	std::vector<VulkanPhysicalDevice> physical_devices_2;
	physical_devices_2.resize(device_count);
	for (u32 i = 0; i < physical_devices_2.size(); i++) {
		physical_devices_2[i].m_physical_device = physical_devices[i];
	}

	//std::cout << "available devices:\n";
	//for (u32 i = 0; i < physical_devices.size(); i++) {
	//	VkPhysicalDeviceProperties device_properties;
	//	vkGetPhysicalDeviceProperties(physical_devices[i], &device_properties);
	//	auto vkver = vulkan_get_api_version(device_properties.apiVersion);
	//	std::cout
	//		<< '\t' << "device_properties.apiVersion: " << vkver.variant << "." << vkver.major << "." << vkver.minor << "." << vkver.patch << "." << '\n'
	//		<< '\t' << "device_properties.driverVersion: " << device_properties.driverVersion << '\n'
	//		<< '\t' << "device_properties.vendorID: " << std::hex << device_properties.vendorID << std::dec << '\n'
	//		<< '\t' << "device_properties.deviceID: " << device_properties.deviceID << '\n'
	//		<< '\t' << "device_properties.deviceType: " << vulkan_get_device_type_string(device_properties.deviceType) << '\n'
	//		<< '\t' << "device_properties.deviceName: " << device_properties.deviceName << '\n'
	//		<< '\t' << "device_properties.pipelineCacheUUID: " /*<< device_properties.pipelineCacheUUID*/ << '\n'
	//		<< '\t' << "device_properties.limits: " /*<< device_properties.limits*/ << '\n'
	//		<< '\t' << "device_properties.sparseProperties: " /*<< device_properties.sparseProperties*/ << '\n'
	//		;
	//}
	return physical_devices_2;
}

auto VulkanInstance::setup_debug() -> void {
	std::cout << __FUNCTION__ << std::endl;
	if (!enable_validation_layers) return;
	VkDebugUtilsMessengerCreateInfoEXT create_info;
	populate_debug_messenger_create_info(create_info);

	if (vkCreateDebugUtilsMessengerEXT(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

auto VulkanInstance::pick_physical_deivce() -> void {
	std::cout << __FUNCTION__ << std::endl;
	m_physical_devices = this->query_physical_devices();
	for (u32 i = 0; i < m_physical_devices.size(); i++) {
		m_physical_devices[i].m_extension_properties = m_physical_devices[i].query_extension_properties();
		m_physical_devices[i].m_extension_support = m_physical_devices[i].check_extension_support();
		m_physical_devices[i].m_queue_family_indices = find_queue_families(m_physical_devices[i], m_surface);
		m_physical_devices[i].m_swapchain_support_details = query_swapchain_support(m_physical_devices[i], m_surface);
	}
	for (u32 i = 0; i < m_physical_devices.size(); i++) {
		if (is_device_suitable(m_physical_devices[i])) {
			m_physical_device = m_physical_devices[i];
		}
	}
	if (m_physical_device.m_physical_device == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

auto VulkanInstance::create_surface(HWND window_handle) -> void {
	std::cout << __FUNCTION__ << std::endl;
	//PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR2;
	//vkCreateWin32SurfaceKHR2 = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkCreateWin32SurfaceKHR");
	//if (!vkCreateWin32SurfaceKHR2) {
	//	std::cout << "lll" << std::endl;
	//	__debugbreak();
	//}

	VkWin32SurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	//create_info.pNext;
	//create_info.flags;
	create_info.hinstance = ::GetModuleHandleW(NULL);
	create_info.hwnd = window_handle;

	VkResult result = vkCreateWin32SurfaceKHR(m_instance, &create_info, nullptr, &m_surface.m_surface);
	if (result != VK_SUCCESS) {
		std::cout << result << std::endl;
		__debugbreak();
		throw std::runtime_error("failed to create window surface!");
	}
}

auto VulkanInstance::init(HWND window_handle) -> void {
	m_layers = this->query_layers();
	m_extensions = this->query_extensions();
	m_extension_names.resize(m_extensions.size());
	for (u32 i = 0; i < m_extensions.size(); i++) {
		m_extension_names[i] = m_extensions[i].extensionName;
	}
	for (u32 i = 0; i < m_extensions.size(); i++) {
		m_extension_names[i] = m_extensions[i].extensionName;
	}

	if (enable_validation_layers && !check_validation_layer_support(m_layers)) {
		throw std::runtime_error("validation layers requested, but not available!");
	}


	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	//app_info.pNext;
	app_info.pApplicationName = "Hello Triangle";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_2;



	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	//create_info.pNext;
	//create_info.flags;
	create_info.pApplicationInfo = &app_info;
	//create_info.enabledLayerCount;
	//create_info.ppEnabledLayerNames;
	create_info.enabledExtensionCount = m_extensions.size();
	create_info.ppEnabledExtensionNames = m_extension_names.data();
	//__debugbreak();

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
	if (enable_validation_layers) {
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();

		populate_debug_messenger_create_info(debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	} else {
		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;
	}

	VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
		//__debugbreak();
	}

	this->setup_debug();
	m_surface.init(m_instance, window_handle);
	//this->create_surface(window_handle);
	this->pick_physical_deivce();
	m_device.init(m_physical_device, m_surface, window_handle);
	//this->create_logical_device(m_surface);

}

auto VulkanInstance::deinit() -> void {
	if (enable_validation_layers) {
		vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
	}
	vkDestroySurfaceKHR(m_instance, m_surface.m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

auto VulkanLogicalDevice::create_swapchain(VulkanPhysicalDevice physical_device, VulkanSurface surface, HWND window_handle) -> void {
	std::cout << __FUNCTION__ << std::endl;
	SwapChainSupportDetails swapchain_support = physical_device.m_swapchain_support_details;

	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swapchain_support.m_formats);
	VkPresentModeKHR present_mode = choose_swap_present_mode(swapchain_support.m_present_modes);
	VkExtent2D extent = choose_swap_extent(window_handle, swapchain_support.m_capabilities);

	u32 image_count = swapchain_support.m_capabilities.minImageCount + 1;
	if (swapchain_support.m_capabilities.maxImageCount > 0 && image_count > swapchain_support.m_capabilities.maxImageCount) {
		image_count = swapchain_support.m_capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	//create_info.pNext;
	//create_info.flags;
	create_info.surface = surface.m_surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//create_info.imageSharingMode;
	//create_info.queueFamilyIndexCount;
	//create_info.pQueueFamilyIndices;
	//create_info.preTransform;
	//create_info.compositeAlpha;
	//create_info.presentMode;
	//create_info.clipped;
	//create_info.oldSwapchain;


	QueueFamilyIndices& indices = physical_device.m_queue_family_indices;
	u32 queue_family_indices[] = {
		indices.graphics_family.value(),
		indices.present_family.value()
	};

	if (indices.graphics_family != indices.present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	create_info.preTransform = swapchain_support.m_capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	VkResult res = vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain);
	if (res != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
	m_swapchain_images.resize(image_count);
	if (VK_SUCCESS != vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, m_swapchain_images.data())) {
		__debugbreak();
	}

	m_swapchain_image_format = surface_format.format;
	m_swapchain_extent = extent;
}

auto VulkanLogicalDevice::create_image_views() -> void {
	std::cout << __FUNCTION__ << std::endl;
	m_swapchain_image_views.resize(m_swapchain_images.size());
	for (size_t i = 0; i < m_swapchain_images.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapchain_images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapchain_image_format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchain_image_views[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

auto VulkanLogicalDevice::create_render_pass() -> void {
	std::cout << __FUNCTION__ << std::endl;
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = m_swapchain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	if (vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_render_pass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

auto VulkanLogicalDevice::create_graphics_pipeline() -> void {
	std::cout << __FUNCTION__ << std::endl;
	//auto tm = &JadeFrame::get_singleton()->m_apps[0]->m_time_manager;
	//auto time_0 = tm->get_time();

#if 1
	std::future<std::vector<u32>> vert_shader_spirv = std::async(std::launch::async, string_to_SPIRV, vs, 0);
	std::future<std::vector<u32>> frag_shader_spirv = std::async(std::launch::async, string_to_SPIRV, fs, 1);

	VkShaderModule vert_shader_module = create_shader_module_from_spirv(m_device, vert_shader_spirv.get());
	VkShaderModule frag_shader_module = create_shader_module_from_spirv(m_device, frag_shader_spirv.get());
#else
	std::vector<u32> vert_shader_spirv = string_to_SPIRV(vs, 0);
	std::vector<u32> frag_shader_spirv = string_to_SPIRV(fs, 1);

	VkShaderModule vert_shader_module = create_shader_module_from_spirv(m_device, vert_shader_spirv);
	VkShaderModule frag_shader_module = create_shader_module_from_spirv(m_device, frag_shader_spirv);
#endif

	//auto time_1 = tm->get_time();
	//std::cout << "it took " << time_1 - time_0 << std::endl;

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	//vert_shader_stage_info.pNext;
	//vert_shader_stage_info.flags;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";
	//vert_shader_stage_info.pSpecializationInfo;

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	//frag_shader_stage_info.pNext;
	//frag_shader_stage_info.flags;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";
	//frag_shader_stage_info.pSpecializationInfo;

	VkPipelineShaderStageCreateInfo shader_stages[] = {
		vert_shader_stage_info,
		frag_shader_stage_info,
	};


	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapchain_extent.width;
	viewport.height = (float)m_swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchain_extent;

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &m_pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.layout = m_pipeline_layout;
	pipeline_info.renderPass = m_render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;


	if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_graphics_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(m_device, frag_shader_module, nullptr);
	vkDestroyShaderModule(m_device, vert_shader_module, nullptr);
}

auto VulkanLogicalDevice::create_framebuffers() -> void {
	std::cout << __FUNCTION__ << std::endl;
	m_swapchain_framebuffers.resize(m_swapchain_image_views.size());
	for (size_t i = 0; i < m_swapchain_image_views.size(); i++) {
		VkImageView attachments[] = {
			m_swapchain_image_views[i]
		};

		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = m_render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = m_swapchain_extent.width;
		framebuffer_info.height = m_swapchain_extent.height;
		framebuffer_info.layers = 1;

		if (vkCreateFramebuffer(m_device, &framebuffer_info, nullptr, &m_swapchain_framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

auto VulkanLogicalDevice::create_command_pool(VulkanPhysicalDevice physical_device) -> void {
	std::cout << __FUNCTION__ << std::endl;
	QueueFamilyIndices queue_family_indices = physical_device.m_queue_family_indices;
	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
	pool_info.flags = 0; // Optional

	if (vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

auto VulkanLogicalDevice::create_command_buffers() -> void {
	std::cout << __FUNCTION__ << std::endl;
	m_command_buffers.resize(m_swapchain_framebuffers.size());
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = m_command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)m_command_buffers.size();

	if (vkAllocateCommandBuffers(m_device, &alloc_info, m_command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
	for (size_t i = 0; i < m_command_buffers.size(); i++) {
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0; // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(m_command_buffers[i], &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = m_render_pass;
		render_pass_info.framebuffer = m_swapchain_framebuffers[i];
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = m_swapchain_extent;

		VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues = &clear_color;

		vkCmdBeginRenderPass(m_command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);
		vkCmdDraw(m_command_buffers[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(m_command_buffers[i]);

		if (vkEndCommandBuffer(m_command_buffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

auto VulkanLogicalDevice::create_sync_objects() -> void {
	std::cout << __FUNCTION__ << std::endl;
	m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	m_images_in_flight.resize(m_swapchain_images.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_available_semaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_render_finished_semaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_device, &fence_info, nullptr, &m_in_flight_fences[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

auto VulkanLogicalDevice::recreate_swapchain() -> void {
	vkDeviceWaitIdle(m_device);

	//this->create_swapchain();
	this->create_image_views();
	this->create_render_pass();
	this->create_graphics_pipeline();
	this->create_framebuffers();
	this->create_command_buffers();
}

auto VulkanLogicalDevice::draw_frame() -> void {
	std::cout << __FUNCTION__ << std::endl;
	vkWaitForFences(m_device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);
	u32 image_index;

	if (VK_SUCCESS != vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &image_index)) {
		__debugbreak();
	}

	if (m_images_in_flight[image_index] != VK_NULL_HANDLE) {
		vkWaitForFences(m_device, 1, &m_images_in_flight[image_index], VK_TRUE, UINT64_MAX);
	}

	VkSemaphore wait_semaphores[] = { m_image_available_semaphores[m_current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &m_command_buffers[image_index];

	VkSemaphore signal_semaphores[] = { m_render_finished_semaphores[m_current_frame] };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	vkResetFences(m_device, 1, &m_in_flight_fences[m_current_frame]);

	if (vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swapchains[] = { m_swapchain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;

	present_info.pImageIndices = &image_index;

	vkQueuePresentKHR(m_present_queue, &present_info);

	m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

auto VulkanLogicalDevice::init(VulkanPhysicalDevice physical_device, VulkanSurface surface, HWND window_handle) -> void {

	std::cout << __FUNCTION__ << std::endl;
	QueueFamilyIndices indices = physical_device.m_queue_family_indices;
	get_queue_families_info(physical_device);

	std::set<u32> unique_queue_families = {
		indices.graphics_family.value(),
		indices.present_family.value()
	};
	f32 queue_priority = 1.0_f32;
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	for (u32 queue_familiy : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_familiy;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures devices_features = {};
	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_create_infos.data();
	create_info.queueCreateInfoCount = static_cast<u32>(queue_create_infos.size());;
	create_info.pEnabledFeatures = &devices_features;
	create_info.enabledExtensionCount = static_cast<u32>(device_extensions.size());
	create_info.ppEnabledExtensionNames = device_extensions.data();

	if (enable_validation_layers) {
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
	} else {
		create_info.enabledLayerCount = 0;
	}
	if (vkCreateDevice(physical_device.m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}


	vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);
	vkGetDeviceQueue(m_device, indices.present_family.value(), 0, &m_present_queue);

	this->create_swapchain(physical_device, surface, window_handle);
	this->create_image_views();
	this->create_render_pass();
	this->create_graphics_pipeline();
	this->create_framebuffers();
	this->create_command_pool(physical_device);
	this->create_command_buffers();
	this->create_sync_objects();
}

auto VulkanLogicalDevice::deinit() -> void {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(m_device, m_render_finished_semaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_image_available_semaphores[i], nullptr);
		vkDestroyFence(m_device, m_in_flight_fences[i], nullptr);
	}
	vkDestroyCommandPool(m_device, m_command_pool, nullptr);
	vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
	vkDestroyRenderPass(m_device, m_render_pass, nullptr);
	for (u32 i = 0; i < m_swapchain_image_views.size(); i++) {
		vkDestroyImageView(m_device, m_swapchain_image_views[i], nullptr);
	}
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	vkDestroyDevice(m_device, nullptr);
}

auto VulkanSurface::init(VkInstance instance, HWND window_handle) -> void {
	std::cout << __FUNCTION__ << std::endl;
	//PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR2;
	//vkCreateWin32SurfaceKHR2 = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkCreateWin32SurfaceKHR");
	//if (!vkCreateWin32SurfaceKHR2) {
	//	std::cout << "lll" << std::endl;
	//	__debugbreak();
	//}

	VkWin32SurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	//create_info.pNext;
	//create_info.flags;
	create_info.hinstance = ::GetModuleHandleW(NULL);
	create_info.hwnd = window_handle;

	VkResult result = vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &m_surface);
	if (result != VK_SUCCESS) {
		std::cout << result << std::endl;
		__debugbreak();
		throw std::runtime_error("failed to create window surface!");
	}
}

auto VulkanPhysicalDevice::query_extension_properties() -> std::vector<VkExtensionProperties> {
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> available_extensions(extension_count);
	if (VK_SUCCESS != vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr, &extension_count, available_extensions.data())) {
		__debugbreak();
	}
	return available_extensions;
}

auto VulkanPhysicalDevice::check_extension_support() -> bool {
	std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
	for (u32 i = 0; i < m_extension_properties.size(); i++) {
		required_extensions.erase(m_extension_properties[i].extensionName);
	}
	return required_extensions.empty();
}
