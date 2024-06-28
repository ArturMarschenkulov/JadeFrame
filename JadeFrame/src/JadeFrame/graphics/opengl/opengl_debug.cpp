#include "pch.h"
#include "opengl_debug.h"
#include <glad/glad.h>
#include <string>
#include "JadeFrame/prelude.h"

namespace JadeFrame {
namespace opengl {

auto opengl_message_callback(
    u32 source,
    u32 type,
    u32 id,
    u32 severity,
    i32 /*length*/,
    const char* message,
    const void* /*userParam*/
) -> void {
    std::string _source;
#define foo(name)                                                                        \
    case name: _source = #name; break
    switch (source) {
        foo(GL_DEBUG_SOURCE_API);
        foo(GL_DEBUG_SOURCE_WINDOW_SYSTEM);
        foo(GL_DEBUG_SOURCE_SHADER_COMPILER);
        foo(GL_DEBUG_SOURCE_THIRD_PARTY);
        foo(GL_DEBUG_SOURCE_APPLICATION);
        foo(GL_DEBUG_SOURCE_OTHER);
        default: _source = "UNKNOWN"; break;
    }
#undef foo

    std::string _type;
#define foo(name)                                                                        \
    case name: _type = #name; break
    switch (type) {
        foo(GL_DEBUG_TYPE_ERROR);
        foo(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR);
        foo(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
        foo(GL_DEBUG_TYPE_PORTABILITY);
        foo(GL_DEBUG_TYPE_PERFORMANCE);
        foo(GL_DEBUG_TYPE_OTHER);
        foo(GL_DEBUG_TYPE_MARKER);
        foo(GL_DEBUG_TYPE_PUSH_GROUP);
        foo(GL_DEBUG_TYPE_POP_GROUP);
        default: _type = "UKNOWN"; break;
    }
#undef foo

    auto _id = id;

    std::string _severity;
#define foo(name)                                                                        \
    case name: _severity = #name; break

    switch (severity) {
        foo(GL_DEBUG_SEVERITY_HIGH);
        foo(GL_DEBUG_SEVERITY_MEDIUM);
        foo(GL_DEBUG_SEVERITY_LOW);
        foo(GL_DEBUG_SEVERITY_NOTIFICATION);
        default: _severity = "UKNOWN"; break;
    }
#undef foo

    if (0) {
        Logger::log(
            "GL_ERR: Source: {} | Type: {} | ID: {} | Severity: {} | Message: {}",
            _source,
            _type,
            _id,
            _severity,
            message
        );
    } else {
        Logger::log(
            R"(
GL_ERR: Source: {} 
| Type: {} 
| ID: {} 
| Severity: {} 
| Message: {}
|-----------------------------------------|
)",
            _source,
            _type,
            _id,
            _severity,
            message
        );
    }
}

auto set_debug_mode(bool enable_debug) -> void {
    int flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    bool is_debug_enabled = (flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0;

    if (is_debug_enabled) {
        Logger::info("OpenGL debug mode enabled");
    } else {
        Logger::warn("OpenGL debug mode disabled");
    }
    if (enable_debug) { // enable debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(opengl_message_callback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

        if (!is_debug_enabled) { Logger::warn("OpenGL debug mode enabled"); }
    } else {
        glDisable(GL_DEBUG_OUTPUT);
        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        if (is_debug_enabled) { Logger::warn("OpenGL debug mode disabled"); }
    }
}
} // namespace opengl
} // namespace JadeFrame