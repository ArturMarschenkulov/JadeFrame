#pragma once
#include<JadeFrame.h>


namespace JadeFrame {

struct Example_Rotating_Primitive : public BaseApp {

	Example_Rotating_Primitive(const DESC& desc);
	virtual ~Example_Rotating_Primitive() = default;

	virtual auto on_init() -> void override;
	virtual auto on_update() -> void override;
	virtual auto on_draw() -> void override;

public:
	Object m_obj;
	Mesh m_mesh;
	MaterialHandle m_material;
};

Example_Rotating_Primitive::Example_Rotating_Primitive(const DESC& desc)
	: BaseApp(desc) {

}

auto Example_Rotating_Primitive::on_init() -> void {
	m_renderer->set_clear_color({ 0.2f, 0.0f, 0.0f, 1.0f });
	m_camera.othographic_mode(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);


	const f32 s = 0.8f;
	VertexData vertex_data = VertexDataFactory::make_triangle(
		{ +0, +s, 0.0f },
		{ +s, -s, 0.0f },
		{ -s, -s, 0.0f }
	);
	vertex_data.m_colors = {
		{1.0f, 0.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 1.0f, 1.0f},
	};

	m_mesh.add_to_data(vertex_data);
	m_obj.m_mesh = &m_mesh;

	m_obj.m_buffer_layout = BufferLayout{
		{ SHADER_TYPE::FLOAT_3, "v_position" },
		{ SHADER_TYPE::FLOAT_4, "v_color" },
	};


	GLSLCode glsl_code = GLSLCodeLoader::get_by_name("spirv_test_0");
	m_material.m_shader_handle = new ShaderHandle(glsl_code);
	m_material.m_texture_handle = nullptr;

	m_obj.m_material_handle = &m_material;


}
auto Example_Rotating_Primitive::on_update() -> void {

}
auto Example_Rotating_Primitive::on_draw() -> void {


	m_renderer->submit(m_obj);
}
using TestApp = Example_Rotating_Primitive;

}