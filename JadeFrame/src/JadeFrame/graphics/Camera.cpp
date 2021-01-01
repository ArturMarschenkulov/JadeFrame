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

	Vec3 front;
	front.x = cos(to_radians(m_yaw)) * cos(to_radians(m_pitch));
	front.y = sin(to_radians(m_pitch));
	front.z = sin(to_radians(m_yaw)) * cos(to_radians(m_pitch));
	m_front = front.normalize();
}

auto Camera::get_view_matrix() const -> Mat4 {
	return Mat4::lookAt(m_position, m_position + m_front, m_up);
	
}

auto Camera::get_projection_matrix() const -> Mat4 {
	return m_projection_matrix;
}

auto camera_control(Camera& camera) -> void {
	float r = 0.1f;

	WinInputManager i = BaseApp::get_instance()->m_input_manager;

	auto& c = camera;


	if (i.is_key_down(KEY::E)) c.m_position += c.m_up * r;
	if (i.is_key_down(KEY::Q)) c.m_position -= c.m_up * r;

	if (i.is_key_down(KEY::A)) c.m_position -= c.m_right * r;
	if (i.is_key_down(KEY::D)) c.m_position += c.m_right * r;

	if (i.is_key_down(KEY::S)) c.m_position -= c.m_front * r;
	if (i.is_key_down(KEY::W)) c.m_position += c.m_front * r;

	if (i.is_key_down(KEY::UP))    c.m_pitch += r;
	if (i.is_key_down(KEY::DOWN))  c.m_pitch -= r;
	if (i.is_key_down(KEY::RIGHT)) c.m_yaw += r;
	if (i.is_key_down(KEY::LEFT))  c.m_yaw -= r;


	static float old_fovy = c.m_fovy;
	if (i.is_key_down(KEY::R))  c.m_fovy += to_radians(r);
	if (i.is_key_down(KEY::F))  c.m_fovy -= to_radians(r);

	if (old_fovy != c.m_fovy) {
		old_fovy = c.m_fovy;
		c.perspective(c.m_position, c.m_fovy, c.m_aspect, c.m_zNear, c.m_zFar);
		__debugbreak();
		//BaseApp::get_instance()->m_renderer->matrix_stack.projection_matrix = get_projection_matrix();
	}





	Vec3 front;
	front.x = cos(to_radians(c.m_yaw)) * cos(to_radians(c.m_pitch));
	front.y = sin(to_radians(c.m_pitch));
	front.z = sin(to_radians(c.m_yaw)) * cos(to_radians(c.m_pitch));
	c.m_front = front.normalize();

	//m_right = m_worldUp.cross(m_front).normalize();
	//m_up = m_front.cross(m_right).normalize();

	//m_right = m_front.cross(m_worldUp).normalize();
	//m_up = m_right.cross(m_front).normalize();
}

auto camera_control(Camera1& camera) -> void {
	float r = 0.1f;

	WinInputManager i = BaseApp::get_instance()->m_input_manager;

	auto& c = camera;

	Vec3 right = { 0.0f, 1.0f, 0.0f };
	Vec3 front = { -1.0f, 0.0f, 0.0f };
	//if (i.is_key_down(KEY::E)) c.m_position += c.m_up * r;
	//if (i.is_key_down(KEY::Q)) c.m_position -= c.m_up * r;

	if (i.is_key_down(KEY::A)) c.m_position -= right * r;
	if (i.is_key_down(KEY::D)) c.m_position += right * r;

	if (i.is_key_down(KEY::S)) c.m_position -= front * r;
	if (i.is_key_down(KEY::W)) c.m_position += front * r;

	//if (i.is_key_down(KEY::UP))    c.m_pitch += r;
	//if (i.is_key_down(KEY::DOWN))  c.m_pitch -= r;
	//if (i.is_key_down(KEY::RIGHT)) c.m_yaw += r;
	//if (i.is_key_down(KEY::LEFT))  c.m_yaw -= r;


	//static float old_fovy = c.m_fovy;
	//if (i.is_key_down(KEY::R))  c.m_fovy += to_radians(r);
	//if (i.is_key_down(KEY::F))  c.m_fovy -= to_radians(r);

	//if (old_fovy != c.m_fovy) {
	//	old_fovy = c.m_fovy;
	//	c.perspective(c.m_position, c.m_fovy, c.m_aspect, c.m_zNear, c.m_zFar);
	//	__debugbreak();
	//	//BaseApp::get_instance()->m_renderer->matrix_stack.projection_matrix = get_projection_matrix();
	//}




	float yaw = 0;
	float pitch = 0;
	Vec3 ffront;
	ffront.x = cos(to_radians(yaw)) * cos(to_radians(pitch));
	ffront.y = sin(to_radians(pitch));
	ffront.z = sin(to_radians(yaw)) * cos(to_radians(pitch));
	c.m_forward = ffront.normalize();

	////m_right = m_worldUp.cross(m_front).normalize();
	////m_up = m_front.cross(m_right).normalize();

	////m_right = m_front.cross(m_worldUp).normalize();
	////m_up = m_right.cross(m_front).normalize();
}

auto Camera1::perspective(Vec3 position, float fov, float aspect, float zNear, float zFar) -> void {
	// probably move to constructor
	m_position = position;
	m_forward = { -1.0f, 0.0f, 0.0f };
	m_up = { 0.0f, 0.0f, 1.0f };
	//m_forward = { 0.0f, 0.0f, -1.0f };
	//m_up = { 0.0f, 1.0f, 0.0f };

	m_mode = MODE::PERSPECTIVE;

	m_projection = Mat4::perspective(fov, aspect, zNear, zFar);
	m_fov = fov;
	m_aspect = aspect;
	m_near = zNear;
	m_far = zFar;


	float yaw = 0;
	float pitch = 0;

	//Vec3 front;
	//front.x = cos(to_radians(yaw)) * cos(to_radians(pitch));
	//front.y = sin(to_radians(pitch));
	//front.z = sin(to_radians(yaw)) * cos(to_radians(pitch));
	//m_forward = front.normalize();
}
