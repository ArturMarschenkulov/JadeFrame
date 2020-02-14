#include "Mesh.h"

Mesh MeshManager::makeRectangle(Vec3 pos, Vec3 size) {
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

Mesh MeshManager::makeTriangle(Vec3 pos1, Vec3 pos2, Vec3 pos3) {
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
