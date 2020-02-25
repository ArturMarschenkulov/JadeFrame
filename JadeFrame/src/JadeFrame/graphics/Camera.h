#pragma once
#include "../math/Mat4.h"
#include "../math/Vec3.h"


enum class CAMERA_MODE {
	ORTHO,
	PERSPECTIVE
};
class Camera {
public:
	Camera() {}

	void set_mode(CAMERA_MODE mode) {
		m_mode = mode;
	}
	void perspective(const Vec3 pos, float fov, float aspect, float zNear, float zFar) {
		m_perspective = Mat4::perspective(fov, aspect, zNear, zFar);
		m_position = pos;
		m_forward = { 0,0,1 };
		m_up = { 0, 1,0 };
	}
	Mat4 get_view_projection() const {
		return Mat4::lookAt(m_position, m_position + m_forward, m_up);
	}

	Mat4 m_perspective;
	Vec3 m_position;
	Vec3 m_forward;
	Vec3 m_up;

	CAMERA_MODE m_mode;
};