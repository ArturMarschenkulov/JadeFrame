#pragma once
#include "JadeFrame/defines.h"
#include "vulkan_context.h"




struct HWND__;	typedef HWND__* HWND;
class Vulkan_Renderer {
public:
	auto set_context(const HWND& window) -> void;
private:
	Vulkan_Context m_context;
};