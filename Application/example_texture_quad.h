#pragma once
#include<JadeFrame.h>

namespace JadeFrame {

struct Example_Texture_Quad : public BaseApp {
	Example_Texture_Quad(const DESC& desc);
	virtual ~Example_Texture_Quad() = default;

	virtual auto on_init() -> void override;
	virtual auto on_update() -> void override;
	virtual auto on_draw() -> void override;

public:
	Object m_obj;
	MaterialHandle m_material;
};

Example_Texture_Quad::Example_Texture_Quad(const DESC& desc)
	: BaseApp(desc) {

}

auto Example_Texture_Quad::on_init() -> void {
	m_renderer->set_clear_color({ 50_u8, 230_u8, 250_u8, 253_u8 });
	m_camera.othographic_mode(0, m_windows[0].get_size().x, m_windows[0].get_size().y, 0, -1, 1);

	const f32 s = 0.5f;
	VertexData* vertex_data = new VertexData();
	auto pos_v1 = std::vector<v3>{
		{ -s, +s, 0.0f },
		{ +s, +s, 0.0f },
		{ -s, -s, 0.0f }
	};
	vertex_data->m_positions = pos_v1;
	vertex_data->m_texture_coordinates = {
		{ -1, +1 },
		{ +1, +1 },
		{ -1, -1 },
	};


	m_obj.m_vertex_data = vertex_data;

	m_obj.m_vertex_format = VertexFormat{
		{ "v_position", SHADER_TYPE::FLOAT_3 },
		{ "v_uv", SHADER_TYPE::FLOAT_2 },
	};

	ShaderHandle::DESC shader_handle_desc;
	shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("spirv_test_0");
	shader_handle_desc.vertex_format = m_obj.m_vertex_format;
	m_material.m_shader_handle = new ShaderHandle(shader_handle_desc);
	m_material.m_texture_handle = new TextureHandle("C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\wall.jpg");
	m_obj.m_material_handle = &m_material;

}
auto Example_Texture_Quad::on_update() -> void {

}
auto Example_Texture_Quad::on_draw() -> void {
	m_renderer->submit(m_obj);
}
using TestApp = Example_Texture_Quad;
}