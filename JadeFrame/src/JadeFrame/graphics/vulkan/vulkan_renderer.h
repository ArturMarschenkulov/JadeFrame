#pragma once
#include "JadeFrame/defines.h"
#include "vulkan_context.h"
#include "JadeFrame/platform/windows/windows_window.h"


class Vulkan_Renderer {
public:
	auto set_context(Windows_Window* window) -> void;
	auto main_loop() -> void;
private:
	//Vulkan_Context m_context;
	Vulkan_Context* m_context_p;
};