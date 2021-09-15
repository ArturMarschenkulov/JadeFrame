#include "pch.h"
#include "camera.h"

#if 1 // NOTE: for camera cotrolling. Remove if we move it
#include "JadeFrame/base_app.h"
#include "JadeFrame/platform/windows/windows_input_manager.h"
#include "JadeFrame/math/math.h"
#endif


//m_camera.perspective_mode(
//	Vec3(0.0f, 0.0f, 3.0f),//{ -20, 10, -5 },
//	to_radians(45.0f),
//	m_current_window_p->m_size.x / m_current_window_p->m_size.y,
//	0.1f,
//	10000.0f
//);

namespace JadeFrame {

auto Camera::perspective(const Vec3& pos, const f32 fovy, const f32 aspect, const f32 zNear, const f32 zFar) -> void {

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
	f32 r = 0.1f;

	Windows_InputManager i = JadeFrameInstance::get_singleton()->m_input_manager;


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


	static f32 old_fovy = m_fovy;
	if (i.is_key_down(KEY::R))  m_fovy += to_radians(r);
	if (i.is_key_down(KEY::F))  m_fovy -= to_radians(r);

	if (old_fovy != m_fovy) {
		old_fovy = m_fovy;
		this->perspective(m_position, m_fovy, m_aspect, m_zNear, m_zFar);
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

auto Camera1::perspective_mode(const Vec3& position, const f32 fov, const f32 aspect, const f32 zNear, const f32 zFar) -> void {
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

auto Camera1::othographic_mode(f32 left, f32 right, f32 bottom, f32 top, f32 near_, f32 far_) -> void {
	assert(left != right);
	assert(bottom != top);
	assert(near_ != far_);


	m_mode = MODE::ORTHOGRAPHIC;
	m_projection_matrix =
		Matrix4x4::orthogonal_projection_matrix(left, right, bottom, top, near_, far_);

	m_position = { 0.0f, 0.0f, 0.0f };

	//m_forward = { 1.0f, 0.0f, 0.0f };
	//m_up = { 0.0f, 1.0f, 0.0f };
	m_forward = { 0.0f, 0.0f, -1.0f };
	m_up = { 0.0f, 1.0f, 0.0f };
}

auto Camera1::get_projection_matrix() const -> Matrix4x4 {
	return m_projection_matrix;
}
auto Camera1::get_view_matrix() const -> Matrix4x4 {
	return Matrix4x4::look_at_matrix(m_position, m_position + m_forward, m_up);
}

auto Camera1::get_view_projection_matrix() const -> Matrix4x4 {
	return this->get_view_matrix() * this->get_projection_matrix();
}

auto Camera1::control() -> void {
	if (m_mode == MODE::PERSPECTIVE) {
		const f32 velocity = 0.1f;
		const Windows_InputManager& i = JadeFrameInstance::get_singleton()->m_input_manager;
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
}