#include "GLRenderer.h"
#include "../BaseApp.h"

GLRenderer::GLRenderer() {}

auto GLRenderer::init() -> void {
	//buffer_data.init();
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