#include "Mesh.h"
#include "../BaseApp.h"
const Color EColor::RED = { 255, 0, 0 };
const Color EColor::GREEN = { 0, 255, 0 };
const Color EColor::BLUE = { 0, 0, 255 };
const Color EColor::YELLOW = { 255, 255, 0 };
const Color EColor::CYAN = { 0, 255, 255 };
const Color EColor::MAGENTA = { 255, 0, 255 };
const Color EColor::BLACK = { 255, 255, 255 };
const Color EColor::WHITE = { 0, 0, 0 };
auto MeshManager::make_line(Vec3 pos1, Vec3 pos2) -> Mesh {
	Mesh mesh;
	mesh.vertices.resize(2);
	mesh.vertices[0].position = pos1;
	mesh.vertices[1].position = pos2;

	mesh.indices.reserve(2);
	mesh.indices = {
		0, 1
	};
	return mesh;
}

auto MeshManager::make_rectangle(Vec3 pos, Vec3 size) -> Mesh {
	Mesh mesh;
	mesh.vertices.resize(4);
	mesh.vertices[0].position = Vec3{ pos.x, pos.y, pos.z };
	mesh.vertices[1].position = Vec3{ pos.x + size.x, pos.y, pos.z };
	mesh.vertices[2].position = Vec3{ pos.x + size.x, pos.y + size.y, pos.z };
	mesh.vertices[3].position = Vec3{ pos.x, pos.y + size.y, pos.z };



	mesh.indices.reserve(6);
	mesh.indices = {
		0, 1, 3,
		1, 2, 3
	};
	return mesh;

}

auto MeshManager::make_triangle(Vec3 pos1, Vec3 pos2, Vec3 pos3) -> Mesh {
	Mesh mesh;
	mesh.vertices.resize(3);
	mesh.vertices[0].position = Vec3{ pos1.x, pos1.y, pos1.z };
	mesh.vertices[1].position = Vec3{ pos2.x, pos2.y, pos2.z };
	mesh.vertices[2].position = Vec3{ pos3.x, pos3.y, pos3.z };

	mesh.indices.reserve(3);
	mesh.indices = {
		0, 1, 2
	};
	return mesh;
}

auto MeshManager::make_circle(Vec3 position, float radius, int numSegments) -> Mesh {

	float theta = 2.0f * 3.1415926f / float(numSegments);//get the current angle 
	float cos = cosf(theta);//calculate the x component 
	float sin = sinf(theta);//calculate the y component 
	float x = radius;
	float y = 0;
	float t;
	Mesh mesh;
	mesh.vertices.resize(numSegments + 1);
	mesh.vertices[0].position = position;

	for(int i = 1; i < numSegments + 1; i++) {
		mesh.vertices[i].position = Vec3{ x + position.x, y + position.y, position.z };//output vertex 

		t = x;
		x = cos * x - sin * y;
		y = sin * t + cos * y;
	}

	GLuint num_index = (numSegments * 3);
	mesh.indices.resize(num_index);

	for(int i = 0; i < numSegments; i++) {
		mesh.indices[(3 * i + 0)] = 0;
		mesh.indices[(3 * i + 1)] = 1 + i;
		mesh.indices[(3 * i + 2)] = 2 + i;
	}
	mesh.indices[num_index - 1] = mesh.indices[1];

	return mesh;
}

auto MeshManager::make_cube(Vec3 pos, Vec3 size) -> Mesh {
	Mesh mesh;
	mesh.vertices.resize(8);
	mesh.vertices[0].position = { pos.x    , pos.y    , pos.z };
	mesh.vertices[1].position = { pos.x    , pos.y + size.y, pos.z };
	mesh.vertices[2].position = { pos.x + size.x, pos.y + size.y, pos.z };
	mesh.vertices[3].position = { pos.x + size.x, pos.y    , pos.z };

	mesh.vertices[4].position = { pos.x + size.x, pos.y    , pos.z - size.z };
	mesh.vertices[5].position = { pos.x + size.x, pos.y + size.y, pos.z - size.z };
	mesh.vertices[6].position = { pos.x    , pos.y + size.y, pos.z - size.z };
	mesh.vertices[7].position = { pos.x    , pos.y    , pos.z - size.z };

	mesh.indices.resize(36);
	mesh.indices = {
		// Front
		0, 1, 3,
		1, 2, 3,

		// Right
		3, 2, 4,
		2, 5, 4,

		// Back
		4, 5, 7,
		5, 6, 7,

		// Top
		7, 0, 4,
		0, 3, 4,

		// Left
		7, 6, 0,
		6, 1, 0,

		// Buttom
		1, 6, 2,
		6, 5, 2
	};
	return mesh;
}
