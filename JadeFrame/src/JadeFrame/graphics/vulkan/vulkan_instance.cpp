#include "pch.h"
#include "vulkan_instance.h"
#include "vulkan_debug.h"




namespace JadeFrame {

auto VulkanInstance::check_validation_layer_support(const std::vector<VkLayerProperties>& available_layers) -> bool {
	for (u32 i = 0; i < m_validation_layers.size(); i++) {
		bool layer_found = false;
		for (u32 j = 0; j < available_layers.size(); j++) {
			if (strcmp(m_validation_layers[i], available_layers[j].layerName) == 0) {
				layer_found = true;
				break;
			}
		}
		if (layer_found == false) {
			return false;
		}
	}
	return true;
}

static auto is_device_suitable(VulkanPhysicalDevice physical_device) -> bool {
	bool swapchain_adequate = false;
	if (physical_device.m_extension_support) {
		swapchain_adequate =
			!physical_device.m_surface_formats.empty() &&
			!physical_device.m_present_modes.empty()
			;
	}
	return
		physical_device.m_queue_family_indices.is_complete() &&
		physical_device.m_extension_support &&
		swapchain_adequate
		;
}

auto VulkanInstance::query_layers() -> std::vector<VkLayerProperties> {
	u32 layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> available_layers;
	available_layers.resize(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
	if (!this->check_validation_layer_support(available_layers)) {
		__debugbreak();
	}
	return available_layers;
}

auto VulkanInstance::query_extensions() -> std::vector<VkExtensionProperties> {
	u32 extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	//std::vector<VkExtensionProperties> m_extensions;
	m_extensions.resize(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, m_extensions.data());
	return m_extensions;
}

auto VulkanInstance::query_physical_devices() -> std::vector<VulkanPhysicalDevice> {
	u32 device_count = 0;
	vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
	if (device_count == 0) {
		//throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> physical_devices;
	physical_devices.resize(device_count);

	vkEnumeratePhysicalDevices(m_instance, &device_count, physical_devices.data());
	std::vector<VulkanPhysicalDevice> physical_devices_2;
	physical_devices_2.resize(device_count);
	for (u32 i = 0; i < physical_devices_2.size(); i++) {
		physical_devices_2[i].m_handle = physical_devices[i];
	}
	return physical_devices_2;
}

auto VulkanInstance::setup_debug() -> void {
	VkResult result;
	if (!m_enable_validation_layers) return;
	VkDebugUtilsMessengerCreateInfoEXT create_info;
	populate_debug_messenger_create_info(create_info);

	result = vkCreateDebugUtilsMessengerEXT_(m_instance, &create_info, nullptr, &m_debug_messenger);
	if (result != VK_SUCCESS) __debugbreak();
}

auto VulkanInstance::init(HWND window_handle) -> void {
	VkResult result;
	m_window_handle = window_handle;

	m_layers = this->query_layers();
	m_extensions = this->query_extensions();
	m_extension_names.resize(m_extensions.size());
	for (u32 i = 0; i < m_extensions.size(); i++) {
		m_extension_names[i] = m_extensions[i].extensionName;
	}
	for (u32 i = 0; i < m_extensions.size(); i++) {
		m_extension_names[i] = m_extensions[i].extensionName;
	}

	if (m_enable_validation_layers && !check_validation_layer_support(m_layers)) {
		throw std::runtime_error("validation layers requested, but not available!");
	}


	const VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = {},
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_2,
	};


	VkInstanceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.pApplicationInfo = &app_info,
		.enabledLayerCount = {},
		.ppEnabledLayerNames = {},
		.enabledExtensionCount = static_cast<u32>(m_extensions.size()),
		.ppEnabledExtensionNames = m_extension_names.data(),
	};

	VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
	if (m_enable_validation_layers) {
		create_info.enabledLayerCount = static_cast<u32>(m_validation_layers.size());
		create_info.ppEnabledLayerNames = m_validation_layers.data();

		populate_debug_messenger_create_info(debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	} else {
		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;
	}

	result = vkCreateInstance(&create_info, nullptr, &m_instance);
	if (result != VK_SUCCESS) __debugbreak();

	this->setup_debug();
	m_surface.init(m_instance, window_handle);
	{
		m_physical_devices = this->query_physical_devices();
		for (u32 i = 0; i < m_physical_devices.size(); i++) {
			m_physical_devices[i].init(*this, m_surface);
		}
		for (u32 i = 0; i < m_physical_devices.size(); i++) {
			if (is_device_suitable(m_physical_devices[i])) {
				m_physical_device = m_physical_devices[i];
			}
		}
		if (m_physical_device.m_handle == VK_NULL_HANDLE) {
			__debugbreak();
		}
	}
	m_logical_device.init(*this);
}

auto VulkanInstance::deinit() -> void {
	if (m_enable_validation_layers) {
		vkDestroyDebugUtilsMessengerEXT_(m_instance, m_debug_messenger, nullptr);
	}
	vkDestroySurfaceKHR(m_instance, m_surface.m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}
}