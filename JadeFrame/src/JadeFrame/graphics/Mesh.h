#pragma once
#include <glad/glad.h>

#include <vector>

#include "../math/Vec2.h"
#include "../math/Vec3.h"
#include "../math/Mat4.h"



class Color {
public:
	float r, g, b, a;

	auto operator==(const Color& color) const -> bool {
		return r == color.r && g == color.g && b == color.b && a == color.a;
	}
	auto operator!=(const Color& color) const -> bool {
		return r != color.r || g != color.g || b != color.b || a != color.a;
	}
};

struct Vertex {
	Vec3 position;
	Color color;
	Vec2 tex_coord;
};




class Mesh3 {
public:
	//private:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
};

class VertexData {
public:
	std::vector<Vec3> positions;
	std::vector<Color> colors;
	std::vector<Vec2> tex_coords;

	std::vector<GLuint> indices;
};


class MeshManager {
public:
	//use PRIMITIVE_TYPE::LINE
	static auto make_line(Vec3 pos1, Vec3 pos2)->Mesh3;

	static auto make_rectangle(Vec3 position, Vec3 size)->Mesh3;
	static auto make_triangle(Vec3 pos1, Vec3 pos2, Vec3 pos3)->Mesh3;
	static auto make_circle(Vec3 position, float radius, int numSegments)->Mesh3;

	static auto make_cube(Vec3 start, Vec3 end)->Mesh3;
};

class VertexDataFactory {
public:
	//use PRIMITIVE_TYPE::LINE
	static auto make_line(Vec3 pos1, Vec3 pos2)->VertexData;

	static auto make_rectangle(Vec3 pos, Vec3 size)->VertexData;
	static auto make_triangle(Vec3 pos1, Vec3 pos2, Vec3 pos3)->VertexData;
	static auto make_circle(Vec3 position, float radius, int numSegments)->VertexData;

	static auto make_cube(Vec3 pos, Vec3 size)->VertexData;
};
