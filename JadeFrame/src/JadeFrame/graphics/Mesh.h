#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

	auto send_to_buffer() -> void;
};

class MeshManager {
public:
	//use PRIMITIVE_TYPE::LINE
	static auto make_line(Vec3 pos1, Vec3 pos2) -> Mesh;
			    
	static auto make_rectangle(Vec3 position, Vec3 size) ->Mesh;
	static auto make_triangle(Vec3 pos1, Vec3 pos2, Vec3 pos3) ->Mesh;
	static auto make_circle(Vec3 position, float radius, int numSegments) ->Mesh;

	static auto make_cube(Vec3 start, Vec3 end) ->Mesh;
};

class MeshPrototype {
	MeshPrototype();
	std::vector<Vertex> vertices; // actual vertex data
	std::vector<GLuint> indices; // actual index data
	int vertex_offset; // at what offset this mesh starts
	Vertex* vertex_buffer_ptr; // pointer to the vertex_buffer. Probably only needed if I have multiple buffers
	bool is_dirty; // says whether this mesh has to be resent to the GPU

};