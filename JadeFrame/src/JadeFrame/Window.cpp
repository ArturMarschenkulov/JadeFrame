#include "Window.h"
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

Window::Window() : m_handle(nullptr) {}

void Window::init(const std::string& title, float width, float height) {
	m_size = { width, height };
	m_title = title;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_handle = glfwCreateWindow(m_size.x, m_size.y, m_title.c_str(), NULL, NULL);
	if(m_handle == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(m_handle);
	glfwSetFramebufferSizeCallback(m_handle, framebuffer_size_callback);



	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	glfwSwapInterval(1);
}

GLFWwindow* Window::get_handle() {
	return m_handle;
}

Vec2 Window::get_size() {
	return m_size;
}

float Window::get_width() {
	return m_size.x;
}

float Window::get_height() {
	return m_size.y;
}
