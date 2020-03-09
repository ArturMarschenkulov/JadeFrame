#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_opengl3.h>

auto GUI_init(GLFWwindow* window) -> void;
auto GUI_new_frame() -> void;
auto GUI_render() -> void;
auto GUI_destroy() -> void;