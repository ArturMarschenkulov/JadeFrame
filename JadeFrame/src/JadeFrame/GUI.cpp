#include "pch.h"
#include "gui.h"
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_opengl3.h>

namespace JadeFrame {

auto GUI_init(HWND window) -> void {
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(window);
	const char* glsl_version = "#version 450";
	ImGui_ImplOpenGL3_Init(glsl_version);
}
auto GUI_new_frame() -> void {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}
auto GUI_render() -> void {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
auto GUI_destroy() -> void {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
}