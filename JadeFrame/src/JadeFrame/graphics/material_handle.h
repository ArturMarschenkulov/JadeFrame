#pragma once
#include "JadeFrame/defines.h"

#include "opengl/opengl_shader_loader.h"

#include <string>

namespace JadeFrame {

struct TextureHandle {
public:

	TextureHandle(const TextureHandle&) = delete;
	auto operator=(const TextureHandle&)->TextureHandle & = delete;
	auto operator=(TextureHandle&&)->TextureHandle & = delete;

	TextureHandle() = default;
	TextureHandle(TextureHandle&&) = default;
	TextureHandle(const std::string& path);

	~TextureHandle();

	auto init() -> void;
public:
	u8* m_data;
	i32 m_width;
	i32 m_height;
	i32 m_num_components;

	enum class API {
		UNDEFINED,
		OPENGL,
		VULKAN,
	} api = API::OPENGL;
	void* m_handle = nullptr;
};
struct ShaderHandle {
public:
	ShaderHandle() = default;
	ShaderHandle(const GLSLCode& code);
	auto init() -> void;

public:
	std::string vertex_shader;
	std::string fragment_shader;


	enum class API {
		UNDEFINED,
		OPENGL,
		VULKAN,
	} api = API::OPENGL;
	void* m_handle = nullptr;
};
struct MaterialHandle {
	ShaderHandle* m_shader_handle;
	TextureHandle* m_texture_handle;

	auto init() const -> void;

	bool m_is_initialized = false;
};

}
