#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "math/Vec2.h"
#include "math/Vec3.h"


class Color {
public:
	float r, g, b, a;
};

struct Vertex {
	Vec3 position;
	Color color;
};




class Mesh {
public:
	inline GLuint vertexBufferSize() const {
		return this->vertices.size() * sizeof(Vertex);
	};
	inline GLuint indexBufferSize() const {
		return this->indices.size() * sizeof(GLuint);
	};
	//public:

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
};

class MeshManager {
public:
	static Mesh makeRectangle(Vec3 position, Vec3 size);
	static Mesh makeTriangle(Vec3 pos1, Vec3 pos2, Vec3 pos3);
	static Mesh makeCircle(Vec3 position, float radius, int numSegments);

};
class RectangleMesh {
public:
	RectangleMesh()
		: position()
		, size() {

	}
	RectangleMesh(Vec2 position, Vec2 size)
		: position(position)
		, size(size) {

	}

	void draw();
	Vec2 position;
	Vec2 size;
};
class TriangleMesh {
public:
	TriangleMesh()
		: pos1()
		, pos2()
		, pos3(){

	}
	TriangleMesh(Vec2 pos1, Vec2 pos2, Vec3 pos3)
		: pos1(pos1)
		, pos2(pos2)
		, pos3(pos3){

	}

	void draw();
	Vec2 pos1, pos2, pos3;
};
class CircleMesh {
public:
	CircleMesh()
		: position()
		, radius()
		, numSegment(8){

	}
	CircleMesh(Vec2 position, int radius)
		: position(position)
		, radius(radius)
		, numSegment(8) {
	}
	CircleMesh(Vec2 position, float radius, int numSegments)
		: position(position)
		, radius(radius)
		, numSegment(numSegments) {
	}

	void draw();
	Vec2 position;
	int radius;
	int numSegment;
};