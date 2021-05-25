
#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan_context.h"
#include "JadeFrame/defines.h"



#include <iostream>
#include <optional>


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validation_layers = {
	"VK_LAYER_KHRONOS_validation"
};

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

static auto is_device_suitable(VkPhysicalDevice device) -> bool {
	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(device, &device_properties);

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(device, &device_features);

	return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader;
}

//static auto get_required_instance_extensions(u32* count) -> const char* {
//    const char* extensions[2];
//    extensions[0] = "VK_KHR_surface";
//    extensions[1] = "VK_KHR_win32_surface";
//    return *extensions;
//}


static auto get_vulkan_device_type(const VkPhysicalDeviceType& device_type) -> const char* {
	const char* result;
	switch (device_type) {
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			result = "VK_PHYSICAL_DEVICE_TYPE_OTHER";
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			result = "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
	}
	return result;
}

struct QueueFamilyIndices {
	std::optional<u32> graphics_family;
	std::optional<u32> present_family;
	auto is_complete() -> bool {
		return graphics_family.has_value() && present_family.has_value();
	}

};

static auto find_queue_families(VkPhysicalDevice device) -> QueueFamilyIndices {
	QueueFamilyIndices indices;

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

	u32 i = 0;
	for (u32 j = 0; j < queue_families.size(); j++) {
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}

		if (indices.is_complete()) {
			break;
		}

		i++;
	}

	return indices;
}

auto Vulkan_Context::query_layers() -> std::vector<VkLayerProperties> {
	u32 layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> available_layers;
	available_layers.resize(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
	std::cout << "available layers:\n";
	for (u32 i = 0; i < available_layers.size(); i++) {
		std::cout << '\t' << available_layers[i].layerName << '\n';
	}
	if (!check_validation_layer_support(available_layers)) {
		__debugbreak();
	}
	return available_layers;
}

auto Vulkan_Context::query_physical_devices() -> std::vector<VkPhysicalDevice> {
	u32 device_count = 0;
	vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
	if (device_count == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> physical_devices;
	physical_devices.resize(device_count);

	vkEnumeratePhysicalDevices(m_instance, &device_count, physical_devices.data());
	std::cout << "available devices:\n";
	for (u32 i = 0; i < physical_devices.size(); i++) {
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(physical_devices[i], &device_properties);
		std::cout
			<< '\t' << "device_properties.apiVersion: " << device_properties.apiVersion << '\n'
			<< '\t' << "device_properties.driverVersion: " << device_properties.driverVersion << '\n'
			<< '\t' << "device_properties.vendorID: " << device_properties.vendorID << '\n'
			<< '\t' << "device_properties.deviceID: " << device_properties.deviceID << '\n'
			<< '\t' << "device_properties.deviceType: " << get_vulkan_device_type(device_properties.deviceType) << '\n'
			<< '\t' << "device_properties.deviceName: " << device_properties.deviceName << '\n'
			<< '\t' << "device_properties.pipelineCacheUUID: " /*<< device_properties.pipelineCacheUUID*/ << '\n'
			<< '\t' << "device_properties.limits: " /*<< device_properties.limits*/ << '\n'
			<< '\t' << "device_properties.sparseProperties: " /*<< device_properties.sparseProperties*/ << '\n'
			;
	}
	return physical_devices;
}

auto Vulkan_Context::query_extensions() -> std::vector<VkExtensionProperties> {
	u32 extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> m_extensions;
	m_extensions.resize(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, m_extensions.data());
	std::cout << "available extensions:\n";
	for (u32 i = 0; i < m_extensions.size(); i++) {
		std::cout << '\t' << m_extensions[i].extensionName << '\n';
	}
	return m_extensions;
}

Vulkan_Context::Vulkan_Context(HWND window) {
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext;
	app_info.pApplicationName = "Hello Triangle";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	m_extensions = this->query_extensions();
	m_extension_names.resize(m_extensions.size());
	for (u32 i = 0; i < m_extensions.size(); i++) {
		m_extension_names[i] = m_extensions[i].extensionName;
	}

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pNext = nullptr;
	create_info.flags;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledLayerCount;
	create_info.ppEnabledLayerNames;
	create_info.enabledExtensionCount = m_extensions.size();
	create_info.ppEnabledExtensionNames = m_extension_names.data();

	VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
		__debugbreak();
	}



	m_layers = this->query_layers();
	m_physical_devices = this->query_physical_devices();
	for (u32 i = 0; i < m_physical_devices.size(); i++) {
		m_physical_device = m_physical_devices[0];
	}
	if (m_physical_device == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	{
		QueueFamilyIndices indices = find_queue_families(m_physical_device);
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = indices.graphics_family.value();
		queue_create_info.queueCount = 1;

		f32 queue_priority = 1.0_f32;
		queue_create_info.pQueuePriorities = &queue_priority;

		VkPhysicalDeviceFeatures devices_features = {};
		VkDeviceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pQueueCreateInfos = &queue_create_info;
		create_info.queueCreateInfoCount = 1;

		create_info.pEnabledFeatures = &devices_features;

		create_info.enabledExtensionCount = 0;

		if (enable_validation_layers) {
			create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			create_info.ppEnabledLayerNames = validation_layers.data();
		} else {
			create_info.enabledLayerCount = 0;
		}
		if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);

	}
	{ //This is window specific

		VkWin32SurfaceCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		create_info.hwnd = window;
		create_info.hinstance = ::GetModuleHandleW(nullptr);
		VkResult result = vkCreateWin32SurfaceKHR(m_instance, &create_info, nullptr, &m_surface);
		if (result != VK_SUCCESS) {
			std::cout << result << std::endl;
			__debugbreak();
			throw std::runtime_error("failed to create window surface!");
		}
	}
	{
		//VkBool32 presentSupport = false;
		//vkGetPhysicalDeviceSurfaceSupportKHR(m_device, i, m_surface, &presentSupport);
	}





}

Vulkan_Context::~Vulkan_Context() {

	vkDestroyDevice(m_device, nullptr);
	//if (enable_validation_layers) {
	//	DestroyDebugUtilsMessengerEXT(m_instance, debug_messenger, nullptr);
	//}
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}