#pragma once
#include<JadeFrame.h>
#include <JadeFrame/math/Math.h>


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


	const f32 s = 0.5f;
	VertexData vertex_data;
	auto pos_v1 = std::vector<Vec3>{
		{ +0, +s, 0.0f },
		{ +s, -s, 0.0f },
		{ -s, -s, 0.0f }
	};
	auto pos_v2 = std::vector<Vec3>{
		{ -s, +s, 0.0f },
		{ +s, +s, 0.0f },
		{ -s, -s, 0.0f }
	};



	vertex_data.m_positions = pos_v2;
	vertex_data.m_colors = {
		{1.0f, 0.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 1.0f, 1.0f},
	};

	m_mesh.add_to_data(vertex_data);
	m_obj.m_mesh = &m_mesh;

	m_obj.m_vertex_format = VertexFormat{
		{ "v_position", SHADER_TYPE::FLOAT_3 },
		{ "v_color", SHADER_TYPE::FLOAT_4 },
	};
	ShaderHandle::DESC shader_handle_desc;
	shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("spirv_test_0");
	shader_handle_desc.vertex_format = m_obj.m_vertex_format;
	m_material.m_shader_handle = new ShaderHandle(shader_handle_desc);
	m_material.m_texture_handle = nullptr;

	m_obj.m_material_handle = &m_material;


}
auto Example_Rotating_Primitive::on_update() -> void {

}
auto Example_Rotating_Primitive::on_draw() -> void {
	static auto start_time = std::chrono::high_resolution_clock::now();

	auto current_time = std::chrono::high_resolution_clock::now();
	f32 time = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - start_time).count();

	m_obj.m_transform = Matrix4x4::rotation_matrix(
		time * to_radians(90.0f),
		Vec3(0.0f, 0.0f, 1.0f)
	);
	m_renderer->submit(m_obj);
}
using TestApp = Example_Rotating_Primitive;

}