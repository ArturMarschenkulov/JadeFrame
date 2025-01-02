#include "JadeFrame/platform/window_event.h"
#include "JadeFrame/utils/assert.h"

#include "gui.h"

#include "JadeFrame/platform/window.h"
#include "JadeFrame/platform/linux/linux_window.h"
#include <imgui/imgui.h>
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include "JadeFrame/platform/window.h"

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

auto GUI::init(Window* window, GRAPHICS_API api) -> void {
    if (api != GRAPHICS_API::VULKAN && api != GRAPHICS_API::OPENGL) {
        Logger::err("{} is not supported", to_string(api));
        JF_ASSERT(false, "Right now only VULKAN and OPENGL are supported");
    }
    m_graphics_api = api;

    ImGui::CreateContext();
#if _WIN32
    // ImGui_ImplWin32_Init((HWND)window->get());
#elif __linux__

    NativeWindow* native_window = window->m_native_window.get();
    auto*         native_window_x11 = dynamic_cast<X11_NativeWindow*>(native_window);
    ::Display*    display = native_window_x11->m_display;
    ::Window      win = native_window_x11->m_window;
    ImGui_ImplX11_Init(display, (void*)win);
#endif
    switch (api) {
        case GRAPHICS_API::OPENGL: {
            const char* glsl_version = "#version 450";
            ImGui_ImplOpenGL3_Init(glsl_version);
            m_is_initialized = true;
        } break;
        case GRAPHICS_API::VULKAN: {
            m_is_initialized = false;
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
            //                     // VK_SAMPLE_COUNT_1_BIT)
            // info.Allocator;
            // info.CheckVkResultFn;

            // ImGui_ImplVulkan_Init(&info);
            // ImGui_ImplOpenGL3_Init(glsl_version);
        } break;
        default: assert(0);
    }
}

auto GUI::new_frame() -> void {

    if (m_graphics_api == GRAPHICS_API::OPENGL) {
        ImGui_ImplOpenGL3_NewFrame();
    } else if (m_graphics_api == GRAPHICS_API::VULKAN) {
        // ImGui_ImplVulkan_NewFrame();
    } else {
        assert(0);
    }
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

    if (m_graphics_api == GRAPHICS_API::OPENGL) {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    } else if (m_graphics_api == GRAPHICS_API::VULKAN) {
        // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), nullptr);
    } else {
        assert(0);
    }
}

auto GUI::destroy() -> void {
    if (m_graphics_api == GRAPHICS_API::OPENGL) {
        ImGui_ImplOpenGL3_Shutdown();
    } else if (m_graphics_api == GRAPHICS_API::VULKAN) {
        // ImGui_ImplVulkan_Shutdown();
    } else {
        assert(0);
    }
#if _WIN32
    ImGui_ImplWin32_Shutdown();
#elif __linux__
    ImGui_ImplX11_Shutdown();
#endif
    ImGui::DestroyContext();
}
} // namespace JadeFrame