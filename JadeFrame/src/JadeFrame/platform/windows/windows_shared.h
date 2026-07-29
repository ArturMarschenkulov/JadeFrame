#pragma once
#include "JadeFrame/prelude.h"
#include <Windows.h>

namespace JadeFrame {
namespace win32 {

struct EventMessage {
    HWND   hWnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
};

} // namespace win32
} // namespace JadeFrame
