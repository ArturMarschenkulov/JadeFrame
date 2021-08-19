#include "pch.h"

#include "vulkan_renderer.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "../opengl/opengl_renderer.h"

#include "../shared.h"


#include <iostream>
namespace JadeFrame {


Vulkan_Renderer::Vulkan_Renderer(const Windows_Window& window)
	: m_context(window) {

}
auto Vulkan_Renderer::set_clear_color(const Color& color) -> void {
	// dummy
}

auto Vulkan_Renderer::main_loop() -> void {
	m_context.main_loop();
}

auto Vulkan_Renderer::present() const -> void {
	//m_context.m_instance.m_logical_device.draw_frame();
}

auto Vulkan_Renderer::clear_background() -> void {
}

auto Vulkan_Renderer::render(const Matrix4x4& /*view_projection*/) -> void {
}

auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {

}
auto Vulkan_Renderer::submit(const Object& obj) -> void {

}
auto Vulkan_Renderer::take_screenshot(const char* filename) -> void {
	__debugbreak();
}
}