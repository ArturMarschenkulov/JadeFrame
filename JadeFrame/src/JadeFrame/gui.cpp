#include "pch.h"
#include "gui.h"
#include <imgui/imgui.h>
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#if defined(_WIN32)
    #include "imgui/backends/imgui_impl_win32.h"
    #include "JadeFrame/platform/windows/windows_window.h"
#elif defined(__linux__)
    #include "imgui/backends/imgui_impl_x11.h"
#endif

/*
    The `imgui` repo does not have a linux/x11/wayland backend. One can use `glfw`,
    but that means that we'd have to integrate `glfw` into the engine, which is not
   optimal.


    This means that we'll have to write our own. There are some attempts in the wild
   already. For now, I will collect various links to such resources so that I can write my
   own x11/wayland backend. https://github.com/ocornut/imgui/pull/3372
   https://github.com/ocornut/imgui/issues/4224

*/

namespace JadeFrame {

auto GUI::init(IWindow* window, GRAPHICS_API api) -> void {
    ImGui::CreateContext();
#if _WIN32
    ImGui_ImplWin32_Init((HWND)window->get());
#elif __linux__
    ImGui_ImplX11_Init(nullptr, nullptr);
#endif
    switch (api) {
        case GRAPHICS_API::OPENGL: {
            const char* glsl_version = "#version 450";
            ImGui_ImplOpenGL3_Init(glsl_version);
        } break;
        case GRAPHICS_API::VULKAN: {
            // const char*               glsl_version = "#version 450";
            // ImGui_ImplVulkan_InitInfo info;
            // info.Instance;
            // info.PhysicalDevice;
            // info.Device;
            // info.QueueFamily;
            // info.Queue;
            // info.PipelineCache;
            // info.DescriptorPool;
            // info.Subpass;
            // info.MinImageCount; // >= 2
            // info.ImageCount;    // >= MinImageCount
            // info.MSAASamples;   // >= VK_SAMPLE_COUNT_1_BIT (0 -> default to
            // VK_SAMPLE_COUNT_1_BIT) info.Allocator; info.CheckVkResultFn;

            // ImGui_ImplVulkan_Init(&info);
            // ImGui_ImplOpenGL3_Init(glsl_version);
        } break;
        default: assert(0);
    }
}

auto GUI::new_frame() -> void {

    ImGui_ImplOpenGL3_NewFrame();
#if _WIN32
    ImGui_ImplWin32_NewFrame();
#elif __linux__
    ImGui_ImplX11_NewFrame();
#endif

    ImGui::NewFrame();
}

auto GUI::render() -> void {

    ImGui::Text("Hello, world %d", 123);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

auto GUI::destroy() -> void {
    ImGui_ImplOpenGL3_Shutdown();
#if _WIN32
    ImGui_ImplWin32_Shutdown();
#elif __linux__
    ImGui_ImplX11_Shutdown();
#endif
    ImGui::DestroyContext();
}
} // namespace JadeFrame