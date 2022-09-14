#include "pch.h"
#include "gui.h"
#include <imgui/imgui.h>
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include "imgui/backends/imgui_impl_win32.h"

namespace JadeFrame {

auto GUI::init(HWND window, GRAPHICS_API api) -> void {
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(window);
    switch (api) {
        case GRAPHICS_API::OPENGL: {
            const char* glsl_version = "#version 450";
            ImGui_ImplOpenGL3_Init(glsl_version);
        } break;
        case GRAPHICS_API::VULKAN: {
            const char*               glsl_version = "#version 450";
            ImGui_ImplVulkan_InitInfo info;
            info.Instance;
            info.PhysicalDevice;
            info.Device;
            info.QueueFamily;
            info.Queue;
            info.PipelineCache;
            info.DescriptorPool;
            info.Subpass;
            info.MinImageCount; // >= 2
            info.ImageCount;    // >= MinImageCount
            info.MSAASamples;   // >= VK_SAMPLE_COUNT_1_BIT (0 -> default to VK_SAMPLE_COUNT_1_BIT)
            info.Allocator;
            info.CheckVkResultFn;

            // ImGui_ImplVulkan_Init(&info);
            // ImGui_ImplOpenGL3_Init(glsl_version);
        } break;
        default: assert(0);
    }
}

auto GUI::new_frame() -> void {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}
auto GUI::render() -> void {

    ImGui::Text("Hello, world %d", 123);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
auto GUI::destroy() -> void {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

// auto GUI_init(HWND window) -> void {
//     ImGui::CreateContext();
//     ImGui_ImplWin32_Init(window);
//     const char* glsl_version = "#version 450";
//     ImGui_ImplOpenGL3_Init(glsl_version);
// }
// auto GUI_new_frame() -> void {
//     ImGui_ImplOpenGL3_NewFrame();
//     ImGui_ImplWin32_NewFrame();
//     ImGui::NewFrame();
// }
// auto GUI_render() -> void {
//     ImGui::Text("Hello, world %d", 123);
//     ImGui::Render();
//     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
// }
// auto GUI_destroy() -> void {
//     ImGui_ImplOpenGL3_Shutdown();
//     ImGui_ImplWin32_Shutdown();
//     ImGui::DestroyContext();
// }
} // namespace JadeFrame