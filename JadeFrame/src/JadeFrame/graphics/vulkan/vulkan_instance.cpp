#include "vulkan_instance.h"
#include "vulkan_debug.h"

#include <iostream>



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
		if (!layer_found) {
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
	if (!check_validation_layer_support(available_layers)) {
		__debugbreak();
	}
	return available_layers;
	//std::cout << "available layers:\n";
	//for (u32 i = 0; i < available_layers.size(); i++) {
	//	std::cout << '\t' << available_layers[i].layerName << '\n';
	//}
}

auto VulkanInstance::query_extensions() -> std::vector<VkExtensionProperties> {
	u32 extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	//std::vector<VkExtensionProperties> m_extensions;
	m_extensions.resize(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, m_extensions.data());
	return m_extensions;
	//std::cout << "available extensions:\n";
	//for (u32 i = 0; i < m_extensions.size(); i++) {
	//	std::cout << '\t' << m_extensions[i].extensionName << '\n';
	//}
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
	VkResult result;
	if (!m_enable_validation_layers) return;
	VkDebugUtilsMessengerCreateInfoEXT create_info;
	populate_debug_messenger_create_info(create_info);

	result = vkCreateDebugUtilsMessengerEXT(m_instance, &create_info, nullptr, &m_debug_messenger);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
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
	if (m_enable_validation_layers) {
		create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
		create_info.ppEnabledLayerNames = m_validation_layers.data();

		populate_debug_messenger_create_info(debug_create_info);
		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
	} else {
		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;
	}

	result = vkCreateInstance(&create_info, nullptr, &m_instance);
	if (result != VK_SUCCESS) {
		std::cout << "failed to create instance!" << std::endl;
		__debugbreak();
	}

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
			std::cout << "failed to find a suitable GPU!" << std::endl;
			__debugbreak();
		}
	}
	m_logical_device.init(*this);
}

auto VulkanInstance::deinit() -> void {
	if (m_enable_validation_layers) {
		vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
	}
	vkDestroySurfaceKHR(m_instance, m_surface.m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}
}