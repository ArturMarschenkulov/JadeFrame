#pragma once
#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/vec_3.h"
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

public:
	f32 r, g, b, a;
};


struct Vertex {
	Vec3 position;
	RGBAColor color;
	Vec2 tex_coord;
};

struct v3f32;
class VertexData {
public:
	std::vector<Vec3> m_positions;
	std::vector<RGBAColor> m_colors;
	std::vector<Vec2> m_texture_coordinates;
	std::vector<Vec3> m_normals;

	std::vector<u32> m_indices;
};

inline auto convert_into_data(const VertexData& vertex_data, const bool interleaved, bool with_color = true) -> std::vector<f32> {
	//assert(mesh.m_positions.size() == mesh.m_normals.size());
	const u64 size
		= vertex_data.m_positions.size() * 3
		+ vertex_data.m_colors.size() * 4
		+ vertex_data.m_texture_coordinates.size() * 2
		+ vertex_data.m_normals.size() * 3;

	std::vector<f32> data;
	data.reserve(size);
	if (interleaved == true) {
		for (size_t i = 0; i < vertex_data.m_positions.size(); i++) {
			data.push_back(vertex_data.m_positions[i].x);
			data.push_back(vertex_data.m_positions[i].y);
			data.push_back(vertex_data.m_positions[i].z);
			if (vertex_data.m_colors.size()) {
				data.push_back(vertex_data.m_colors[i].r);
				data.push_back(vertex_data.m_colors[i].g);
				data.push_back(vertex_data.m_colors[i].b);
				data.push_back(vertex_data.m_colors[i].a);
			}
			if (vertex_data.m_texture_coordinates.size()) {
				data.push_back(vertex_data.m_texture_coordinates[i].x);
				data.push_back(vertex_data.m_texture_coordinates[i].y);
			}
			if (vertex_data.m_normals.size()) {
				data.push_back(vertex_data.m_normals[i].x);
				data.push_back(vertex_data.m_normals[i].y);
				data.push_back(vertex_data.m_normals[i].z);
			}
		}
	} else {

		__debugbreak();
		for (size_t i = 0; i < vertex_data.m_positions.size(); i++) {
			data.push_back(vertex_data.m_positions[i].x);
			data.push_back(vertex_data.m_positions[i].y);
			data.push_back(vertex_data.m_positions[i].z);
		}
		for (size_t i = 0; i < vertex_data.m_colors.size(); i++) {
			data.push_back(vertex_data.m_colors[i].r);
			data.push_back(vertex_data.m_colors[i].g);
			data.push_back(vertex_data.m_colors[i].b);
			data.push_back(vertex_data.m_colors[i].a);
		}
		for (size_t i = 0; i < vertex_data.m_texture_coordinates.size(); i++) {
			data.push_back(vertex_data.m_texture_coordinates[i].x);
			data.push_back(vertex_data.m_texture_coordinates[i].y);
		}
	}

	return data;
}


class VertexDataFactory {
public:
	struct DESC {
		bool has_position = true; //NOTE: Probably unneccessary
		bool has_texture_coordinates = true;
		bool has_indices = false;
		bool has_normals = true;
	};
	static auto make_line(const Vec3& pos1, const Vec3& pos2)->VertexData;

	static auto make_rectangle(const Vec3& pos, const Vec3& size, const DESC desc = DESC{})->VertexData;
	static auto make_triangle(const Vec3& pos1, const Vec3& pos2, const Vec3& pos3)->VertexData;
	static auto make_circle(const Vec3& position, const f32 radius, const u32 numSegments)->VertexData;

	static auto make_cube(const Vec3& pos, const Vec3& size)->VertexData;
};
}