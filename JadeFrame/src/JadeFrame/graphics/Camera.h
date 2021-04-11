#pragma once

#include "../math/mat_4.h"
#include "../math/vec_3.h"

class Camera {
public:
	Camera() {
	}

	auto perspective(const Vec3& pos, const float fov, const float aspect, const float zNear, const float zFar) -> void;
	auto get_projection_matrix() const->Matrix4x4;
	auto get_view_matrix() const->Matrix4x4;

	Matrix4x4 m_projection_matrix{};
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
	auto control() -> void;
};

class Camera1 {
	enum class MODE {
		ORTHOGRAPHIC,
		PERSPECTIVE,
	};
public:
	auto perspective(const Vec3& position, const float fov, const float aspect, const float zNear, const float zFar) -> void;
	auto othographic(const float left, const float right, const float buttom, const float top, const float near_, const float far_) -> void;
	auto get_projection_matrix() const->Matrix4x4 {
		return m_projection_matrix;
	}
	auto get_view_matrix() const->Matrix4x4 {
		return Matrix4x4::look_at_matrix(m_position, m_position + m_forward, m_up);
	}

public:
	MODE m_mode;
	Matrix4x4 m_projection_matrix = {};
	Matrix4x4 m_view_matrix = {};

	Vec3 m_position = {};
	Vec3 m_forward = {}; // front
	Vec3 m_up = {};
	Vec3 m_world_up = {};
	Vec3 m_right = {};

	float m_fov = {};
	float m_aspect = {};
	float m_near = {};
	float m_far = {};

	float m_yaw = 0;
	float m_pitch = {};

	auto control() -> void;
};

class Camera0 {
	Matrix4x4 m_projection;
	Matrix4x4 m_view;

	Vec3 m_position;
	Vec3 m_forward;
	Vec3 m_up;
	Vec3 m_right;

	float m_FOV;
	float m_aspect;
	float m_near;
	float m_far;
	bool m_is_perspective;

	auto set_perspective(const float fov, const float aspect, const float t_near, const float t_far) -> void {
		m_is_perspective = true;
		m_projection = Matrix4x4::perspective_projection_matrix(fov, aspect, t_near, t_far);
		m_FOV = fov;
		m_aspect = aspect;
		m_near = t_near;
		m_far = t_far;
	}
	auto set_orthographic(const float left, const float right, const float top, const float bottom, const float t_near, const float t_far) -> void {
		m_is_perspective = false;
		m_projection = Matrix4x4::orthogonal_projection_matrix(left, right, top, bottom, t_near, t_far);
		m_near = t_near;
		m_far = t_far;
	}
	auto update_view() -> void {
		m_view = Matrix4x4::look_at_matrix(m_position, m_position + m_forward, m_up);
	}
};