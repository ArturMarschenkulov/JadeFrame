#pragma once
#include "../platform_shared.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>

namespace JadeFrame {

auto translate_key(KeySym keysym) -> KEY;
auto translate_button(unsigned int button) -> BUTTON;
#ifdef __linux__
// using InputManager = Linux_InputManager;
#endif

} // namespace JadeFrame