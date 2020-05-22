#include "GLRenderer.h"
#include "../BaseApp.h"

constexpr int MAX_BATCH_QUADS = 100000;
constexpr int MAX_VERTICES_FOR_BATCH = 4 * MAX_BATCH_QUADS;
constexpr int MAX_INDICES_FOR_BATCH = 6 * MAX_BATCH_QUADS;




GLBatchRenderer::GLBatchRenderer() {}
void GLBatchRenderer::init(GLShader* shader) {
	current_shader = shader;
	current_shader->use();

	buffer_data.init();

}

void GLBatchRenderer::start(PRIMITIVE_TYPE type) {
	buffer_data.m_primitive_type = type;
	current_shader->use();

	cam.move();
	cam.update();
}
void GLBatchRenderer::end() {

	matrix_stack.view_matrix = cam.get_view_matrix();
	Mat4 MVP = matrix_stack.view_matrix * matrix_stack.projection_matrix;
	BaseApp::get_app_instance()->m_shader->set_uniform_matrix("MVP", MVP);

	//this->update_matrices_and_send_to_GPU();
	buffer_data.update();
	buffer_data.draw();
	buffer_data.reset_counters();
}
void GLBatchRenderer::update_matrices_and_send_to_GPU() {
	//Mat4 MVP = cam.get_view_matrix() * matrix_stack.projection_matrix;
	matrix_stack.view_matrix = cam.get_view_matrix();
	Mat4 MVP = matrix_stack.view_matrix * matrix_stack.projection_matrix;
	BaseApp::get_app_instance()->m_shader->set_uniform_matrix("MVP", MVP);
}
void GLBatchRenderer::add_to_buffer(Mesh& mesh) {
	buffer_data.add_to_buffer(mesh);
}
void GLBatchRenderer::add_to_buffer(Mesh&& mesh) {
	buffer_data.add_to_buffer(mesh);
}


void GLBatchRenderer::set_color(const Color& color) {
	buffer_data.current_color = color;
}



void GLBatchRenderer::GLBufferData::init() {

	//CPU
	vertices.resize(MAX_VERTICES_FOR_BATCH);
	indices.resize(MAX_INDICES_FOR_BATCH);

	//GPU

	vertex_buffer.bind();
	GLuint vertex_buffer_size_in_bytes = vertices.size() * sizeof(Vertex);
	vertex_buffer.reserve_in_GPU(vertex_buffer_size_in_bytes);

	vertex_array.bind();
	vertex_array.set_layout();

	index_buffer.bind();
	GLuint index_buffer_size_in_bytes = indices.size() * sizeof(GLuint);
	index_buffer.reserve_in_GPU(index_buffer_size_in_bytes);

	vertex_array.unbind();
}
void GLBatchRenderer::GLBufferData::add_to_buffer(Mesh& mesh) {
	//if vertex or index count is larger than the max, flush the data
	if ((vertex_count + mesh.vertices.size() > MAX_VERTICES_FOR_BATCH) ||
		(index_count + mesh.indices.size() > MAX_INDICES_FOR_BATCH)) {
		update();
		draw();
		reset_counters();
	}

	for (GLuint i = 0; i < mesh.vertices.size(); i++) {
		vertices[i + vertex_offset].position = mesh.vertices[i].position;
		vertices[i + vertex_offset].color = current_color;;
		vertex_count++;
	}

	for (int i = 0; i < mesh.indices.size(); i++) {
		indices[i + index_offset] = mesh.indices[i] + vertex_offset;
		index_count++;
	}

	vertex_offset += mesh.vertices.size();
	index_offset += mesh.indices.size();
}
void GLBatchRenderer::GLBufferData::update() {

	vertex_array.bind();


	vertex_buffer.bind();
	GLuint vertex_buffer_size_in_bytes = vertex_count * sizeof(Vertex);
	vertex_buffer.update(vertex_buffer_size_in_bytes, vertices.data());



	index_buffer.bind();
	GLuint index_buffer_size_in_bytes = index_count * sizeof(GLuint);
	index_buffer.update(index_buffer_size_in_bytes, indices.data());

	vertex_array.unbind();
}
void GLBatchRenderer::GLBufferData::draw() {
	vertex_array.bind();
	glDrawElements((GLenum)m_primitive_type, index_count, GL_UNSIGNED_INT, 0);
}
void GLBatchRenderer::GLBufferData::reset_counters() {
	vertex_offset = 0;
	index_offset = 0;
	vertex_count = 0;
	index_count = 0;
}
