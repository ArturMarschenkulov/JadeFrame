#include "Renderer.h"
#include "BaseApp.h"




constexpr int MAX_BATCH_QUADS = 100000;
constexpr int MAX_VERTICES_FOR_BATCH = 4 * MAX_BATCH_QUADS;
constexpr int MAX_INDICES_FOR_BATCH = 6 * MAX_BATCH_QUADS;
Renderer::Renderer() {}
void Renderer::init(Shader* shader) {

	currentShader = shader;
	currentShader->use();
	//CPU
	bufferData.vertices.resize(MAX_VERTICES_FOR_BATCH);
	bufferData.indices.resize(MAX_INDICES_FOR_BATCH);

	//GPU
	glCreateBuffers(1, &bufferData.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, bufferData.VBO);
	GLuint vertexBufferSizeInBytes = bufferData.vertices.size() * sizeof(Vertex);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSizeInBytes, NULL, GL_DYNAMIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(Vertex) * 1000, 0, GL_STATIC_DRAW);


	glCreateVertexArrays(1, &bufferData.VAO);
	glBindVertexArray(bufferData.VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(0 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glCreateBuffers(1, &bufferData.IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferData.IBO);
	GLuint indexBufferSizeInBytes = bufferData.indices.size() * sizeof(GLuint);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSizeInBytes, NULL, GL_DYNAMIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLfloat) * 1000, 0, GL_STATIC_DRAW);


	glBindVertexArray(0);
}
void Renderer::setColor(const Color& color) {
	currentColor = color;
}
void Renderer::drawRectangle(Vec2 pos, Vec2 size) {
	Mesh rectMesh = MeshManager::makeRectangle(pos, size);
	addToBufferData(rectMesh);
}
void Renderer::drawTriangle(Vec3 pos1, Vec3 pos2, Vec3 pos3) {
	Mesh triangleMesh = MeshManager::makeTriangle(pos1, pos2, pos3);
	addToBufferData(triangleMesh);
}
void Renderer::addToBufferData(Mesh& mesh) {


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

void Renderer::start() {
	proj = Mat4();
	view = Mat4();
	model = Mat4();

	currentShader->use();
	//glBindVertexArray(bufferData.VAO);


}
void Renderer::drawBuffers() {
	currentShader->use();
	glBindVertexArray(bufferData.VAO);
	glDrawElements(GL_TRIANGLES, bufferData.indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	bufferData.vertexOffset = 0;
	bufferData.indexOffset = 0;
	bufferData.vertexCount = 0;
	bufferData.indexCount = 0;
}
void Renderer::end() {
	this->updateBuffers();
	this->drawBuffers();
}

void Renderer::updateBuffers() {

	Mat4 MVP = proj * view * model;
	currentShader->setUniformMatrix4fv("MVP", MVP);

	glBindVertexArray(bufferData.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, bufferData.VBO);
	GLuint vertexBufferSizeInBytes = bufferData.vertexCount * sizeof(Vertex);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBufferSizeInBytes, bufferData.vertices.data());



	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferData.IBO);
	GLuint indexBufferSizeInBytes = bufferData.indexCount * sizeof(GLuint);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexBufferSizeInBytes, bufferData.indices.data());

	glBindVertexArray(0);
}


void Renderer::ortho(float left, float right, float buttom, float top, float zNear, float zFar) {
	proj = Mat4::ortho(left, right, buttom, top, zNear, zFar);
}

void Renderer::handleTime() {
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
