#include "camera.h"

//#if 1 // NOTE: for camera cotrolling. Remove if we move it
#include "../base_app.h"
#include "../platform/windows/windows_input_manager.h"
#include "../math/math.h"
//#endif




auto Camera::perspective(const Vec3& pos, const float fovy, const float aspect, const float zNear, const float zFar) -> void {

	m_projection_matrix = Matrix4x4::perspective_projection_matrix(fovy, aspect, zNear, zFar);
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
	m_front = front.get_normal();
}

auto Camera::get_view_matrix() const -> Matrix4x4 {
	return Matrix4x4::look_at_matrix(m_position, m_position + m_front, m_up);

}

auto Camera::control() -> void {
	float r = 0.1f;

	Windows_InputManager i = JadeFrame::get_singleton()->m_input_manager;


	if (i.is_key_down(KEY::E)) m_position += m_up * r;
	if (i.is_key_down(KEY::Q)) m_position -= m_up * r;

	if (i.is_key_down(KEY::A)) m_position -= m_right * r;
	if (i.is_key_down(KEY::D)) m_position += m_right * r;

	if (i.is_key_down(KEY::S)) m_position -= m_front * r;
	if (i.is_key_down(KEY::W)) m_position += m_front * r;

	if (i.is_key_down(KEY::UP))    m_pitch += r;
	if (i.is_key_down(KEY::DOWN))  m_pitch -= r;
	if (i.is_key_down(KEY::RIGHT)) m_yaw += r;
	if (i.is_key_down(KEY::LEFT))  m_yaw -= r;


	static float old_fovy = m_fovy;
	if (i.is_key_down(KEY::R))  m_fovy += to_radians(r);
	if (i.is_key_down(KEY::F))  m_fovy -= to_radians(r);

	if (old_fovy != m_fovy) {
		old_fovy = m_fovy;
		perspective(m_position, m_fovy, m_aspect, m_zNear, m_zFar);
		__debugbreak();
	}





	Vec3 front;
	front.x = cos(to_radians(m_yaw)) * cos(to_radians(m_pitch));
	front.y = sin(to_radians(m_pitch));
	front.z = sin(to_radians(m_yaw)) * cos(to_radians(m_pitch));
	m_front = front.get_normal();

	//m_right = m_worldUp.cross(m_front).normalize();
	//m_up = m_front.cross(m_right).normalize();

	//m_right = m_front.cross(m_worldUp).normalize();
	//m_up = m_right.cross(m_front).normalize();
}

auto Camera::get_projection_matrix() const -> Matrix4x4 {
	return m_projection_matrix;
}

auto Camera1::perspective(const Vec3& position, const float fov, const float aspect, const float zNear, const float zFar) -> void {
	m_mode = MODE::PERSPECTIVE;
	// probably move to constructor
	m_position = position;
	m_forward = { 0.0f, 1.0f, 0.0f };
	m_world_up = { 0.0f, 0.0f, 1.0f };

	m_projection_matrix = Matrix4x4::perspective_projection_matrix(fov, aspect, zNear, zFar);

	m_fov = fov;
	m_aspect = aspect;
	m_near = zNear;
	m_far = zFar;
}

auto Camera1::othographic(float left, float right, float bottom, float top, float near_, float far_) -> void {
	m_mode = MODE::ORTHOGRAPHIC;
	m_projection_matrix =
		Matrix4x4::rotation_matrix(to_radians(180), { 1, 0, 0 }) *
		Matrix4x4::rotation_matrix(to_radians(90), { 0, 1, 0 }) *
		//Matrix4x4::scale({ 20, 20, 1 }) *
		Matrix4x4::translation_matrix({ 0, top, 0 }) *
		Matrix4x4::orthogonal_projection_matrix(left, right, bottom, top, near_, far_);

	m_position = { 0.0f, 0.0f, 0.0f };

	m_forward = { 1.0f, 0.0f, 0.0f };
	m_up = { 0.0f, 1.0f, 0.0f };
}

auto Camera1::control() -> void {
	if (m_mode == MODE::PERSPECTIVE) {
		const float velocity = 0.1f;
		const Windows_InputManager& i = JadeFrame::get_singleton()->m_input_manager;
		if (i.is_key_down(KEY::E)) m_position += m_up * velocity;
		if (i.is_key_down(KEY::Q)) m_position -= m_up * velocity;

		if (i.is_key_down(KEY::A)) m_position -= m_right * velocity;
		if (i.is_key_down(KEY::D)) m_position += m_right * velocity;

		if (i.is_key_down(KEY::S)) m_position -= m_forward * velocity;
		if (i.is_key_down(KEY::W)) m_position += m_forward * velocity;

		auto sensitivity = 10;
		if (i.is_key_down(KEY::LEFT)) m_pitch += velocity * sensitivity;
		if (i.is_key_down(KEY::RIGHT)) m_pitch -= velocity * sensitivity;
		if (i.is_key_down(KEY::UP)) m_yaw += velocity * sensitivity;
		if (i.is_key_down(KEY::DOWN)) m_yaw -= velocity * sensitivity;


		//if (m_pitch > 89.0f)
		//	m_pitch = 89.0f;
		//if (m_pitch < -89.0f)
		//	m_pitch = -89.0f;

		Vec3 front;
		front.x = cos(to_radians(m_yaw)) * cos(to_radians(m_pitch));
		front.y = sin(to_radians(m_pitch));
		front.z = sin(to_radians(m_yaw)) * cos(to_radians(m_pitch));
		m_forward = front.get_normal();

		m_right = m_forward.cross(m_world_up).get_normal();
		m_up = m_right.cross(m_forward).get_normal();
	}
}