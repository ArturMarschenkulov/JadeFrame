#include "pch.h"
#include "opengl_debug.h"
#include <glad/glad.h>
#include <string>
#include "JadeFrame/defines.h"

namespace JadeFrame {

auto opengl_message_callback(u32 source, u32 type, u32 id, u32 severity, i32 length, const char* message, const void* userParam) -> void {
	std::string _source;
	switch (source) {
		case GL_DEBUG_SOURCE_API:				_source = "API "; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		_source = "Window "; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:	_source = "Shader "; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:		_source = "Third Party "; break;
		case GL_DEBUG_SOURCE_APPLICATION:		_source = "Application "; break;
		case GL_DEBUG_SOURCE_OTHER:				_source = "Other "; break;
		default:								_source = "UNKNOWN "; break;
	}

	std::string _type;
	switch (type) {
		case GL_DEBUG_TYPE_ERROR:				_type = "Type Error "; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	_type = "Deprecated Behavior "; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	_type = "Undefined Behavior "; break;
		case GL_DEBUG_TYPE_PORTABILITY:			_type = "Protability "; break;
		case GL_DEBUG_TYPE_PERFORMANCE:			_type = "Performance "; break;
		case GL_DEBUG_TYPE_OTHER:				_type = "Other "; break;
		case GL_DEBUG_TYPE_MARKER:				_type = "Marker "; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:			_type = "Push Group "; break;
		case GL_DEBUG_TYPE_POP_GROUP:			_type = "Pop Group "; break;
		default:								_type = "UKNOWN "; break;
	}

	auto _id = id;

	std::string _severity;
	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:			_severity = "High "; break;
		case GL_DEBUG_SEVERITY_MEDIUM:			_severity = "Medium "; break;
		case GL_DEBUG_SEVERITY_LOW:				_severity = "Low "; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:	_severity = "Notification "; break;
		default:								_severity = "UKNOWN "; break;
	}

	if (0) {
		Logger::log("GL_ERR: Source: {} | Type: {} | ID: {} | Severity {} | Message {}",
			_source, _type, _id, _severity, message
		);
	} else {
		Logger::log(
R"(
GL_ERR: Source: {} 
| Type: {} 
| ID: {} 
| Severity {} 
| Message {}
|-----------------------------------------|
)",
			_source, _type, _id, _severity, message
		);
	}
}

auto set_debug_mode(bool b) -> void {
	if (b) { // enable debug output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(opengl_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	}
}
}