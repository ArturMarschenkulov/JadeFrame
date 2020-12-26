#pragma once
#include "../math/Mat4.h"
#include "../math/Vec3.h"

struct Cam {
	enum class TYPE {
		ORTHOGRAPHIC,
		PERSPECTIVE,
	};
	Cam(TYPE type);

};

class Camera {
public:
	Camera() {
	}

	auto perspective(Vec3 pos, float fov, float aspect, float zNear, float zFar) -> void;
	auto get_projection_matrix() const->Mat4;
	auto get_view_matrix() const->Mat4;
	auto move() -> void;
	auto update() -> void;

	Mat4 m_projection_matrix{};
	Vec3 m_position{};
	Vec3 m_up{};
	Vec3 m_right{};
	Vec3 m_front{};
	Vec3 m_worldUp{};

	float m_fovy{};
	float m_yaw{};
	float m_pitch{};
	float m_roll{};

	float m_aspect;
	float m_zNear;
	float m_zFar;

};


class Camera0 {
	Mat4 m_projection;
	Mat4 m_view;

	Vec3 m_position;
	Vec3 m_forward;
	Vec3 m_up;
	Vec3 m_right;

	float m_FOV;
	float m_aspect;
	float m_near;
	float m_far;
	bool m_is_perspective;

	auto set_perspective(float fov, float aspect, float t_near, float t_far) -> void {
		m_is_perspective = true;
		m_projection = Mat4::perspective(fov, aspect, t_near, t_far);
		m_FOV = fov;
		m_aspect = aspect;
		m_near = t_near;
		m_far = t_far;
	}
	auto set_orthographic(float left, float right, float top, float bottom, float t_near, float t_far) {
		m_is_perspective = false;
		m_projection = Mat4::ortho(left, right, top, bottom, t_near, t_far);
		m_near = t_near;
		m_far = t_far;
	}
	auto update_view() {
		m_view = Mat4::lookAt(m_position, m_position + m_forward, m_up);
	}
};