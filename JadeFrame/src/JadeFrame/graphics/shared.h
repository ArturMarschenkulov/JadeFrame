#pragma once
#include "JadeFrame/defines.h"
#include <JadeFrame/math/mat_4.h>
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
/*
	TODO: Consider whether this is a good way and whether it is worth it to introdcue inheritance.
		Right now, inheritance should be mainly used as a sanity check such that all renderers have a common interface.
*/
class IRenderer {
public: // client stuff
	virtual auto submit(const Object& obj) -> void = 0;
	virtual auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void = 0;
	
	virtual auto take_screenshot(const char* filename) -> void = 0;

	virtual auto main_loop() -> void = 0;

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


class BufferLayout {
public:
	struct BufferElement {
		std::string name;
		SHADER_TYPE type;
		u32 size;
		size_t offset;
		bool normalized;

		BufferElement(SHADER_TYPE type, const std::string& name, bool normalized = false);
	};

public:
	BufferLayout() = default;
	BufferLayout(const std::initializer_list<BufferElement>& elements);
	BufferLayout(const BufferLayout&) = default;
	auto operator=(const BufferLayout&)->BufferLayout & = default;
	auto calculate_offset_and_stride(std::vector<BufferElement>& elements) -> void;

	std::vector<BufferElement> m_elements;
	u8 m_stride = 0;

};
}