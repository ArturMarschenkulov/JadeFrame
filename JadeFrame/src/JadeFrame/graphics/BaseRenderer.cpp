#include "BaseRenderer.h"
#include "../BaseApp.h"

constexpr int MAX_BATCH_QUADS = 100000;
constexpr int MAX_VERTICES_FOR_BATCH = 4 * MAX_BATCH_QUADS;
constexpr int MAX_INDICES_FOR_BATCH = 6 * MAX_BATCH_QUADS;




BaseRenderer::BaseRenderer() {}
void BaseRenderer::init(BaseShader* shader) {
	current_shader = shader;
	current_shader->use();
}

void BaseRenderer::start() {
	matrix_stack.projection_matrix = Mat4();
	matrix_stack.view_matrix = Mat4();
	matrix_stack.model_matrix = Mat4();
	matrix_stack.current_matrix = &matrix_stack.view_matrix;

	current_shader->use();
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
void BaseRenderer::handle_mesh(Mesh& mesh) {
	VertexData vD(mesh);
	glDrawElements(GL_TRIANGLES, mesh.indices.size() * sizeof(GLuint), GL_UNSIGNED_INT, (void*)0);
}
void BaseRenderer::update_matrices() {
	Mat4 MVP = matrix_stack.model_matrix * matrix_stack.view_matrix * matrix_stack.projection_matrix;
	BaseApp::get_app_instance()->shader.set_uniform_matrix4fv("MVP", MVP);
}
void BaseRenderer::end() {
	update_matrices();
}

void BaseRenderer::set_color(const Color& color) {
	static Color cColor;
	if(!(color == cColor)) {
		current_shader->setUniform4f("color", { color.r, color.g, color.b, color.a });
	}
}
void BaseRenderer::set_clear_color(const Color& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}



void BaseRenderer::ortho(float left, float right, float buttom, float top, float zNear, float zFar) {
	matrix_stack.projection_matrix = Mat4::ortho(left, right, buttom, top, zNear, zFar);
}
void BaseRenderer::perspective(float fovy, float aspect, float zNear, float zFar) {
	matrix_stack.projection_matrix = Mat4::perspective(fovy, aspect, zNear, zFar);
}
void BaseRenderer::translate(float x, float y, float z) {
	*matrix_stack.current_matrix = Mat4::translate(*matrix_stack.current_matrix, Vec3(x, y, z));
}
void BaseRenderer::rotate(float angle, float x, float y, float z) {
	*matrix_stack.current_matrix = Mat4::rotate(*matrix_stack.current_matrix, angle, Vec3(x, y, z));
}
void BaseRenderer::scale(float x, float y, float z) {
	*matrix_stack.current_matrix = Mat4::scale(*matrix_stack.current_matrix, Vec3(x, y, z));
}