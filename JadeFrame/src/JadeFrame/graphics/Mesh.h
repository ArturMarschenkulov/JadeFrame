#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "../math/Vec2.h"
#include "../math/Vec3.h"


class Color {
public:
	float r, g, b, a;

	bool operator==(const Color& color) const {
		return r == color.r && g == color.g && b == color.b && a == color.a;
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
};

class MeshManager {
public:
	static Mesh make_rectangle(Vec3 position, Vec3 size);
	static Mesh make_triangle(Vec3 pos1, Vec3 pos2, Vec3 pos3);
	static Mesh make_circle(Vec3 position, float radius, int numSegments);

};
class RectangleMesh {
public:
	RectangleMesh(Vec2 position, Vec2 size);

	void send_to_buffer();
	Mesh mesh;
};
class TriangleMesh {
public:
	TriangleMesh(Vec2 pos1, Vec2 pos2, Vec3 pos3);
	void send_to_buffer();
	Mesh mesh;
};
class CircleMesh {
public:
	CircleMesh(Vec2 position, float radius, int numSegments = 12);
	void send_to_buffer();
	Mesh mesh;
};