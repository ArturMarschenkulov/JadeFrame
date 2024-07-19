#pragma once
#include "JadeFrame/math/vec.h"
#include "../platform_shared.h"
#include <Windows.h>

#include <array>
#include <queue>

namespace JadeFrame {
namespace win32 {
struct EventMessage;

struct Event {};

auto key_callback(const EventMessage& window_message,  WindowEventQueue* event_queue) -> void;
auto char_callback(const EventMessage& window_message) -> void;
auto mouse_button_callback(const EventMessage& window_message,  WindowEventQueue* event_queue) -> void;

} // namespace win32
} // namespace JadeFrame