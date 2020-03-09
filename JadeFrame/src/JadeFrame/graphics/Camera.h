#pragma once
#include "../math/Mat4.h"
#include "../math/Vec3.h"

class Camera {
public:
	Camera() {}

	auto perspective(Vec3 pos, float fov, float aspect, float zNear, float zFar) -> void;
	auto get_projection_matrix() const -> Mat4;
	auto get_view_matrix() const -> Mat4;
	auto move() -> void;
	auto update() -> void;

	Mat4 m_projection_matrix;
	Vec3 m_position;
	Vec3 m_up;
	Vec3 m_right;
	Vec3 m_front;
	Vec3 m_worldUp;

	float m_fovy;
	float m_yaw;
	float m_pitch;
	float m_roll;

};