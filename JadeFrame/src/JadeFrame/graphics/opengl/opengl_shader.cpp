#include "opengl_shader.h"

#include "../../math/vec_2.h"
#include "../../math/vec_3.h"
#include "../../math/vec_4.h"
#include "../../math/mat_4.h"

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
	m_uniforms = m_program.query_uniforms(GL_ACTIVE_UNIFORMS);
	m_attributes = m_program.query_uniforms(GL_ACTIVE_ATTRIBUTES);
	//m_vertex_shader.query_source();
}

auto OpenGL_Shader::bind() const -> void {
	m_program.bind();
}
auto OpenGL_Shader::unbind() const -> void {
	m_program.unbind();
}

auto OpenGL_Shader::get_uniform_location(const std::string& name) const -> GLint {
	if (m_uniforms.contains(name)) {
		return m_uniforms.at(name).location;
	}
	__debugbreak();
}

auto OpenGL_Shader::set_uniform(const std::string& name, const Vec3& value) -> void {
	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<Vec3>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			auto& v = std::get<Vec3>(m_uniforms[name].value);
			glUniform3f(m_uniforms[name].location, v.x, v.y, v.z);
			return;
		}
	}

	__debugbreak();
}
auto OpenGL_Shader::set_uniform(const std::string& name, const Mat4& value) -> void {
	if (m_uniforms.contains(name)) {
		if (std::holds_alternative<Mat4>(m_uniforms[name].value)) {
			m_uniforms[name].value = value;
			glUniformMatrix4fv(m_uniforms[name].location, 1, GL_FALSE, &std::get<Mat4>(m_uniforms[name].value)[0][0]);
			return;
		}
	}
	__debugbreak();
}

auto OpenGL_Shader::update_uniforms() -> void {
	for (auto& uniform : m_uniforms) {
		auto variable = uniform.second;
		if (std::holds_alternative<Mat4>(variable.value)) {
			glUniformMatrix4fv(variable.location, 1, GL_FALSE, &std::get<Mat4>(variable.value)[0][0]);
		} else if (std::holds_alternative<Vec3>(variable.value)) {
			auto& v = std::get<Vec3>(variable.value);
			glUniform3f(variable.location, v.x, v.y, v.z);
		} else {
			__debugbreak();
		}
	}
}
