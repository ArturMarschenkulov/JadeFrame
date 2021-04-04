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

	auto add_to_data(const VertexData& vertex_data) -> void;
};
class VertexDataFactory {
public:
	static auto make_line(const Vec3& pos1, const Vec3& pos2) -> VertexData;

	static auto make_rectangle(const Vec3& pos, const Vec3& size)->VertexData;
	static auto make_triangle(const Vec3& pos1, const Vec3& pos2, const Vec3& pos3)->VertexData;
	static auto make_circle(const Vec3& position, const float radius, const int numSegments)->VertexData;

	static auto make_cube(const Vec3& pos, const Vec3& size)->VertexData;
};
