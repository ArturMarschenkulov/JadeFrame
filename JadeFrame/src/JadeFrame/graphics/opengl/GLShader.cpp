#include "GLShader.h"

#include "../../math/Vec2.h"
#include "../../math/Vec3.h"
#include "../../math/Vec4.h"
#include "../../math/Mat4.h"

#include<array>
#include<tuple>



static auto get_default_shader_flat() -> std::tuple<std::string, std::string> {
	const GLchar* vertex_shader =
		R"(
#version 450 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec2 v_texture_coord;

out vec4 f_color;
out vec2 f_texture_coord;

uniform mat4 MVP;
uniform mat4 view_projection;
uniform mat4 model;

void main() {
	gl_Position = view_projection * model * vec4(v_position, 1.0);
	f_color = v_color;
	f_texture_coord = v_texture_coord;
}
	)";
	const GLchar* fragment_shader =
		R"(
#version 450 core

in vec4 f_color;
in vec2 f_texture_coord;
out vec4 o_color;

uniform sampler2D texture_0;

void main() {
	
	//o_color = mix(f_color, texture(texture_0, f_texture_coord), 0.8);
	//o_color = texture(texture_0, f_texture_coord);
	o_color = f_color;
}
	)";

	return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}
static auto get_default_shader_with_texture() -> std::tuple<std::string, std::string> {
	const GLchar* vertex_shader =
		R"(
#version 450 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec2 v_texture_coord;

out vec4 f_color;
out vec2 f_texture_coord;

uniform mat4 MVP;
uniform mat4 view_projection;
uniform mat4 model;

void main() {
	gl_Position = view_projection * model * vec4(v_position, 1.0);
	f_color = v_color;
	f_texture_coord = v_texture_coord;
}
	)";
	const GLchar* fragment_shader =
		R"(
#version 450 core

in vec4 f_color;
in vec2 f_texture_coord;
out vec4 o_color;

uniform sampler2D texture_0;

void main() {
	
	o_color = mix(f_color, texture(texture_0, f_texture_coord), 0.8);
	o_color = texture(texture_0, f_texture_coord);
	//o_color = f_color;
}
	)";

	return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}

static auto get_default_shader_depth_testing() -> std::tuple<std::string, std::string> {
	const GLchar* vertex_shader =
		R"(
#version 450 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec2 v_texture_coord;

out vec4 f_color;
out vec2 f_texture_coord;

uniform mat4 view_projection;
uniform mat4 model;

void main() {
	gl_Position = view_projection * model * vec4(v_position, 1.0);
	f_color = v_color;
	f_texture_coord = v_texture_coord;
}
	)";
	const GLchar* fragment_shader =
		R"(
#version 450 core
out vec4 o_color;

const float far = 100.0; 
float linearize_depth(float depth) {
	float near = 0.1;
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main() {             
    float depth = linearize_depth(gl_FragCoord.z) / far; // divide by far to get depth in range [0,1] for visualization purposes
    o_color = vec4(vec3(0.4 - depth), 1.0);
}
	)";

	return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}

static auto get_default_shader_light_client() -> std::tuple<std::string, std::string> {
	const GLchar* vertex_shader =
		R"(
#version 450 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec2 v_texture_coord;
layout (location = 3) in vec3 v_normal;

out vec3 f_fragment_position;
out vec3 f_normal;

uniform mat4 view_projection;
uniform mat4 model;

void main() {
	f_fragment_position = vec3(model * vec4(v_position, 1.0));
	f_normal = mat3(transpose(inverse(model))) * v_normal; //v_normal;
	gl_Position = view_projection * model * vec4(f_fragment_position, 1.0);

}
	)";
	const GLchar* fragment_shader =
		R"(
#version 450 core
out vec4 o_color;

in vec3 f_normal;
in vec3 f_fragment_position;

uniform vec3 light_position;
uniform vec3 light_color;
uniform vec3 object_color;

void main(){
	float ambient_strength = 0.1;
	vec3 ambient = ambient_strength * light_color;

	vec3 norm = normalize(f_normal);
	vec3 light_direction = normalize(light_position - f_fragment_position);
	float difference = max(dot(norm, light_direction), 0.0);
	vec3 diffuse = difference * light_color;

	vec3 result = (ambient + diffuse) * object_color;
    o_color = vec4(result, 1.0);
}
	)";

	return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}
static auto get_default_shader_light_server() -> std::tuple<std::string, std::string> {
	const GLchar* vertex_shader =
		R"(
#version 450 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec2 v_texture_coord;

uniform mat4 view_projection;
uniform mat4 model;

void main() {
	gl_Position = view_projection * model * vec4(v_position, 1.0);
}
	)";
	const GLchar* fragment_shader =
		R"(
#version 450 core
out vec4 o_color;

void main()
{             
    o_color = vec4(1.0);
}
	)";

	return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}


OpenGL_Shader::OpenGL_Shader(const std::string& name)
	: m_program()
	, m_vertex_shader(GL_VERTEX_SHADER)
	, m_fragment_shader(GL_FRAGMENT_SHADER) {

	std::string vs_default, fs_default;
	if (name == "flat_0") {
		std::tie(vs_default, fs_default) = get_default_shader_flat();
	} else if (name == "with_texture_0") {
		std::tie(vs_default, fs_default) = get_default_shader_with_texture();
	} else if (name == "depth_testing_0") {
		std::tie(vs_default, fs_default) = get_default_shader_depth_testing();
	} else if (name == "light_server") {
		std::tie(vs_default, fs_default) = get_default_shader_light_server();
	} else if (name == "light_client") {
		std::tie(vs_default, fs_default) = get_default_shader_light_client();
	}

	m_vertex_shader.set_source(vs_default);
	m_vertex_shader.compile();

	m_fragment_shader.set_source(fs_default);
	m_fragment_shader.compile();

	m_program.attach(m_vertex_shader);
	m_program.attach(m_fragment_shader);
	m_program.link();
	m_program.validate();
	m_program.detach(m_vertex_shader);
	m_program.detach(m_fragment_shader);


	m_vertex_source = vs_default;
	m_fragment_source = fs_default;
	m_uniforms   = m_program.query_uniforms(GL_ACTIVE_UNIFORMS);
	m_attributes = m_program.query_uniforms(GL_ACTIVE_ATTRIBUTES);
}

auto OpenGL_Shader::bind() const -> void {
	m_program.bind();
}
auto OpenGL_Shader::unbind() const -> void {
	m_program.unbind();
}

auto OpenGL_Shader::get_uniform_location(const std::string& name) const -> GLint {
	return m_program.get_uniform_location(name);

	//GLint location = glGetUniformLocation(m_program.m_ID, name.c_str());
	//if (location == -1) {
	//	std::cout << "Location of " << name << " can not be found" << std::endl;
	//	__debugbreak();
	//}
	//return location;

	//if (m_uniform_location_cache.find(name) != m_uniform_location_cache.end()) {
	//	return m_uniform_location_cache[name];
	//} else {
	//	GLint location = glGetUniformLocation(m_ID, name.c_str());
	//	m_uniform_location_cache[name] = location;
	//	return location;
	//}

	//// read from uniform/attribute array as originally obtained from OpenGL
	//for (unsigned int i = 0; i < m_uniforms.size(); ++i) {
	//	if (m_uniforms[i].name == name) {
	//		return m_uniforms[i].location;
	//	}
	//}
	//std::cout << "Location of " << name << " can not be found" << std::endl;
	//__debugbreak();
	//return -1;
}


auto OpenGL_Shader::set_uniform(const std::string& name, const int value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform1i(loc, value);
}

auto OpenGL_Shader::set_uniform(const std::string& name, const unsigned int value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform1ui(loc, value);
}
auto OpenGL_Shader::set_uniform(const std::string& name, float value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform1f(loc, value);
}
auto OpenGL_Shader::set_uniform(const std::string& name, const Vec2& value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform2f(loc, value.x, value.y);
}
auto OpenGL_Shader::set_uniform(const std::string& name, const Vec3& value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform3f(loc, value.x, value.y, value.z);
}
auto OpenGL_Shader::set_uniform(const std::string& name, const Vec4& value) const -> void {
	GLint loc = get_uniform_location(name);
	glUniform4f(loc, value.x, value.y, value.z, value.w);
}
auto OpenGL_Shader::set_uniform_matrix(const std::string& name, const Mat4& mat) const -> void {
	GLint loc = get_uniform_location(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

////-------------------------------
//
//
//struct Shader {
//	Shader(GLenum shader_type) {
//		m_ID = glCreateShader(shader_type);
//	}
//	~Shader() {
//		glDeleteShader(m_ID);
//	}
//	auto set_shader_source(const std::string& code_source) -> void {
//		const GLchar* shader_code = code_source.c_str();
//		glShaderSource(m_ID, 1, &shader_code, nullptr);
//	}
//
//	auto compile() -> void {
//		glCompileShader(m_ID);
//	}
//	auto get_info(GLenum pname) -> GLint {
//		GLint result;
//		glGetShaderiv(m_ID, pname, &result);
//		return result;
//	}
//	auto get_compile_status() -> GLint {
//		GLint is_compiled = GL_FALSE;
//		glGetShaderiv(m_ID, GL_COMPILE_STATUS, &is_compiled);
//		return is_compiled;
//	}
//	auto get_info_log(GLsizei max_length) -> std::string {
//		GLchar info_log[512];
//		glGetShaderInfoLog(m_ID, max_length, &max_length, &info_log[0]);
//		std::string result(info_log);
//		return result;
//	}
//
//	GLuint m_ID;
//};
//
//struct Program {
//	Program() {
//		m_ID = glCreateProgram();
//	}
//	auto init() -> void {
//		//m_shaders.push_back({ GL_VERTEX_SHADER });
//		//m_shaders[0].set_shader_source(vs_source_flat);
//		//m_shaders[0].compile();
//
//		//m_shaders.push_back({ GL_FRAGMENT_SHADER });
//		//m_shaders[1].set_shader_source(vs_source_flat);
//		//m_shaders[1].compile();
//
//		//Program program;
//		//program.attach(v_shader);
//		//program.attach(f_shader);
//		//program.link();
//
//
//		//program.detach(v_shader);
//		//program.detach(f_shader);
//	}
//	auto attach(const Shader& shader) -> void {
//		glAttachShader(m_ID, shader.m_ID);
//	}
//	auto detach(const Shader& shader) -> void {
//		glDetachShader(m_ID, shader.m_ID);
//	}
//	auto link() -> void {
//		glLinkProgram(m_ID);
//	}
//	auto get_info(GLenum pname) -> GLint {
//		GLint result;
//		glGetProgramiv(m_ID, pname, &result);
//		return result;
//	}
//	auto use() -> void {
//		glUseProgram(m_ID);
//	}
//
//	auto get_info_log(GLsizei max_length) -> std::string {
//		GLchar info_log[512];
//		glGetProgramInfoLog(m_ID, max_length, &max_length, &info_log[0]);
//		std::string result(info_log);
//		return result;
//	}
//	GLuint m_ID;
//	std::vector<Shader> m_shaders; //0:vertex_shader, 1: fragment_shader
//};
//
//struct OpenGL_ShaderProgram {
//	Program program;
//};