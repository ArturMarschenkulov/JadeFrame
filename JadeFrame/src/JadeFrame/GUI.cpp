#include "GUI.h"

auto GUI_init(GLFWwindow* window) -> void {
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	//ImGui_ImplWin32_Init(window);
	ImGui_ImplOpenGL3_Init();
}
auto GUI_new_frame() -> void {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	//ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}
auto GUI_render() -> void {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}
auto GUI_destroy() -> void {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	//ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}