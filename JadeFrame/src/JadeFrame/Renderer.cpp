#include "Renderer.h"
#include "BaseApp.h"






Renderer::Renderer() {}
void Renderer::init(Shader* shader) {

	currentShader = shader;
	currentShader->use();

	bufferData.init();

}

void Renderer::start() {
	camera = Camera();

	currentShader->use();
}
void Renderer::end() {
	camera.update();
	bufferData.update();
	bufferData.draw();
	bufferData.resetCounters();
}

void Renderer::setColor(const Color& color) {
	currentColor = color;
}
void Renderer::drawRectangle(Vec2 pos, Vec2 size) {
	Mesh mesh = MeshManager::makeRectangle(pos, size);
	bufferData.add(mesh);
}
void Renderer::drawTriangle(Vec3 pos1, Vec3 pos2, Vec3 pos3) {
	Mesh mesh = MeshManager::makeTriangle(pos1, pos2, pos3);
	bufferData.add(mesh);
}



void Renderer::BufferData::init() {

	//CPU
	vertices.resize(MAX_VERTICES_FOR_BATCH);
	indices.resize(MAX_INDICES_FOR_BATCH);

	//GPU
	glCreateBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	GLuint vertexBufferSizeInBytes = vertices.size() * sizeof(Vertex);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSizeInBytes, NULL, GL_DYNAMIC_DRAW);


	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(1);

	glCreateBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	GLuint indexBufferSizeInBytes = indices.size() * sizeof(GLuint);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSizeInBytes, NULL, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);

}
void Renderer::BufferData::add(Mesh& mesh) {
	if((vertexCount + mesh.vertices.size() > MAX_VERTICES_FOR_BATCH) ||
		(indexCount + mesh.indices.size() > MAX_INDICES_FOR_BATCH)) {
		update();
		draw();
		resetCounters();
	}


	for(unsigned int i = 0; i < mesh.vertices.size(); i++) {
		//mesh.vertices[i].color = BaseApp::getAppInstance()->renderer.currentColor;;
		//bufferData.vertices[i + bufferData.vertexOffset] = mesh.vertices[i];
		vertices[i + vertexOffset].position = mesh.vertices[i].position;
		vertices[i + vertexOffset].color = BaseApp::getAppInstance()->renderer.currentColor;;
		
		BaseApp::getAppInstance()->renderer.currentColor;
		vertexCount++;
	}



	for(int i = 0; i < mesh.indices.size(); i++) {
		indices[i + indexOffset] = mesh.indices[i] + vertexOffset;
		indexCount++;
	}

	vertexOffset += mesh.vertices.size();
	indexOffset += mesh.indices.size();

}
void Renderer::BufferData::update() {

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	GLuint vertexBufferSizeInBytes = vertexCount * sizeof(Vertex);
	GLuint vertexBufferSizeInBytes2 = vertices.size() * sizeof(Vertex);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertexBufferSizeInBytes, vertices.data());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	GLuint indexBufferSizeInBytes = indexCount * sizeof(GLuint);
	GLuint indexBufferSizeInBytes2 = indices.size() * sizeof(GLuint);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexBufferSizeInBytes, indices.data());

	glBindVertexArray(0);
}
void Renderer::BufferData::draw() {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

void Renderer::BufferData::resetCounters() {
	vertexOffset = 0;
	indexOffset = 0;
	vertexCount = 0;
	indexCount = 0;
}

Camera::Camera() {
	proj = Mat4();
	view = Mat4();
	model = Mat4();
}

void Camera::update() {
	Mat4 MVP = proj * view * model;
	BaseApp::getAppInstance()->shader.setUniformMatrix4fv("MVP", MVP);
}

void Camera::ortho(float left, float right, float buttom, float top, float zNear, float zFar) {
	proj = Mat4::ortho(left, right, buttom, top, zNear, zFar);
}
void Camera::perspective(float fovy, float aspect, float zNear, float zFar) {
	proj = Mat4::perspective(fovy, aspect, zNear, zFar);
}
void TimeManager::handleTime() {
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