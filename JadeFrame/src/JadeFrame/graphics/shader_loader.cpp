#include "shader_loader.h"

#include "graphics_language.h"

// #include <glad/glad.h>

namespace JadeFrame {

/*
        Conventions for variables
        v_ = from CPU to vertex
        f_ = from vertex to fragment
        o_ = from fragment to output
        u_ = uniform
           = local variables


        Certain variables should be inside a shader.
                "v_position" is the actual position
                "u_view_projection"
                "u_model"

        The structure is:
                in variables with layout
                out variables
                uniforms


        Sets:
        0 = per frame
        1 = per pass
        2 = per material
        3 = per drawcall/object

*/

/*
    This comment will be about how to write shader code to make it as portable as possible
   across different APIs. The idea is that the main dialect is the Vulkan dialect of GLSL.
   This can be compiled to the OpenGL dialect in case of OpenGL or to HSHL in case of
   DirectX and its various versions. Here I gather resources to achieve that goal.




    Links:
    https://www.khronos.org/assets/uploads/developers/library/2016-vulkan-devday-uk/10-Porting-to-Vulkan.pdf
    https://community.arm.com/arm-community-blogs/b/graphics-gaming-and-vr-blog/posts/porting-a-graphics-engine-to-the-vulkan-api
    https://on-demand.gputechconf.com/gtc/2016/events/vulkanday/Migrating_from_OpenGL_to_Vulkan.pdf
    https://developer.nvidia.com/transitioning-opengl-vulkan
    https://gpuopen.com/wp-content/uploads/2017/03/GDC2017-D3D12-And-Vulkan-Lessons-Learned.pdf

*/

static auto get_shader_framebuffer_test_0() {

    const char* vertex_shader =
        R"(
#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_texture_coordinate;

layout(location = 0) out vec2 f_texture_coordinate;

void main() {
	gl_Position = vec4(v_position.x, v_position.y, 0.0, 1.0);
	f_texture_coordinate = v_texture_coordinate;
}
	)";
    const char* fragment_shader =
        R"(
#version 450 core
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec2 f_texture_coordinate;

layout(location = 0) out vec4 o_color;

layout(binding = 0) uniform sampler2D u_screen_texture;

float convert_srgb_to_linear(float rgba_val) {
    return rgba_val <= 0.04045f 
        ? rgba_val / 12.92f 
        : pow((rgba_val + 0.055f) / 1.055f, 2.4f);
}

void main() {
    float gamma = 2.2;
	o_color = texture(u_screen_texture,  f_texture_coordinate);
    o_color.x = convert_srgb_to_linear(o_color.x);
    o_color.y = convert_srgb_to_linear(o_color.y);
    o_color.z = convert_srgb_to_linear(o_color.z);
    // o_color = vec4(pow(o_color.xyz, vec3(gamma)), o_color.w);
}
)";

    return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}

/*
    This is the vulkan testing shader which uses the 4 descriptor sets philosophy.
    0 = per frame
    1 = per pass
    2 = per material
    3 = per drawcall/object
*/
static auto get_shader_spirv_test_1() -> std::tuple<std::string, std::string> {
    static const char* vertex_shader =
        R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec4 v_color;

layout(location = 0) out vec4 f_color;

layout(std140, set = 0, binding = 0) uniform Camera {
    mat4 view_projection;
} u_camera;

layout(std140, set = 3, binding = 0) uniform Transform {
	mat4 model;
} u_transform;

void main() {
	gl_Position = u_camera.view_projection * u_transform.model * vec4(v_position, 1.0);

	f_color = v_color;
}
)";

    static const char* fragment_shader =
        R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 f_color;

layout(location = 0) out vec4 o_color;

void main() {    
    if(!gl_FrontFacing) {
        o_color = vec4(1.0, 0.412, 0.7, 1.0);
        return;
    }
    // float gamma = 1.0;
    // o_color = vec4(pow(f_color.xyz, vec3(gamma)), f_color.w);
    o_color = f_color;
}
)";

    return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}

static auto get_shader_spirv_test_0() -> std::tuple<std::string, std::string> {
    static const char* vertex_shader =
        R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec4 v_color;

layout(location = 0) out vec4 f_color;

layout(std140, set = 0, binding = 0) uniform Camera {
    mat4 view_projection;
} u_camera;

layout(std140, set = 0, binding = 1) uniform Transform {
	mat4 model;
} u_transform;

void main() {
	gl_Position = u_camera.view_projection * u_transform.model * vec4(v_position, 1.0);

	f_color = v_color;
}
)";

    static const char* fragment_shader =
        R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 f_color;

layout(location = 0) out vec4 o_color;

void main() {
    o_color = f_color;
}
)";

    return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}

static auto get_default_shader_flat_0() -> std::tuple<std::string, std::string> {
    const char* vertex_shader =
        R"(
#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec4 v_color;

layout(location = 0) out vec4 f_color;

layout(std140, set = 0, binding = 0) uniform Camera {
    mat4 view_projection;
} u_camera;

layout(std140, set = 3, binding = 0) uniform Transform {
	mat4 model;
} u_transform;

void main() {
	gl_Position = u_camera.view_projection * u_transform.model * vec4(v_position, 1.0);
	f_color = v_color;		
	f_texture_coord = v_texture_coord;
}
)";
    const char* fragment_shader =
        R"(
#version 450 core

layout (location = 0) in vec4 f_color;
layout (location = 1) in vec2 f_texture_coord;

layout (location = 0) out vec4 o_color;

void main() {
    if(!gl_FrontFacing) {
        o_color = vec4(1.0, 0.412, 0.7, 1.0);
        return;
    }
	o_color = f_color;
}
)";

    return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}

static auto get_default_shader_with_texture() -> std::tuple<std::string, std::string> {
    const char* vertex_shader =
        R"(
#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec2 v_texture_coord;

layout(location = 0) out vec4 f_color;
layout(location = 1) out vec2 f_texture_coord;

layout(std140, set = 0, binding = 0) uniform Camera {
    mat4 view_projection;
} u_camera;

layout(std140, set = 3, binding = 0) uniform Transform {
	mat4 model;
} u_transform;

void main() {
	f_color = v_color;
	f_texture_coord = v_texture_coord;
	// vec3 fragment_position = vec3(u_transform.model * vec4(v_position, 1.0));
	gl_Position = u_camera.view_projection * u_transform.model * vec4(v_position, 1.0);
}
	)";
    const char* fragment_shader =
        R"(
#version 450 core

layout(location = 0) in vec4 f_color;
layout(location = 1) in vec2 f_texture_coord;

layout(location = 0) out vec4 o_color;


layout(set = 2, binding = 0) uniform sampler2D u_texture_0;

void main() {
    if(!gl_FrontFacing) {
        o_color = vec4(1.0, 0.412, 0.7, 1.0);
        return;
    }
	
	// o_color = mix(f_color, texture(u_texture_0, f_texture_coord), 0.8);
	o_color = texture(u_texture_0, f_texture_coord);
	//o_color = f_color;


}
)";

    return std::make_tuple(std::string(vertex_shader), std::string(fragment_shader));
}

static auto get_default_shader_depth_testing() -> std::tuple<std::string, std::string> {
    const char* vertex_shader =
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
    const char* fragment_shader =
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
    const char* vertex_shader =
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
    const char* fragment_shader =
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
    const char* vertex_shader =
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
    const char* fragment_shader =
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

// Retrieves a `ShadingCode` object by name.
// Note the resulting code is in SPIRV format.
auto GLSLCodeLoader::get_by_name(const std::string& name) -> ShadingCode {

    using ShaderGetter = std::function<std::tuple<std::string, std::string>()>;
    static const std::unordered_map<std::string, ShaderGetter> shader_map = {
        {          "flat_0",          [&]() { return get_shader_spirv_test_1(); }},
        {  "with_texture_0",  [&]() { return get_default_shader_with_texture(); }},
        {    "spirv_test_1",          [&]() { return get_shader_spirv_test_1(); }},
        { "depth_testing_0", [&]() { return get_default_shader_depth_testing(); }},
        {    "light_server",  [&]() { return get_default_shader_light_server(); }},
        {    "light_client",  [&]() { return get_default_shader_light_client(); }},
        {    "spirv_test_0",          [&]() { return get_shader_spirv_test_0(); }},
        {"framebuffer_test",    [&]() { return get_shader_framebuffer_test_0(); }}
    };
    auto it = shader_map.find(name);
    if (it == shader_map.end()) {
        Logger::err("GLSLCodeLoader::get_by_name: Shader with name {} not found", name);
        assert(false);
    }
    auto shader_tuple = it->second();

    // TODO: Make the graphics API not hardcoded
    auto [vs, fs] = shader_tuple;
    ShadingCode code;
    auto        api = GRAPHICS_API::VULKAN;
    code.m_shading_language = SHADING_LANGUAGE::GLSL_VULKAN;
    code.m_modules.resize(2);
    code.m_modules[0].m_stage = SHADER_STAGE::VERTEX;
    code.m_modules[0].m_code = GLSL_to_SPIRV(vs, SHADER_STAGE::VERTEX, api);
    code.m_modules[1].m_stage = SHADER_STAGE::FRAGMENT;
    code.m_modules[1].m_code = GLSL_to_SPIRV(fs, SHADER_STAGE::FRAGMENT, api);
    return code;
}
} // namespace JadeFrame