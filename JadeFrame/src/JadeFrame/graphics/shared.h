#pragma once
#include "JadeFrame/defines.h"
#include <JadeFrame/math/mat_4.h>
//#include "Mesh.h"

namespace JadeFrame {
/*
	TODO: Consider whether this is a good way and whether it is worth it to introdcue inheritance.
		Right now, inheritance should be mainly used as a sanity check such that all renderers have a common interface.
*/
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
}