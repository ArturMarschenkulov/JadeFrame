#pragma once
#include <glad/glad.h>

#include <vector>

#include "../math/Vec2.h"
#include "../math/Vec3.h"
#include "../math/Mat4.h"



class Color {
public:
	float r, g, b, a;

	bool operator==(const Color& color) const {
		return r == color.r && g == color.g && b == color.b && a == color.a;
	}
	bool operator!=(const Color& color) const {
		return r != color.r || g != color.g || b != color.b || a != color.a;
	}
};
class EColor {
public:
	static const Color RED;
	static const Color GREEN;
	static const Color BLUE;
	static const Color YELLOW;
	static const Color CYAN;
	static const Color MAGENTA;
	static const Color BLACK;
	static const Color WHITE;
};



struct Vertex {
	Vec3 position;
	Color color;
};




class Mesh {
public:
	//private:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	int vertex_offset;
	Vertex* vertex_buffer_ptr;
	bool is_dirty;
};

class MeshManager {
public:
	//use PRIMITIVE_TYPE::LINE
	static auto make_line(Vec3 pos1, Vec3 pos2)->Mesh;

	static auto make_rectangle(Vec3 position, Vec3 size)->Mesh;
	static auto make_triangle(Vec3 pos1, Vec3 pos2, Vec3 pos3)->Mesh;
	static auto make_circle(Vec3 position, float radius, int numSegments)->Mesh;

	static auto make_cube(Vec3 start, Vec3 end)->Mesh;
};