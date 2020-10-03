#include "GLRenderer.h"
#include "../BaseApp.h"

GLRenderer::GLRenderer() {}

auto GLRenderer::init() -> void {
	buffer_data.init();
}
auto GLRenderer::start(PRIMITIVE_TYPE type) -> void {
	buffer_data.m_primitive_type = type;
	current_shader->use();

	cam.move();
	//cam.update();
}
auto GLRenderer::end() -> void {
	matrix_stack.view_matrix = cam.get_view_matrix();
	Mat4 MVP = matrix_stack.view_matrix * matrix_stack.projection_matrix;
	current_shader->set_uniform_matrix("MVP", MVP);

	buffer_data.update();
	buffer_data.draw();
	buffer_data.reset_counters();
}

auto GLBufferData::set_color(const Color& color) -> void {
	current_color = color;
}

auto GLBufferData::init() -> void {

	//CPU
	vertices.resize(MAX_VERTICES_FOR_BATCH);
	indices.resize(MAX_INDICES_FOR_BATCH);

	//GPU

	vertex_buffer.bind();
	vertex_buffer.reserve_in_GPU(/*vertex_buffer_size_in_bytes*/vertices.size() * sizeof(Vertex));

	vertex_array.bind();

	BufferLayout layout = {
		{SHADER_DATA_TYPE::FLOAT_3, "v_pos"},
		{SHADER_DATA_TYPE::FLOAT_4, "v_col"},
		{SHADER_DATA_TYPE::FLOAT_2, "v_texture_coord"},
	};
	vertex_array.set_layout(layout);

	index_buffer.bind();
	index_buffer.reserve_in_GPU(/*index_buffer_size_in_bytes*/indices.size() * sizeof(GLuint));

	vertex_array.unbind();
}
auto GLBufferData::add_to_buffer(const Mesh& mesh) -> void {
	//if vertex or index count is larger than the max, flush the data
	if ((vertex_count + mesh.vertices.size() > MAX_VERTICES_FOR_BATCH) ||
		(index_count + mesh.indices.size() > MAX_INDICES_FOR_BATCH)) {
		update();
		draw();
		reset_counters();
	}

	for (GLuint i = 0; i < mesh.vertices.size(); i++) {
		vertices[i + vertex_offset].position = mesh.vertices[i].position;
		vertices[i + vertex_offset].color = current_color;
		vertex_count++;
	}

	for (int i = 0; i < mesh.indices.size(); i++) {
		indices[i + index_offset] = mesh.indices[i] + vertex_offset;
		index_count++;
	}


	if (0) {
		Mesh2 mesh;
		for (GLuint i = 0; i < mesh.positions.size(); i++) {
			vertices[i + vertex_offset].position = mesh.positions[i];
			vertices[i + vertex_offset].color = current_color;
			vertex_count++;
		}

		for (int i = 0; i < mesh.indices.size(); i++) {
			indices[i + index_offset] = mesh.indices[i] + vertex_offset;
			index_count++;
		}
	}
	vertex_offset += mesh.vertices.size();
	index_offset += mesh.indices.size();
}
auto GLBufferData::update() -> void {
	vertex_array.bind();

	vertex_buffer.bind();
	vertex_buffer.update(/*vertex_buffer_size_in_bytes*/vertex_count * sizeof(Vertex), vertices.data());

	index_buffer.bind();
	index_buffer.update(/*index_buffer_size_in_bytes*/index_count * sizeof(GLuint), indices.data());

	vertex_array.unbind();
}
auto GLBufferData::draw() -> void {
	vertex_array.bind();
	glDrawElements(static_cast<GLenum>(m_primitive_type), index_count, GL_UNSIGNED_INT, 0);
}
auto GLBufferData::reset_counters() -> void {
	vertex_offset = 0;
	index_offset = 0;
	vertex_count = 0;
	index_count = 0;
}