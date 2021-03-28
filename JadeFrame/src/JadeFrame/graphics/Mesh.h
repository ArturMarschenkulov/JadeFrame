#pragma once
#include "../math/Vec2.h"
#include "../math/Vec3.h"
#include "../math/Mat4.h"

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

class VertexDataFactory {
public:
	static auto make_line(const Vec3& pos1, const Vec3& pos2) -> VertexData;

	static auto make_rectangle(const Vec3& pos, const Vec3& size)->VertexData;
	static auto make_triangle(const Vec3& pos1, const Vec3& pos2, const Vec3& pos3)->VertexData;
	static auto make_circle(const Vec3& position, const float radius, const int numSegments)->VertexData;

	static auto make_cube(const Vec3& pos, const Vec3& size)->VertexData;
};
