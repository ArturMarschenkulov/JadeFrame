#pragma once
#include "JadeFrame/math/vec.h"
#include "JadeFrame/math/mat_4.h"

#include <vector>

namespace JadeFrame {

class RGBAColor {
public:
	RGBAColor() = default;
	RGBAColor(f32 r, f32 g, f32 b, f32 a)
		: r(r)
		, g(g)
		, b(b)
		, a(a) {
	}
	RGBAColor(u8 r, u8 g, u8 b, u8 a)
		: r(r / 256.0f)
		, g(g / 256.0f)
		, b(b / 256.0f)
		, a(a / 256.0f) {

	}
	auto operator==(const RGBAColor& color) const -> bool {
		return r == color.r && g == color.g && b == color.b && a == color.a;
	}
	auto operator!=(const RGBAColor& color) const -> bool {
		return !(*this == color);
	}

	static auto solid_black() -> RGBAColor {
		return RGBAColor(0.0f, 0.0f, 0.0f, 1.0f);
	}
	static auto solid_white() -> RGBAColor {
		return RGBAColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	static auto solid_red() -> RGBAColor {
		return RGBAColor(1.0f, 0.0f, 0.0f, 1.0f);
	}
	static auto solid_green() -> RGBAColor {
		return RGBAColor(0.0f, 1.0f, 0.0f, 1.0f);
	}
	static auto solid_blue() -> RGBAColor {
		return RGBAColor(0.0f, 0.0f, 1.0f, 1.0f);
	}

	static auto solid_yellow() -> RGBAColor {
		return RGBAColor(1.0f, 1.0f, 0.0f, 1.0f);
	}
	static auto solid_cyan() -> RGBAColor {
		return RGBAColor(0.0f, 1.0f, 1.0f, 1.0f);
	}
	static auto solid_magenta() -> RGBAColor {
		return RGBAColor(1.0f, 0.0f, 1.0f, 1.0f);
	}

	static auto solid_transparent() -> RGBAColor {
		return RGBAColor(0.0f, 0.0f, 0.0f, 0.0f);
	}
	auto set_opacity(f32 opacity) -> RGBAColor {
		return RGBAColor(r, g, b, opacity);
	}

public:
	f32 r, g, b, a;
};


struct Vertex {
	v3 position;
	RGBAColor color;
	v2 tex_coord;
};

//struct v3f32;
class VertexData {
public:
	std::vector<v3> m_positions;
	std::vector<RGBAColor> m_colors;
	std::vector<v2> m_texture_coordinates;
	std::vector<v3> m_normals;

	std::vector<u32> m_indices;
};

auto convert_into_data(const VertexData& vertex_data, const bool interleaved) -> std::vector<f32>;


class VertexDataFactory {
public:
	struct Desc {
		bool has_position = true; //NOTE: Probably unneccessary
		bool has_texture_coordinates = true;
		bool has_indices = false;
		bool has_normals = true;
	};
	static auto make_line(const v3& pos1, const v3& pos2)->VertexData;

	static auto make_rectangle(const v3& pos, const v3& size, const Desc desc = Desc{ true, true, false, true })->VertexData;
	static auto make_triangle(const v3& pos1, const v3& pos2, const v3& pos3)->VertexData;
	static auto make_circle(const v3& position, const f32 radius, const u32 numSegments)->VertexData;

	static auto make_cube(const v3& pos, const v3& size)->VertexData;
};
}