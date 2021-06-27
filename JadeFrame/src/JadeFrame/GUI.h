#pragma once
#include <imgui/imgui.h>


struct HWND__;
typedef HWND__* HWND;

namespace JadeFrame {

auto GUI_init(HWND window) -> void;
auto GUI_new_frame() -> void;
auto GUI_render() -> void;
auto GUI_destroy() -> void;
}