#pragma once
#include "../math/vec_2.h"
#include "../math/vec_3.h"
#include "../math/mat_4.h"

#include <vector>

class Color {
public:
	float r, g, b, a;

	auto operator==(const Color& color) const -> bool {
		return r == color.r && g == color.g && b == color.b && a == color.a;
	}
	auto operator!=(const Color& color) const -> bool {
		return !(*this == color);
	}
};

struct Vertex {
	Vec3 position;
	Color color;
	Vec2 tex_coord;
};


class VertexData {
public:
	std::vector<Vec3> positions;
	std::vector<Color> colors;
	std::vector<Vec2> tex_coords;
	std::vector<Vec3> normals;

	std::vector<uint32_t> indices;
};


struct Mesh {
	std::vector<Vec3>	m_positions;
	std::vector<Color>	m_colors;
	std::vector<Vec2>	m_tex_coords;
	std::vector<Vec3> m_normals;
	std::vector<uint32_t> m_indices;

	Color current_color = { 0.5f, 0.5f, 0.5f, 1.0f };
	//PRIMITIVE_TYPE m_primitive_type = PRIMITIVE_TYPE::TRIANGLES;

	auto add_to_data(const VertexData& vertex_data) -> void {

		m_positions.resize(vertex_data.positions.size());
		m_colors.resize(vertex_data.positions.size());
		m_tex_coords.resize(vertex_data.tex_coords.size());
		for (size_t i = 0; i < vertex_data.positions.size(); i++) {
			m_positions[i] = vertex_data.positions[i];
			m_colors[i] = current_color;
			m_tex_coords[i] = vertex_data.tex_coords[i];
		}

		m_normals.resize(vertex_data.normals.size());
		for (size_t i = 0; i < vertex_data.normals.size(); i++) {
			m_normals[i] = vertex_data.normals[i];
		}



		m_indices.resize(vertex_data.indices.size());
		for (size_t i = 0; i < vertex_data.indices.size(); i++) {
			m_indices[i] = vertex_data.indices[i];
		}

	}
	auto set_color(const Color& color) -> void {
		current_color = color;
	}
};
class VertexDataFactory {
public:
	static auto make_line(const Vec3& pos1, const Vec3& pos2) -> VertexData;

	static auto make_rectangle(const Vec3& pos, const Vec3& size)->VertexData;
	static auto make_triangle(const Vec3& pos1, const Vec3& pos2, const Vec3& pos3)->VertexData;
	static auto make_circle(const Vec3& position, const float radius, const int numSegments)->VertexData;

	static auto make_cube(const Vec3& pos, const Vec3& size)->VertexData;
};
