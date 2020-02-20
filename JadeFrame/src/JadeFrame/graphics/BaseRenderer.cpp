#include "BaseRenderer.h"
#include "../BaseApp.h"

constexpr int MAX_BATCH_QUADS = 100000;
constexpr int MAX_VERTICES_FOR_BATCH = 4 * MAX_BATCH_QUADS;
constexpr int MAX_INDICES_FOR_BATCH = 6 * MAX_BATCH_QUADS;




BaseRenderer::BaseRenderer() {}
void BaseRenderer::init(BaseShader* shader) {

	currentShader = shader;
	currentShader->use();

	//bufferData.init();

}

void BaseRenderer::start() {
	matrixStack.projectionMatrix = Mat4();
	matrixStack.viewMatrix = Mat4();
	matrixStack.modelMatrix = Mat4();
	matrixStack.currentMatrix = &matrixStack.viewMatrix;

	currentShader->use();
}

class sVBO {
public:
	sVBO(Mesh& mesh) {
		if(!ID) glCreateBuffers(1, &ID);
		glBindBuffer(GL_ARRAY_BUFFER, ID);
		glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
	}
	~sVBO() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &ID);
		ID = 0;
	}
	GLuint ID = 0;
};
class sVAO {
public:
	sVAO(Mesh& mesh) {
		if(!ID) glCreateVertexArrays(1, &ID);
		glBindVertexArray(ID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);
	}
	~sVAO() {
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &ID);
		ID = 0;
	}
	GLuint ID = 0;
};
class sIBO {
public:
	sIBO(Mesh& mesh) {
		if(!ID) glCreateBuffers(1, &ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(), GL_STATIC_DRAW);
	}
	~sIBO() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &ID);
		ID = 0;
	}
	GLuint ID = 0;
};

class VertexData {
public:
	sVBO VBO;
	sVAO VAO;
	sIBO IBO;

	VertexData(Mesh& mesh)
		: VBO(mesh), VAO(mesh), IBO(mesh) {}
	~VertexData() {}
};
void BaseRenderer::handleMesh(Mesh& mesh) {
	VertexData vD(mesh);
	glDrawElements(GL_TRIANGLES, mesh.indices.size() * sizeof(GLuint), GL_UNSIGNED_INT, (void*)0);
}
void BaseRenderer::updateMatrices() {
	Mat4 MVP = matrixStack.modelMatrix * matrixStack.viewMatrix * matrixStack.projectionMatrix;
	BaseApp::getAppInstance()->shader.setUniformMatrix4fv("MVP", MVP);
}
void BaseRenderer::end() {
	updateMatrices();
}

void BaseRenderer::setColor(const Color& color) {
	currentShader->setUniform4f("color", { color.r, color.g, color.b, color.a });
}
void BaseRenderer::setClearColor(const Color& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}



void BaseRenderer::ortho(float left, float right, float buttom, float top, float zNear, float zFar) {
	matrixStack.projectionMatrix = Mat4::ortho(left, right, buttom, top, zNear, zFar);
}
void BaseRenderer::perspective(float fovy, float aspect, float zNear, float zFar) {
	matrixStack.projectionMatrix = Mat4::perspective(fovy, aspect, zNear, zFar);
}
void BaseRenderer::translate(float x, float y, float z) {
	*matrixStack.currentMatrix = Mat4::translate(*matrixStack.currentMatrix, Vec3(x, y, z));
}
void BaseRenderer::rotate(float angle, float x, float y, float z) {
	*matrixStack.currentMatrix = Mat4::rotate(*matrixStack.currentMatrix, angle, Vec3(x, y, z));
}
void BaseRenderer::scale(float x, float y, float z) {
	*matrixStack.currentMatrix = Mat4::scale(*matrixStack.currentMatrix, Vec3(x, y, z));
}

Camera2::Camera2() {

}
void TimeManager2::handleTime() {
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
