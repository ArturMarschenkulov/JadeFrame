#include "BatchRenderer.h"
#include "BaseApp.h"

constexpr int MAX_BATCH_QUADS = 100000;
constexpr int MAX_VERTICES_FOR_BATCH = 4 * MAX_BATCH_QUADS;
constexpr int MAX_INDICES_FOR_BATCH = 6 * MAX_BATCH_QUADS;




BatchRenderer::BatchRenderer() {}
void BatchRenderer::init(BatchShader* shader) {

	currentShader = shader;
	currentShader->use();

	bufferData.init();

}

void BatchRenderer::start() {
	projectionMatrix = Mat4();
	viewMatrix = Mat4();
	modelMatrix = Mat4();
	currentMatrix = &viewMatrix;
	auto window = BaseApp::getAppInstance()->window;


	currentShader->use();
}
void BatchRenderer::updateMatrices() {
	//Mat4 MVP = proj * view * model;
	Mat4 MVP = modelMatrix * viewMatrix * projectionMatrix;
	BaseApp::getAppInstance()->shader.setUniformMatrix4fv("MVP", MVP);
}
void BatchRenderer::end() {
	updateMatrices();
	bufferData.update();
	bufferData.draw();
	bufferData.resetCounters();
}

void BatchRenderer::setColor(const Color& color) {
	currentColor = color;
}
void BatchRenderer::setClearColor(const Color& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}



void BatchRenderer::BufferData::init() {

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
void BatchRenderer::BufferData::add(Mesh& mesh) {
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
void BatchRenderer::BufferData::update() {

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
void BatchRenderer::BufferData::draw() {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}
void BatchRenderer::BufferData::resetCounters() {
	vertexOffset = 0;
	indexOffset = 0;
	vertexCount = 0;
	indexCount = 0;
}





void BatchRenderer::ortho(float left, float right, float buttom, float top, float zNear, float zFar) {
	projectionMatrix = Mat4::ortho(left, right, buttom, top, zNear, zFar);
}
void BatchRenderer::perspective(float fovy, float aspect, float zNear, float zFar) {
	projectionMatrix = Mat4::perspective(fovy, aspect, zNear, zFar);
}
void BatchRenderer::translate(float x, float y, float z) {
	*currentMatrix = Mat4::translate(*currentMatrix, Vec3(x, y, z));
}
void BatchRenderer::rotate(float angle, float x, float y, float z) {
	*currentMatrix = Mat4::rotate(*currentMatrix, angle, Vec3(x, y, z));
}
void BatchRenderer::scale(float x, float y, float z) {
	*currentMatrix = Mat4::scale(*currentMatrix, Vec3(x, y, z));
}
void BatchRenderer::pushMatrix() {

	useTransformMatrix = true;
	currentMatrix = &transformMatrix;

	matrixStack.push(*currentMatrix);
}
void BatchRenderer::popMatrix() {
	if(!matrixStack.empty()) {
		Mat4 mat = matrixStack.top();
		*currentMatrix = mat;
		matrixStack.pop();
	}

	if(matrixStack.empty()) {
		currentMatrix = &viewMatrix;
		useTransformMatrix = false;
	}
}

Camera::Camera() {

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