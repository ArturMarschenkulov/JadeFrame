#include "Mesh.h"
#include "../BaseApp.h"
auto MeshManager::make_line(Vec3 pos1, Vec3 pos2) -> Mesh3 {
	Mesh3 mesh;
	mesh.vertices.resize(2);
	mesh.vertices[0].position = pos1;
	mesh.vertices[1].position = pos2;

	mesh.vertices[0].tex_coord = { 0.0f, 0.0f };
	mesh.vertices[1].tex_coord = { 0.0f, 0.0f };

	mesh.indices.reserve(2);
	mesh.indices = {
		0, 1
	};
	return mesh;
}

auto MeshManager::make_rectangle(Vec3 pos, Vec3 size) -> Mesh3 {
	Mesh3 mesh;
	mesh.vertices.resize(4);
	mesh.vertices[0].position = Vec3{ pos.x, pos.y, pos.z };
	mesh.vertices[1].position = Vec3{ pos.x + size.x, pos.y, pos.z };
	mesh.vertices[2].position = Vec3{ pos.x + size.x, pos.y + size.y, pos.z };
	mesh.vertices[3].position = Vec3{ pos.x, pos.y + size.y, pos.z };

	mesh.vertices[0].tex_coord = { 1.0, 1.0 };
	mesh.vertices[1].tex_coord = { 1.0, 0.0 };
	mesh.vertices[2].tex_coord = { 0.0, 0.0 };
	mesh.vertices[3].tex_coord = { 0.0, 1.0 };


	mesh.indices.reserve(6);
	mesh.indices = {
		0, 1, 3,
		1, 2, 3
	};
	return mesh;

}

auto MeshManager::make_triangle(Vec3 pos1, Vec3 pos2, Vec3 pos3) -> Mesh3 {
	Mesh3 mesh;
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

auto MeshManager::make_circle(Vec3 position, float radius, int numSegments) -> Mesh3 {

	float theta = 2.0f * 3.1415926f / float(numSegments);//get the current angle 
	float cos = cosf(theta);//calculate the x component 
	float sin = sinf(theta);//calculate the y component 
	float x = radius;
	float y = 0;
	float t;
	Mesh3 mesh;
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

auto MeshManager::make_cube(Vec3 pos, Vec3 size) -> Mesh3 {
	Mesh3 mesh;
	mesh.vertices.resize(8);
	mesh.vertices[0].position = { pos.x    , pos.y    , pos.z };
	mesh.vertices[1].position = { pos.x    , pos.y + size.y, pos.z };
	mesh.vertices[2].position = { pos.x + size.x, pos.y + size.y, pos.z };
	mesh.vertices[3].position = { pos.x + size.x, pos.y    , pos.z };

	mesh.vertices[4].position = { pos.x + size.x, pos.y    , pos.z - size.z };
	mesh.vertices[5].position = { pos.x + size.x, pos.y + size.y, pos.z - size.z };
	mesh.vertices[6].position = { pos.x    , pos.y + size.y, pos.z - size.z };
	mesh.vertices[7].position = { pos.x    , pos.y    , pos.z - size.z };

	mesh.vertices[0].tex_coord = { 1.0, 1.0 };
	mesh.vertices[1].tex_coord = { 1.0, 0.0 };
	mesh.vertices[2].tex_coord = { 0.0, 0.0 };
	mesh.vertices[3].tex_coord = { 0.0, 1.0 };
	mesh.vertices[4].tex_coord = { 1.0, 1.0 };
	mesh.vertices[5].tex_coord = { 1.0, 0.0 };
	mesh.vertices[6].tex_coord = { 0.0, 0.0 };
	mesh.vertices[7].tex_coord = { 0.0, 1.0 };


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



class Material {
	GLShader m_shader;
};

class Model {
	std::vector<Mesh> m_meshes;
	std::vector<Material> m_materials;
};

class Scene {
	std::vector<Model> m_models;
	
};

auto VertexDataFactory::make_line(Vec3 pos1, Vec3 pos2) -> VertexData {
	VertexData vertex_data;
	vertex_data.positions.resize(2);
	vertex_data.positions[0] = pos1;
	vertex_data.positions[1] = pos2;

	vertex_data.tex_coords.resize(2);
	vertex_data.tex_coords[0] = { 0.0f, 0.0f };
	vertex_data.tex_coords[1] = { 0.0f, 0.0f };

	vertex_data.indices.reserve(2);
	vertex_data.indices = {
		0, 1
	};
	return vertex_data;
}

auto VertexDataFactory::make_rectangle(Vec3 pos, Vec3 size) -> VertexData {
	VertexData vertex_data;
	vertex_data.positions.resize(4);
	vertex_data.positions[0] = Vec3{ pos.x, pos.y, pos.z };
	vertex_data.positions[1] = Vec3{ pos.x + size.x, pos.y, pos.z };
	vertex_data.positions[2] = Vec3{ pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.positions[3] = Vec3{ pos.x, pos.y + size.y, pos.z };

	vertex_data.tex_coords.resize(4);
	vertex_data.tex_coords[0] = { 1.0, 1.0 };
	vertex_data.tex_coords[1] = { 1.0, 0.0 };
	vertex_data.tex_coords[2] = { 0.0, 0.0 };
	vertex_data.tex_coords[3] = { 0.0, 1.0 };


	vertex_data.indices.reserve(6);
	vertex_data.indices = {
		0, 1, 3,
		1, 2, 3
	};
	return vertex_data;
}

auto VertexDataFactory::make_triangle(Vec3 pos1, Vec3 pos2, Vec3 pos3) -> VertexData {
	VertexData vertex_data;
	vertex_data.positions.resize(3);
	vertex_data.positions[0] = Vec3{ pos1.x, pos1.y, pos1.z };
	vertex_data.positions[1] = Vec3{ pos2.x, pos2.y, pos2.z };
	vertex_data.positions[2] = Vec3{ pos3.x, pos3.y, pos3.z };

	vertex_data.indices.reserve(3);
	vertex_data.indices = {
		0, 1, 2
	};
	return vertex_data;
}

auto VertexDataFactory::make_circle(Vec3 position, float radius, int numSegments) -> VertexData {
	float theta = 2.0f * 3.1415926f / float(numSegments);//get the current angle 
	float cos = cosf(theta);//calculate the x component 
	float sin = sinf(theta);//calculate the y component 
	float x = radius;
	float y = 0;
	float t;
	VertexData vertex_data;
	vertex_data.positions.resize(numSegments + 1);
	vertex_data.positions[0] = position;

	for (int i = 1; i < numSegments + 1; i++) {
		vertex_data.positions[i] = Vec3{ x + position.x, y + position.y, position.z };//output vertex 

		t = x;
		x = cos * x - sin * y;
		y = sin * t + cos * y;
	}

	GLuint num_index = (numSegments * 3);
	vertex_data.indices.resize(num_index);

	for (int i = 0; i < numSegments; i++) {
		vertex_data.indices[(3 * i + 0)] = 0;
		vertex_data.indices[(3 * i + 1)] = 1 + i;
		vertex_data.indices[(3 * i + 2)] = 2 + i;
	}
	vertex_data.indices[num_index - 1] = vertex_data.indices[1];

	return vertex_data;
}

auto VertexDataFactory::make_cube(Vec3 pos, Vec3 size) -> VertexData {
	VertexData vertex_data;
	vertex_data.positions.resize(8);
	vertex_data.positions[0] = { pos.x    , pos.y    , pos.z };
	vertex_data.positions[1] = { pos.x    , pos.y + size.y, pos.z };
	vertex_data.positions[2] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.positions[3] = { pos.x + size.x, pos.y    , pos.z };

	vertex_data.positions[4] = { pos.x + size.x, pos.y    , pos.z - size.z };
	vertex_data.positions[5] = { pos.x + size.x, pos.y + size.y, pos.z - size.z };
	vertex_data.positions[6] = { pos.x    , pos.y + size.y, pos.z - size.z };
	vertex_data.positions[7] = { pos.x    , pos.y    , pos.z - size.z };

	vertex_data.tex_coords.resize(8);
	vertex_data.tex_coords[0] = { 1.0, 1.0 };
	vertex_data.tex_coords[1] = { 1.0, 0.0 };
	vertex_data.tex_coords[2] = { 0.0, 0.0 };
	vertex_data.tex_coords[3] = { 0.0, 1.0 };
	vertex_data.tex_coords[4] = { 1.0, 1.0 };
	vertex_data.tex_coords[5] = { 1.0, 0.0 };
	vertex_data.tex_coords[6] = { 0.0, 0.0 };
	vertex_data.tex_coords[7] = { 0.0, 1.0 };


	vertex_data.indices.resize(36);
	vertex_data.indices = {
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
	return vertex_data;
}