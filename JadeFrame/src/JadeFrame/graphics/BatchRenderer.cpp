#include "BatchRenderer.h"
#include "../BaseApp.h"

constexpr int MAX_BATCH_QUADS = 100000;
constexpr int MAX_VERTICES_FOR_BATCH = 4 * MAX_BATCH_QUADS;
constexpr int MAX_INDICES_FOR_BATCH = 6 * MAX_BATCH_QUADS;




BatchRenderer::BatchRenderer() {}
void BatchRenderer::init(Shader* shader) {

	matrix_stack.set_matrices_to_identity();
	current_shader = shader;
	current_shader->use();

	buffer_data.init();

}

void BatchRenderer::start(PRIMITIVE_TYPE type) {
	matrix_stack.model_matrix = Mat4();

	buffer_data.m_primitive_type = type;


	current_shader->use();
}
void BatchRenderer::end() {
	update_matrices_and_send_to_GPU();
	buffer_data.update();
	buffer_data.draw();
	buffer_data.reset_counters();
}
void BatchRenderer::update_matrices_and_send_to_GPU() {
	Mat4 MVP = /*matrix_stack.model_matrix * */matrix_stack.view_matrix * matrix_stack.projection_matrix;
	BaseApp::get_app_instance()->m_shader->set_uniform_matrix("MVP", MVP);
}
void BatchRenderer::handle_mesh(Mesh& mesh) {
	buffer_data.add_to_buffer(mesh);
}


void BatchRenderer::set_color(const Color& color) {
	buffer_data.current_color = color;
}



void BatchRenderer::BufferData::init() {

	//CPU
	vertices.resize(MAX_VERTICES_FOR_BATCH);
	indices.resize(MAX_INDICES_FOR_BATCH);

	//GPU
	GLuint vertex_buffer_size_in_bytes = vertices.size() * sizeof(Vertex);
	vertex_buffer.bind();
	vertex_buffer.reserve_in_GPU(vertex_buffer_size_in_bytes);

	vertex_array.bind();
	vertex_array.set_layout();

	GLuint index_buffer_size_in_bytes = indices.size() * sizeof(GLuint);
	index_buffer.bind();
	index_buffer.reserve_in_GPU(index_buffer_size_in_bytes);

	vertex_array.unbind();
}
void BatchRenderer::BufferData::add_to_buffer(Mesh& mesh) {
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
void BatchRenderer::BufferData::update() {

	vertex_array.bind();

	GLuint vertex_buffer_size_in_bytes = vertex_count * sizeof(Vertex);
	vertex_buffer.bind();
	vertex_buffer.update(vertex_buffer_size_in_bytes, vertices.data());


	GLuint index_buffer_size_in_bytes = index_count * sizeof(GLuint);
	index_buffer.bind();
	index_buffer.update(index_buffer_size_in_bytes, indices.data());

	vertex_array.unbind();
}
void BatchRenderer::BufferData::draw() {
	vertex_array.bind();
	glDrawElements((GLenum)m_primitive_type, index_count, GL_UNSIGNED_INT, 0);
}
void BatchRenderer::BufferData::reset_counters() {
	vertex_offset = 0;
	index_offset = 0;
	vertex_count = 0;
	index_count = 0;
}
