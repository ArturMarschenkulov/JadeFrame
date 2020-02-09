#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"

#include <vector>
#include <iostream>
#include "math/Vec3.h"
#include "math/Vec2.h"
#include "math/Mat4.h"


struct Vertex {
	struct Vec3 {
		float x, y, z;
	};
	Vec3 position;
};
class DynamicBuffer {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	GLuint VBO = 0;
	GLuint VAO = 0;
	GLuint IBO = 0;

	GLuint vertexOffset;
	GLuint indexOffset;

	GLuint vertexCount;
	GLuint indexCount;

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
	static Mesh makeRectangle(Vec3 pos, Vec3 size) {
		Mesh mesh;
		mesh.vertices.reserve(4);
		mesh.vertices = {
			Vertex{ pos.x, pos.y, pos.z },
			Vertex{ pos.x + size.x, pos.y, pos.z },
			Vertex{ pos.x + size.x, pos.y + size.y, pos.z },
			Vertex{ pos.x, pos.y + size.y, pos.z },
		};
		mesh.indices.reserve(6);
		mesh.indices = {
			0, 1, 3,
			1, 2, 3
		};
		return mesh;
	}

	static Mesh makeTriangle(Vec3 pos1, Vec3 pos2, Vec3 pos3) {
		Mesh mesh;
		mesh.vertices.reserve(3);
		mesh.vertices = {
			{pos1.x, pos1.y, pos1.z},
			{pos2.x, pos2.y, pos2.z},
			{pos3.x, pos3.y, pos3.z}
		};
		mesh.indices.reserve(3);
		mesh.indices = {
			0, 1, 2
		};
		return mesh;
	}
};


class Renderer {
public:
	Renderer();
	void init(Shader* shader);


	void drawRectangle(Vec2 pos, Vec2 size);
	void draw(Mesh& mesh);

	void startDraw();
	void endDraw();

	DynamicBuffer vertexData;

	Shader* currentShader = nullptr;
};