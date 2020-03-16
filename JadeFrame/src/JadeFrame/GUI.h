#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_opengl3.h>
#include <Windows.h>

auto GUI_init(HWND window) -> void;
auto GUI_new_frame() -> void;
auto GUI_render() -> void;
auto GUI_destroy() -> void;