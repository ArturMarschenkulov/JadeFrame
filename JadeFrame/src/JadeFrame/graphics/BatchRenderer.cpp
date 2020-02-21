#include "BatchRenderer.h"
#include "../BaseApp.h"

constexpr int MAX_BATCH_QUADS = 100000;
constexpr int MAX_VERTICES_FOR_BATCH = 4 * MAX_BATCH_QUADS;
constexpr int MAX_INDICES_FOR_BATCH = 6 * MAX_BATCH_QUADS;




BatchRenderer::BatchRenderer() {}
void BatchRenderer::init(BatchShader* shader) {
	matrix_stack.projection_matrix = Mat4();
	matrix_stack.view_matrix = Mat4();
	matrix_stack.model_matrix = Mat4();
	current_shader = shader;
	current_shader->use();

	buffer_data.init();

}

void BatchRenderer::start() {
	//matrixStack.projectionMatrix = Mat4();
	matrix_stack.view_matrix = Mat4();
	matrix_stack.model_matrix = Mat4();
	matrix_stack.current_matrix = &matrix_stack.view_matrix;
	BaseApp::get_app_instance()->renderer.num_draw_calls = 0;
	auto window = BaseApp::get_app_instance()->window;


	current_shader->use();
}
void BatchRenderer::handle_mesh(Mesh& mesh) {
	if(matrix_stack.use_transform_matrix == true) {
		Mat4 m = matrix_stack.transform_matrix;
		for(int i = 0; i < mesh.vertices.size(); i++) {
			mesh.vertices[i].position = m * mesh.vertices[i].position;
		}
	}

	buffer_data.add(mesh);
}
void BatchRenderer::update_matrices() {
	//Mat4 MVP = proj * view * model;
	Mat4 MVP = matrix_stack.model_matrix * matrix_stack.view_matrix * matrix_stack.projection_matrix;
	BaseApp::get_app_instance()->shader.set_uniform_matrix4fv("MVP", MVP);
}
void BatchRenderer::end() {
	update_matrices();
	buffer_data.update();
	buffer_data.draw();
	buffer_data.reset_counters();
}

void BatchRenderer::set_color(const Color& color) {
	buffer_data.current_color = color;
}
void BatchRenderer::set_clear_color(const Color& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}



void BatchRenderer::BufferData::init() {

	//CPU
	vertices.resize(MAX_VERTICES_FOR_BATCH);
	indices.resize(MAX_INDICES_FOR_BATCH);

	//GPU
	glCreateBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	GLuint vertex_buffer_size_in_bytes = vertices.size() * sizeof(Vertex);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size_in_bytes, NULL, GL_DYNAMIC_DRAW);


	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(1);

	glCreateBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	GLuint index_buffer_size_in_bytes = indices.size() * sizeof(GLuint);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size_in_bytes, NULL, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);

}
void BatchRenderer::BufferData::add(Mesh& mesh) {


	if((vertex_count + mesh.vertices.size() > MAX_VERTICES_FOR_BATCH) ||
		(index_count + mesh.indices.size() > MAX_INDICES_FOR_BATCH)) {
		update();
		draw();
		reset_counters();
	}



	for(unsigned int i = 0; i < mesh.vertices.size(); i++) {
		//mesh.vertices[i].color = BaseApp::getAppInstance()->renderer.currentColor;;
		//bufferData.vertices[i + bufferData.vertexOffset] = mesh.vertices[i];

		vertices[i + vertex_offset].position = mesh.vertices[i].position;
		vertices[i + vertex_offset].color = current_color;;
		vertex_count++;
	}



	for(int i = 0; i < mesh.indices.size(); i++) {
		indices[i + index_offset] = mesh.indices[i] + vertex_offset;
		index_count++;
	}

	vertex_offset += mesh.vertices.size();
	index_offset += mesh.indices.size();

}
void BatchRenderer::BufferData::update() {

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	GLuint vertex_buffer_size_in_bytes = vertex_count * sizeof(Vertex);
	GLuint vertex_buffer_size_in_bytes2 = vertices.size() * sizeof(Vertex);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_buffer_size_in_bytes, vertices.data());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	GLuint index_buffer_size_in_bytes = index_count * sizeof(GLuint);
	GLuint index_buffer_size_in_bytes2 = indices.size() * sizeof(GLuint);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_buffer_size_in_bytes, indices.data());

	glBindVertexArray(0);
}
void BatchRenderer::BufferData::draw() {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
	BaseApp::get_app_instance()->renderer.num_draw_calls++;

}
void BatchRenderer::BufferData::reset_counters() {

	vertex_offset = 0;
	index_offset = 0;
	vertex_count = 0;
	index_count = 0;
}





void BatchRenderer::ortho(float left, float right, float buttom, float top, float zNear, float zFar) {
	matrix_stack.projection_matrix = Mat4::ortho(left, right, buttom, top, zNear, zFar);
}
void BatchRenderer::perspective(float fovy, float aspect, float zNear, float zFar) {
	matrix_stack.projection_matrix = Mat4::perspective(fovy, aspect, zNear, zFar);
}
void BatchRenderer::translate(float x, float y, float z) {
	*matrix_stack.current_matrix = Mat4::translate(*matrix_stack.current_matrix, Vec3(x, y, z));
}
void BatchRenderer::rotate(float angle, float x, float y, float z) {
	*matrix_stack.current_matrix = Mat4::rotate(*matrix_stack.current_matrix, angle, Vec3(x, y, z));
}
void BatchRenderer::scale(float x, float y, float z) {
	*matrix_stack.current_matrix = Mat4::scale(*matrix_stack.current_matrix, Vec3(x, y, z));
}
