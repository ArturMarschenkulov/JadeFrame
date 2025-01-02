#pragma once

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "../platform_shared.h"

namespace JadeFrame {

auto translate_key(KeySym keysym) -> KEY;
auto translate_button(unsigned int button) -> BUTTON;

} // namespace JadeFrame