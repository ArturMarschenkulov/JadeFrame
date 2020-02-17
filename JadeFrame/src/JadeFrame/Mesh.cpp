#include "Mesh.h"
#include "BaseApp.h"

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

Mesh MeshManager::makeCircle(Vec3 position, float radius, int numSegments) {
	
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

	GLuint numIndex = (numSegments * 3);
	mesh.indices.resize(numIndex);

	for(int i = 0; i < numSegments; i++) {
		mesh.indices[(3 * i + 0)] = 0;
		mesh.indices[(3 * i + 1)] = 1 + i;
		mesh.indices[(3 * i + 2)] = 2 + i;
	}
	mesh.indices[numIndex - 1] = mesh.indices[1];

	return mesh;
}

void RectangleMesh::draw() {
	Mesh mesh = MeshManager::makeRectangle(position, size);
	BaseApp::getAppInstance()->renderer.bufferData.add(mesh);
}

void TriangleMesh::draw() {
	Mesh mesh = MeshManager::makeTriangle(pos1, pos2, pos3);
	BaseApp::getAppInstance()->renderer.bufferData.add(mesh);
}

void CircleMesh::draw() {
	Mesh mesh = MeshManager::makeCircle(position, radius, numSegment);
	BaseApp::getAppInstance()->renderer.bufferData.add(mesh);
}
