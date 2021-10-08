#pragma once
#include "JadeFrame/defines.h"
#include <JadeFrame/math/mat_4.h>
//#include "opengl/opengl_shader_loader.h"
//#include "Mesh.h"

namespace JadeFrame {

class Windows_Window;
class Object;
class RGBAColor;



enum class GRAPHICS_API {
	UNDEFINED = -1,
	OPENGL,
	VULKAN,
	D3D11,
	D3D12,
	METAL,
	SOFTWARE,
	TERMINAL,
};
enum class SHADING_LANGUAGE {
	GLSL,
	SPIRV,
	HLSL
};
enum class SHADER_STAGE {
	VERTEX,
	FRAGMET, //PIXEL
	GEOMETRY,
	TESSELATION,
	COMPUTE,
};

struct ShadingCode {
	SHADING_LANGUAGE shading_language;
	std::string m_vertex_shader;
	std::string m_fragment_shader;
};

struct ShaderModule {
	SHADING_LANGUAGE shading_language;
};


struct GPUDataMeshHandle {
	GRAPHICS_API m_api = GRAPHICS_API::UNDEFINED;
	void* m_handle = nullptr;

	mutable bool m_is_initialized = false;
};

/*
	TODO: Consider whether this is a good way and whether it is worth it to introdcue inheritance.
		Right now, inheritance should be mainly used as a sanity check such that all renderers have a common interface.
*/
class IRenderer {
public: // client stuff
	virtual auto submit(const Object& obj) -> void = 0;
	virtual auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void = 0;
	
	virtual auto take_screenshot(const char* filename) -> void = 0;


public: // more internal stuff
	virtual auto set_clear_color(const RGBAColor& color) -> void = 0;
	virtual auto clear_background() -> void = 0;
	virtual auto render(const Matrix4x4& view_projection) -> void = 0;
	virtual auto present()  -> void = 0;
};




//
//struct RasterizerState {
//	enum FILL_MODE {
//	} m_fill_mode;
//	enum CULL_MODE {
//	} m_cull_mode;
//	f32 front_ccw;
//	f32 m_rasterizer_discard;
//
//
//	f32 m_depth_bias;
//	f32 m_slope_scaled_bias;
//	f32 m_offset_clamp;
//	f32 m_clip_setup;
//
//	f32 m_depth_clamp;
//	f32 m_scissor;
//	f32 m_provoking_vertex;
//	f32 m_clip_distance;
//
//	f32 m_point_size;
//	f32 m_line_width;
//};
enum class SHADER_TYPE {
	NONE = 0,
	FLOAT, FLOAT_2, FLOAT_3, FLOAT_4,
	MAT_3, MAT_4,
	INT, INT_2, INT_3, INT_4,
	BOOL,
	SAMPLER_1D, SAMPLER_2D, SAMPLER_3D, SAMPLER_CUBE,
}; 
inline auto SHADER_TYPE_get_size(const SHADER_TYPE type) -> u32 {
	u32 result;
	switch (type) {
		case SHADER_TYPE::FLOAT:	result = 4; break;
		case SHADER_TYPE::FLOAT_2:	result = 4 * 2; break;
		case SHADER_TYPE::FLOAT_3:	result = 4 * 3; break;
		case SHADER_TYPE::FLOAT_4:	result = 4 * 4; break;
		case SHADER_TYPE::MAT_3:	result = 4 * 3 * 3; break;
		case SHADER_TYPE::MAT_4:	result = 4 * 4 * 4; break;
		case SHADER_TYPE::INT:		result = 4; break;
		case SHADER_TYPE::INT_2:	result = 4 * 2; break;
		case SHADER_TYPE::INT_3:	result = 4 * 3; break;
		case SHADER_TYPE::INT_4:	result = 4 * 4; break;
		case SHADER_TYPE::BOOL:	result = 1; break;
		default: __debugbreak(); result = 0; break;
	}
	return result;
}

struct VertexAttribute {
	std::string name;
	SHADER_TYPE type;
	u32 size;
	size_t offset;
	bool normalized;

	VertexAttribute(const std::string& name, SHADER_TYPE type, bool normalized = false);
};

class VertexFormat {
public:


public:
	VertexFormat() = default;
	VertexFormat(const std::initializer_list<VertexAttribute>& attributes);
	VertexFormat(const VertexFormat&) = default;
	auto operator=(const VertexFormat&)->VertexFormat & = default;
	auto calculate_offset_and_stride(std::vector<VertexAttribute>& attributes) -> void;

	std::vector<VertexAttribute> m_attributes;
	u8 m_stride = 0;

};

class IShader {
public:

	struct DESC {
		SHADING_LANGUAGE shading_language;
		ShadingCode code;
		VertexFormat buffer_layout;
	};
public:
};
}