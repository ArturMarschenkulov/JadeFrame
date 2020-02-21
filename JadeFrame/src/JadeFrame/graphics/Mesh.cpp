#include "Mesh.h"
#include "../BaseApp.h"

Mesh MeshManager::make_rectangle(Vec3 pos, Vec3 size) {
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

Mesh MeshManager::make_triangle(Vec3 pos1, Vec3 pos2, Vec3 pos3) {
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

Mesh MeshManager::make_circle(Vec3 position, float radius, int numSegments) {

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



RectangleMesh::RectangleMesh(Vec2 position, Vec2 size) {
	mesh = MeshManager::make_rectangle(position, size);
	//if(BaseApp::getAppInstance()->renderer.matrixStack.useTransformMatrix == true) {
	//	Mat4 m = BaseApp::getAppInstance()->renderer.matrixStack.transformMatrix;
	//	for(int i = 0; i < 4; i++) {
	//		mesh.vertices[i].position = m * mesh.vertices[i].position;
	//	}
	//}
}
void RectangleMesh::send_to_buffer() {

	//if(BaseApp::getAppInstance()->renderer.matrixStack.useTransformMatrix == true) {
	//	Mat4 m = BaseApp::getAppInstance()->renderer.matrixStack.transformMatrix;
	//	for(int i = 0; i < 4; i++) {
	//		mesh.vertices[i].position = m * mesh.vertices[i].position;
	//	}
	//}
	BaseApp::getAppInstance()->renderer.handle_mesh(mesh);
}

TriangleMesh::TriangleMesh(Vec2 pos1, Vec2 pos2, Vec3 pos3) {
	mesh = MeshManager::make_triangle(pos1, pos2, pos3);
}
void TriangleMesh::send_to_buffer() {
	BaseApp::getAppInstance()->renderer.handle_mesh(mesh);
}

CircleMesh::CircleMesh(Vec2 position, float radius, int numSegments) {
	mesh = MeshManager::make_circle(position, radius, numSegments);
}
void CircleMesh::send_to_buffer() {
	BaseApp::getAppInstance()->renderer.handle_mesh(mesh);
}
