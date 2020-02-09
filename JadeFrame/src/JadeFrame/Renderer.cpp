#include "Renderer.h"
#include "BaseApp.h"






Renderer::Renderer() {}
void Renderer::init(Shader* shader) {
	currentShader = shader;

	glCreateBuffers(1, &vertexData.VBO);
	glCreateVertexArrays(1, &vertexData.VAO);
	glCreateBuffers(1, &vertexData.IBO);


	vertexData.vertices.reserve(9000);
	vertexData.indices.reserve(9000);
}
void Renderer::drawRectangle(Vec2 pos, Vec2 size) {
	Mesh rectMesh = MeshManager::makeRectangle(pos, size);
	draw(rectMesh);
}
void Renderer::draw(Mesh& mesh) {


	for(int i = 0; i < mesh.vertices.size(); i++) {
		vertexData.vertices.push_back(mesh.vertices[i]);
		vertexData.vertexCount++;
	}

	for(int i = 0; i < mesh.indices.size(); i++) {
		mesh.indices[i] += vertexData.indexOffset;
		vertexData.indices.push_back(mesh.indices[i]);
	}
	vertexData.indexOffset += mesh.vertices.size();



	{
		//for(int i = 0; i < mesh.vertices.size(); i++) {
		//	vertexData.vertices.push_back(mesh.vertices[i]);
		//}
		//for(int i = 0; i < mesh.indices.size(); i++) {
		//	mesh.indices[i] += vertexData.indexOffset;
		//	vertexData.indices.push_back(mesh.indices[i]);
		//}
		//vertexData.indexOffset += mesh.vertices.size();
	}
}

void Renderer::startDraw() {
	glUseProgram(currentShader->shaderID);
	glBindVertexArray(vertexData.VAO);

	vertexData.vertexOffset = 0;
	vertexData.indexOffset = 0;
	vertexData.vertexCount = 0;

	if(BaseApp::getAppInstance()->input.isKeyDown(KEY::K)) {
		vertexData.vertices.resize(0);
		vertexData.indices.resize(0);
	}
}

void Renderer::endDraw() {

	glBindBuffer(GL_ARRAY_BUFFER, vertexData.VBO);
	GLuint vertexBudderSizeInBytes = vertexData.vertices.size() * sizeof(Vertex);
	glBufferData(GL_ARRAY_BUFFER, vertexBudderSizeInBytes, vertexData.vertices.data(), GL_STATIC_DRAW);


	glBindVertexArray(vertexData.VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexData.IBO);
	GLuint indexBudderSizeInBytes = vertexData.indices.size() * sizeof(GLuint);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBudderSizeInBytes, vertexData.indices.data(), GL_STATIC_DRAW);



	GLsizei  x1 = vertexData.vertexCount;
	GLsizei  x2 = vertexData.vertices.size();

	glDrawElements(GL_TRIANGLES, 16, GL_UNSIGNED_INT, 0);

	//glDrawElements(GL_TRIANGLES, vertexData.vertexCount, GL_UNSIGNED_INT, 0);
	//glDrawElements(GL_TRIANGLES, vertexData.vertices.size(), GL_UNSIGNED_INT, 0);






	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glDeleteBuffers(1, &vertexData.VBO);
	//vertexData.VBO = 0;

	//glBindVertexArray(0);
	//glDeleteVertexArrays(1, &vertexData.VAO);
	//vertexData.VAO = 0;

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glDeleteBuffers(1, &vertexData.IBO);
	//vertexData.IBO = 0;


}
