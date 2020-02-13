#include "Renderer.h"
#include "BaseApp.h"





constexpr int MAX_VERTICES_FOR_BATCH = 100000;

Renderer::Renderer() {}
void Renderer::init(Shader* shader) {
	currentShader = shader;

	glCreateBuffers(1, &bufferData.VBO);
	glCreateVertexArrays(1, &bufferData.VAO);
	glCreateBuffers(1, &bufferData.IBO);



	bufferData.vertices.resize(MAX_VERTICES_FOR_BATCH);
	bufferData.indices.resize(MAX_VERTICES_FOR_BATCH);

	glBufferData(GL_ARRAY_BUFFER, bufferData.vertices.size() * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, bufferData.indices.size() * sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);

}
void Renderer::setColor(const Color& color) {
	currentColor = color;
}
void Renderer::drawRectangle(Vec2 pos, Vec2 size) {
	Mesh rectMesh = MeshManager::makeRectangle(pos, size);
	addToVertexData(rectMesh);
}
void Renderer::drawTriangle(Vec3 pos1, Vec3 pos2, Vec3 pos3) {
	Mesh triangleMesh = MeshManager::makeTriangle(pos1, pos2, pos3);
	addToVertexData(triangleMesh);
}
void Renderer::addToVertexData(Mesh& mesh) {


	for(unsigned int i = 0; i < mesh.vertices.size(); i++) {
		mesh.vertices[i].color = currentColor;
		bufferData.vertices[i + bufferData.vertexOffset] = mesh.vertices[i];
		bufferData.vertexCount++;
	}



	for(int i = 0; i < mesh.indices.size(); i++) {
		bufferData.indices[i + bufferData.indexOffset] = mesh.indices[i] + bufferData.vertexOffset;
		bufferData.indexCount++;
	}

	bufferData.vertexOffset += mesh.vertices.size();
	bufferData.indexOffset += mesh.indices.size();
}

void Renderer::startDraw() {
	proj = Mat4();
	view = Mat4();
	model = Mat4();

	currentShader->use();
	//glBindVertexArray(bufferData.VAO);


}
void Renderer::drawBuffers() {
	Mat4 MVP = proj * view * model;
	currentShader->setUniformMatrix4fv("MVP", MVP);


	glBindBuffer(GL_ARRAY_BUFFER, bufferData.VBO);
	GLuint vertexBufferSizeInBytes = bufferData.vertexCount * sizeof(Vertex);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSizeInBytes, bufferData.vertices.data(), GL_DYNAMIC_DRAW);


	glBindVertexArray(bufferData.VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferData.IBO);
	GLuint indexBufferSizeInBytes = bufferData.indexCount * sizeof(GLuint);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSizeInBytes, bufferData.indices.data(), GL_DYNAMIC_DRAW);



	glDrawElements(GL_TRIANGLES, bufferData.indexCount, GL_UNSIGNED_INT, 0);



	bufferData.vertexOffset = 0;
	bufferData.indexOffset = 0;
	bufferData.vertexCount = 0;
	bufferData.indexCount = 0;
}
void Renderer::endDraw() {

	drawBuffers();

	// Frame time control system
	currentTime = glfwGetTime();
	drawTime = currentTime - previousTime;
	previousTime = currentTime;

	frameTime = updateTime + drawTime;

	// Wait for some milliseconds...
	if(frameTime < targetTime) {
		Sleep((unsigned int)((float)(targetTime - frameTime) * 1000.0f));
		currentTime = glfwGetTime();
		double extraTime = currentTime - previousTime;
		previousTime = currentTime;
		frameTime += extraTime;
	}

}


void Renderer::ortho(float left, float right, float buttom, float top, float zNear, float zFar) {
	proj = Mat4::ortho(left, right, buttom, top, zNear, zFar);
}
