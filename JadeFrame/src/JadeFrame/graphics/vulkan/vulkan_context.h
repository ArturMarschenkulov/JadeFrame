#pragma once
#include <vulkan/vulkan.h>



#include "vulkan_instance.h"

#include <vector>


namespace JadeFrame {

class Windows_Window;

struct Vulkan_Context {
	//Vulkan_Context(const Vulkan_Context&) = delete;
	//Vulkan_Context(Vulkan_Context&&) = delete;
	//auto operator=(const Vulkan_Context&)->Vulkan_Context & = delete;
	//auto operator=(Vulkan_Context&&)->Vulkan_Context & = delete;

	Vulkan_Context() = default;
	Vulkan_Context(const Windows_Window& window);
	~Vulkan_Context();
public:
	VulkanInstance m_instance;
	HWND m_window_handle;
public:
	auto main_loop() -> void;
};
}