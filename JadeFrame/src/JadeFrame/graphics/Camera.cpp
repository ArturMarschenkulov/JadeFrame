#include "Camera.h"
#include "../BaseApp.h"
#include "../Input.h"
#include "../math/Math.h"




auto Camera::perspective(Vec3 pos, float fovy, float aspect, float zNear, float zFar) -> void {

	m_projection_matrix = Mat4::perspective(fovy, aspect, zNear, zFar);
	m_position = pos;
	m_up = { 0, 0, 1 };
	m_worldUp = { 0, 0, 1 };

	m_right = { 0, 1 , 0 };
	m_front = { -1, 0, 0 };

	m_fovy = fovy;
	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_roll = 0.0f;

	m_aspect = aspect;
	m_zNear = zNear;
	m_zFar = zFar;
}

auto Camera::get_view_matrix() const -> Mat4 {
	Mat4 view = Mat4::lookAt(m_position, m_position + m_front, m_up);
	return view;
}

auto Camera::get_projection_matrix() const -> Mat4 {
	Mat4 projection_matrix = m_projection_matrix;
	return projection_matrix;
}
auto Camera::move() -> void {
	float r = 0.1f;

	auto i = BaseApp::get_app_instance()->m_input_manager;


	if(i.is_key_down(KEY::E)) m_position += m_up * r;
	if(i.is_key_down(KEY::Q)) m_position -= m_up * r;

	if(i.is_key_down(KEY::A)) m_position -= m_right * r;
	if(i.is_key_down(KEY::D)) m_position += m_right * r;

	if(i.is_key_down(KEY::S)) m_position -= m_front * r;
	if(i.is_key_down(KEY::W)) m_position += m_front * r;

	if(i.is_key_down(KEY::UP))    m_pitch += r;
	if(i.is_key_down(KEY::DOWN))  m_pitch -= r;
	if(i.is_key_down(KEY::RIGHT)) m_yaw += r;
	if(i.is_key_down(KEY::LEFT))  m_yaw -= r;


	static float old_fovy = m_fovy;
	if(i.is_key_down(KEY::R))  m_fovy += to_radians(r);
	if(i.is_key_down(KEY::F))  m_fovy -= to_radians(r);

	if(old_fovy != m_fovy) {
		old_fovy = m_fovy;
		perspective(m_position, m_fovy, m_aspect, m_zNear, m_zFar);
		__debugbreak();
		//BaseApp::get_app_instance()->m_renderer->matrix_stack.projection_matrix = get_projection_matrix();
	}





	Vec3 front;
	front.x = cos(to_radians(m_yaw)) * cos(to_radians(m_pitch));
	front.y = sin(to_radians(m_pitch));
	front.z = sin(to_radians(m_yaw)) * cos(to_radians(m_pitch));
	m_front = front.normalize();

	//m_right = m_worldUp.cross(m_front).normalize();
	//m_up = m_front.cross(m_right).normalize();

	//m_right = m_front.cross(m_worldUp).normalize();
	//m_up = m_right.cross(m_front).normalize();

}

auto Camera::update() -> void {


}
