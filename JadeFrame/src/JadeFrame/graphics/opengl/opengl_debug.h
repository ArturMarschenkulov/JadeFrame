#include "JadeFrame/defines.h"

namespace JadeFrame {

auto opengl_message_callback(u32 source, u32 type, u32 id, u32 severity, i32 length, const char* message, const void* userParam) -> void;

auto set_debug_mode(bool b) -> void;

}