#include "Camera.h"

//#if 1 // NOTE: for camera cotrolling. Remove if we move it
#include "../base_app.h"
#include "../platform/windows/windows_input_manager.h"
#include "../math/Math.h"
//#endif




auto Camera::perspective(const Vec3& pos, const float fovy, const float aspect, const float zNear, const float zFar) -> void {

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
	return Mat4::look_at(m_position, m_position + m_front, m_up);
	
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
		//BaseApp::get_instance()->m_renderer->matrix_stack.projection_matrix = get_projection_matrix();
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

auto Camera::get_projection_matrix() const -> Mat4 {
	return m_projection_matrix;
}

auto Camera1::perspective(const Vec3& position, const float fov, const float aspect, const float zNear, const float zFar) -> void {
	m_mode = MODE::PERSPECTIVE;
	// probably move to constructor
	m_position = position;
	m_forward = { -1.0f, 0.0f, 0.0f };
	m_up = { 0.0f, 0.0f, 1.0f };

	m_projection = Mat4::perspective(fov, aspect, zNear, zFar);
	m_fov = fov;
	m_aspect = aspect;
	m_near = zNear;
	m_far = zFar;
}

auto Camera1::othographic(float left, float right, float bottom, float top, float near_, float far_) -> void {
	m_mode = MODE::PERSPECTIVE;
	m_projection = Mat4::ortho(left, right, bottom, top, near_, far_);
	m_position = 0;
	m_forward = { -1.0f, 0.0f, 0.0f };
	m_up = { 0.0f, 0.0f, 1.0f };
}

//auto Camera1::perspective(Vec3 position, float fov, float aspect, float zNear, float zFar) -> void {
//	// probably move to constructor
//	m_position = position;
//	m_forward = { -1.0f, 0.0f, 0.0f };
//	m_up = { 0.0f, 0.0f, 1.0f };
//	//m_forward = { 0.0f, 0.0f, -1.0f };
//	//m_up = { 0.0f, 1.0f, 0.0f };
//
//	m_mode = MODE::PERSPECTIVE;
//
//	m_projection = Mat4::perspective(fov, aspect, zNear, zFar);
//	m_fov = fov;
//	m_aspect = aspect;
//	m_near = zNear;
//	m_far = zFar;
//
//
//	float yaw = 0;
//	float pitch = 0;
//
//	//Vec3 front;
//	//front.x = cos(to_radians(yaw)) * cos(to_radians(pitch));
//	//front.y = sin(to_radians(pitch));
//	//front.z = sin(to_radians(yaw)) * cos(to_radians(pitch));
//	//m_forward = front.normalize();
//}

auto Camera1::control() -> void {
	float r = 0.1f;
	auto jf = JadeFrame::get_singleton();
	Windows_InputManager i = jf->m_input_manager;

	Vec3 right = { 0.0f, 1.0f, 0.0f };
	Vec3 front = { -1.0f, 0.0f, 0.0f };
	//if (i.is_key_down(KEY::E)) m_position += m_up * r;
	//if (i.is_key_down(KEY::Q)) m_position -= m_up * r;

	if (i.is_key_down(KEY::A)) m_position -= right * r;
	if (i.is_key_down(KEY::D)) m_position += right * r;

	if (i.is_key_down(KEY::S)) m_position -= front * r;
	if (i.is_key_down(KEY::W)) m_position += front * r;

	//if (i.is_key_down(KEY::UP))    m_pitch += r;
	//if (i.is_key_down(KEY::DOWN))  m_pitch -= r;
	//if (i.is_key_down(KEY::RIGHT)) m_yaw += r;
	//if (i.is_key_down(KEY::LEFT))  m_yaw -= r;


	//static float old_fovy = m_fovy;
	//if (i.is_key_down(KEY::R))  m_fovy += to_radians(r);
	//if (i.is_key_down(KEY::F))  m_fovy -= to_radians(r);

	//if (old_fovy != m_fovy) {
	//	old_fovy = m_fovy;
	//	perspective(m_position, m_fovy, m_aspect, m_zNear, m_zFar);
	//	__debugbreak();
	//	//BaseApp::get_instance()->m_renderer->matrix_stack.projection_matrix = get_projection_matrix();
	//}




	float yaw = 0;
	float pitch = 0;
	Vec3 ffront;
	ffront.x = cos(to_radians(yaw)) * cos(to_radians(pitch));
	ffront.y = sin(to_radians(pitch));
	ffront.z = sin(to_radians(yaw)) * cos(to_radians(pitch));
	m_forward = ffront.normalize();

	////m_right = m_worldUp.cross(m_front).normalize();
	////m_up = m_front.cross(m_right).normalize();

	////m_right = m_front.cross(m_worldUp).normalize();
	////m_up = m_right.cross(m_front).normalize();
}



namespace Test1 {
// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum class CAMERA_MOVEMENT : int {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
class Camera {
public:
	// camera Attributes
	Vec3 m_position;
	Vec3 m_front;
	Vec3 m_up;
	Vec3 m_right;
	Vec3 m_world_up;
	// euler Angles
	float m_yaw;
	float m_pitch;
	// camera options
	float m_movement_speed;
	float m_mouse_sensitivity;
	float m_zoom;

	// constructor with vectors
	Camera(Vec3 position = Vec3(0.0f, 0.0f, 0.0f), Vec3 up = Vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) 
		: m_front(Vec3(0.0f, 0.0f, -1.0f))
		, m_movement_speed(SPEED)
		, m_mouse_sensitivity(SENSITIVITY)
		, m_zoom(ZOOM) {
		m_position = position;
		m_world_up = up;
		m_yaw = yaw;
		m_pitch = pitch;
		this->update_camera_vectors();
	}
	// constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) 
		: m_front(Vec3(0.0f, 0.0f, -1.0f))
		, m_movement_speed(SPEED)
		, m_mouse_sensitivity(SENSITIVITY)
		, m_zoom(ZOOM) {
		m_position = Vec3(posX, posY, posZ);
		m_world_up = Vec3(upX, upY, upZ);
		m_yaw = yaw;
		m_pitch = pitch;
		this->update_camera_vectors();
	}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	Mat4 get_view_matrix() {
		return Mat4::look_at(m_position, m_position + m_front, m_up);
	}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void process_keyboard(CAMERA_MOVEMENT direction, float deltaTime) {
		float velocity = m_movement_speed * deltaTime;
		switch (direction) {
			case CAMERA_MOVEMENT::FORWARD	: m_position += m_front * velocity; break;
			case CAMERA_MOVEMENT::BACKWARD	: m_position -= m_front * velocity; break;
			case CAMERA_MOVEMENT::LEFT		: m_position -= m_right * velocity; break;
			case CAMERA_MOVEMENT::RIGHT		: m_position += m_right * velocity; break;
		}
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void process_mouse_movement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
		xoffset *= m_mouse_sensitivity;
		yoffset *= m_mouse_sensitivity;

		m_yaw += xoffset;
		m_pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch) {
			if (m_pitch > 89.0f)
				m_pitch = 89.0f;
			if (m_pitch < -89.0f)
				m_pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		this->update_camera_vectors();
	}

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void process_mouse_scroll(float yoffset) {
		m_zoom -= (float)yoffset;
		if (m_zoom < 1.0f)
			m_zoom = 1.0f;
		if (m_zoom > 45.0f)
			m_zoom = 45.0f;
	}

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void update_camera_vectors() {
		// calculate the new Front vector
		Vec3 front;
		front.x = cos(to_radians(m_yaw)) * cos(to_radians(m_pitch));
		front.y = sin(to_radians(m_pitch));
		front.z = sin(to_radians(m_yaw)) * cos(to_radians(m_pitch));
		//Front = glm::normalize(front);
		m_front = front.normalize();
		// also re-calculate the Right and Up vector
		m_right = m_front.cross(m_world_up).normalize();  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		m_up = m_right.cross(front).normalize();
	}
};
}