#include "opengl_shader_loader.h"

#include <glad/glad.h>



/*
	Conventions for variables
	v_ = from CPU to vertex
	f_ = from vertex to fragment
	o_ = from fragment to output
	u_ = uniform
	   = local variables
*/
static auto get_default_shader_flat() -> std::tuple<std::string, std::string> {
	const GLchar* vertex_shader =
		R"(
#version 450 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec2 v_texture_coord;

out vec4 f_color;
out vec2 f_texture_coord;

uniform mat4 u_MVP;
uniform mat4 u_view_projection;
uniform mat4 u_model;

void main() {
	gl_Position = u_view_projection * u_model * vec4(v_position, 1.0);
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

uniform sampler2D u_texture_0;

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

uniform mat4 u_MVP;
uniform mat4 u_view_projection;
uniform mat4 u_model;

void main() {
	f_color = v_color;
	f_texture_coord = v_texture_coord;
	vec3 fragment_position = vec3(model * vec4(v_position, 1.0));
	gl_Position = u_view_projection * u_model * vec4(fragment_position, 1.0);
}
	)";
	const GLchar* fragment_shader =
		R"(
#version 450 core

in vec4 f_color;
in vec2 f_texture_coord;
out vec4 o_color;

uniform sampler2D u_texture_0;

void main() {
	
	//o_color = mix(f_color, texture(texture_0, f_texture_coord), 0.8);
	o_color = texture(u_texture_0, f_texture_coord);
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

uniform mat4 u_view_projection;
uniform mat4 u_model;

void main() {
	gl_Position = u_view_projection * u_model * vec4(v_position, 1.0);
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

uniform mat4 u_view_projection;
uniform mat4 u_model;

void main() {
	f_fragment_position = vec3(u_model * vec4(v_position, 1.0));
	f_normal = mat3(transpose(inverse(u_model))) * v_normal; //v_normal;
	gl_Position = u_view_projection * u_model * vec4(f_fragment_position, 1.0);

}
	)";
	const GLchar* fragment_shader =
		R"(
#version 450 core
out vec4 o_color;

in vec3 f_normal;
in vec3 f_fragment_position;

uniform vec3 u_light_position;
uniform vec3 u_light_color;
uniform vec4 u_object_color;
uniform vec3 u_view_position;

uniform float u_specular_strength;

void main(){
	// ambient
	float ambient_strength = 0.1;
	vec3 ambient = ambient_strength * u_light_color;

	// diffuse
	vec3 norm = normalize(f_normal);
	vec3 light_direction = normalize(light_position - f_fragment_position);
	float difference = max(dot(norm, u_light_direction), 0.0);
	vec3 diffuse = difference * u_light_color;

	// specular
	//float specular_strength = 0.5;
	vec3 view_direction = normalize(u_view_position - f_fragment_position);
	vec3 reflect_direction = reflect(-u_light_direction, norm);
	float spec = pow(max(dot(u_view_direction, reflect_direction), 0.0), 256);
	vec3 specular = u_specular_strength * spec * u_light_color;

	vec4 result = vec4((ambient + diffuse + specular), 1.0) * object_color;
    o_color = result;
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

uniform mat4 u_view_projection;
uniform mat4 u_model;

void main() {
	gl_Position = u_view_projection * u_model * vec4(v_position, 1.0);
}
	)";
	const GLchar* fragment_shader =
		R"(
#version 450 core
out vec4 o_color;

uniform vec3 u_light_color;
void main()
{             
    o_color = vec4(u_light_color, 1.0); // vec4(1.0);
}
	)";

	return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}

auto load_shader_by_name(const std::string& name) -> std::tuple<std::string, std::string> {
	std::tuple<std::string, std::string> shader_tuple;
	if (name == "flat_0") {
		shader_tuple = get_default_shader_flat();
	} else if (name == "with_texture_0") {
		shader_tuple = get_default_shader_with_texture();
	} else if (name == "depth_testing_0") {
		shader_tuple = get_default_shader_depth_testing();
	} else if (name == "light_server") {
		shader_tuple = get_default_shader_light_server();
	} else if (name == "light_client") {
		shader_tuple = get_default_shader_light_client();
	}
	return shader_tuple;
}