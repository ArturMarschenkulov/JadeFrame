#include "pch.h"
#include "mesh.h"
namespace JadeFrame {

Mesh::Mesh(const VertexData& vertex_data) {
	this->add_to_data(vertex_data);
}

auto Mesh::add_to_data(const VertexData& vertex_data) -> void {
	m_positions.resize(vertex_data.m_positions.size());
	m_colors.resize(vertex_data.m_colors.size());
	m_tex_coords.resize(vertex_data.m_texture_coordinates.size());

	for (size_t i = 0; i < vertex_data.m_positions.size(); i++) {
		m_positions[i] = vertex_data.m_positions[i];
		if (vertex_data.m_colors.size() > 0) {
			m_colors[i] = vertex_data.m_colors[i];//current_color;
		}
		if (vertex_data.m_texture_coordinates.size() > 0) {
			m_tex_coords[i] = vertex_data.m_texture_coordinates[i];
		}
	}

	m_normals.resize(vertex_data.m_normals.size());
	for (size_t i = 0; i < vertex_data.m_normals.size(); i++) {
		m_normals[i] = vertex_data.m_normals[i];
	}


	m_indices.resize(vertex_data.m_indices.size());
	for (size_t i = 0; i < vertex_data.m_indices.size(); i++) {
		m_indices[i] = vertex_data.m_indices[i];
	}

}

auto Mesh::set_color(const RGBAColor color) -> void {
	if (current_color != color) {
		current_color = color;
		for (size_t i = 0; i < m_positions.size(); i++) {
			m_colors[i] = current_color;
		}
	}
}

auto VertexDataFactory::make_line(const Vec3& pos1, const Vec3& pos2) -> VertexData {
	VertexData vertex_data;
	vertex_data.m_positions.resize(2);
	vertex_data.m_positions[0] = pos1;
	vertex_data.m_positions[1] = pos2;

	vertex_data.m_texture_coordinates.resize(2);
	vertex_data.m_texture_coordinates[0] = { 0.0f, 0.0f };
	vertex_data.m_texture_coordinates[1] = { 0.0f, 0.0f };

	vertex_data.m_indices.reserve(2);
	vertex_data.m_indices = {
		0, 1
	};
	return vertex_data;
}

auto VertexDataFactory::make_rectangle(const Vec3& pos, const Vec3& size, const DESC desc) -> VertexData {
	VertexData vertex_data;
	vertex_data.m_positions.resize(6);
	vertex_data.m_positions[00] = { pos.x			, pos.y			, pos.z };
	vertex_data.m_positions[01] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.m_positions[02] = { pos.x + size.x, pos.y			, pos.z };
	vertex_data.m_positions[03] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.m_positions[04] = { pos.x			, pos.y			, pos.z };
	vertex_data.m_positions[05] = { pos.x			, pos.y + size.y, pos.z };

	if (desc.has_texture_coordinates == true) {
		vertex_data.m_texture_coordinates.resize(6);
		vertex_data.m_texture_coordinates[00] = { +0.0f, +0.0f };
		vertex_data.m_texture_coordinates[01] = { +1.0f, +1.0f };
		vertex_data.m_texture_coordinates[02] = { +1.0f, +0.0f };
		vertex_data.m_texture_coordinates[03] = { +1.0f, +1.0f };
		vertex_data.m_texture_coordinates[04] = { +0.0f, +0.0f };
		vertex_data.m_texture_coordinates[05] = { +0.0f, +1.0f };
	}

	if (desc.has_normals == true) {
		vertex_data.m_normals.resize(6);
		vertex_data.m_normals[00] = { +0.0f, +0.0f, +1.0f };
		vertex_data.m_normals[01] = { +0.0f, +0.0f, +1.0f };
		vertex_data.m_normals[02] = { +0.0f, +0.0f, +1.0f };
		vertex_data.m_normals[03] = { +0.0f, +0.0f, +1.0f };
		vertex_data.m_normals[04] = { +0.0f, +0.0f, +1.0f };
		vertex_data.m_normals[05] = { +0.0f, +0.0f, +1.0f };
	}

	if (desc.has_indices == true) {
		vertex_data.m_indices.reserve(6);
		vertex_data.m_indices = {
			0, 1, 3,
			1, 2, 3
		};
	}

	return vertex_data;
}

auto VertexDataFactory::make_triangle(const Vec3& pos1, const Vec3& pos2, const Vec3& pos3) -> VertexData {
	VertexData vertex_data;
	vertex_data.m_positions.resize(3);
	vertex_data.m_positions[0] = Vec3{ pos1.x, pos1.y, pos1.z };
	vertex_data.m_positions[1] = Vec3{ pos2.x, pos2.y, pos2.z };
	vertex_data.m_positions[2] = Vec3{ pos3.x, pos3.y, pos3.z };

	//vertex_data.m_indices.reserve(3);
	//vertex_data.m_indices = {
	//	0, 1, 2
	//};
	return vertex_data;
}

auto VertexDataFactory::make_circle(const Vec3& position, const f32 radius, const u32 numSegments) -> VertexData {
	const f32 theta = 2.0f * 3.1415926f / f32(numSegments);//get the current angle 
	const f32 cos = cosf(theta);//calculate the x component 
	const f32 sin = sinf(theta);//calculate the y component 


	VertexData vertex_data;
	vertex_data.m_positions.resize(numSegments + 1);
	vertex_data.m_positions[0] = position;
	f32 x = radius;
	f32 y = 0;
	for (u32 i = 1; i < numSegments + 1; i++) {
		vertex_data.m_positions[i] = Vec3{ x + position.x, y + position.y, position.z };//output vertex 

		const f32 t = x;
		x = cos * x - sin * y;
		y = sin * t + cos * y;
	}

	const u32 num_index = (numSegments * 3);
	vertex_data.m_indices.resize(num_index);

	for (u32 i = 0; i < numSegments; i++) {
		vertex_data.m_indices[(3 * i + 0)] = 0;
		vertex_data.m_indices[(3 * i + 1)] = 1 + i;
		vertex_data.m_indices[(3 * i + 2)] = 2 + i;
	}
	vertex_data.m_indices[num_index - 1] = vertex_data.m_indices[1];

	return vertex_data;
}

auto VertexDataFactory::make_cube(const Vec3& pos, const Vec3& size) -> VertexData {
	VertexData vertex_data;

	vertex_data.m_positions.resize(36);
	// back face -z
	vertex_data.m_positions[00] = { pos.x			, pos.y			, pos.z };
	vertex_data.m_positions[01] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.m_positions[02] = { pos.x + size.x, pos.y			, pos.z };
	vertex_data.m_positions[03] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.m_positions[04] = { pos.x			, pos.y			, pos.z };
	vertex_data.m_positions[05] = { pos.x			, pos.y + size.y, pos.z };

	// front face +z
	vertex_data.m_positions[06] = { pos.x			, pos.y			, pos.z + size.z };
	vertex_data.m_positions[07] = { pos.x + size.x, pos.y			, pos.z + size.z };
	vertex_data.m_positions[8l] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.m_positions[9l] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.m_positions[10] = { pos.x			, pos.y + size.y, pos.z + size.z };
	vertex_data.m_positions[11] = { pos.x			, pos.y			, pos.z + size.z };

	// left face -x
	vertex_data.m_positions[12] = { pos.x			, pos.y + size.y, pos.z + size.z };
	vertex_data.m_positions[13] = { pos.x			, pos.y + size.y, pos.z };
	vertex_data.m_positions[14] = { pos.x			, pos.y			, pos.z };
	vertex_data.m_positions[15] = { pos.x			, pos.y			, pos.z };
	vertex_data.m_positions[16] = { pos.x			, pos.y			, pos.z + size.z };
	vertex_data.m_positions[17] = { pos.x			, pos.y + size.y, pos.z + size.z };

	// right face +x
	vertex_data.m_positions[18] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.m_positions[19] = { pos.x + size.x, pos.y			, pos.z };
	vertex_data.m_positions[20] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.m_positions[21] = { pos.x + size.x, pos.y			, pos.z };
	vertex_data.m_positions[22] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.m_positions[23] = { pos.x + size.x, pos.y			, pos.z + size.z };

	// bottom face
	vertex_data.m_positions[24] = { pos.x			, pos.y			, pos.z };
	vertex_data.m_positions[25] = { pos.x + size.x, pos.y			, pos.z };
	vertex_data.m_positions[26] = { pos.x + size.x, pos.y			, pos.z + size.z };
	vertex_data.m_positions[27] = { pos.x + size.x, pos.y			, pos.z + size.z };
	vertex_data.m_positions[28] = { pos.x			, pos.y			, pos.z + size.z };
	vertex_data.m_positions[29] = { pos.x			, pos.y			, pos.z };

	// top face
	vertex_data.m_positions[30] = { pos.x			, pos.y + size.y, pos.z };
	vertex_data.m_positions[31] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.m_positions[32] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.m_positions[33] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.m_positions[34] = { pos.x			, pos.y + size.y, pos.z };
	vertex_data.m_positions[35] = { pos.x			, pos.y + size.y, pos.z + size.z };



	vertex_data.m_texture_coordinates.resize(36);
	{
		// back face -z
		vertex_data.m_texture_coordinates[00] = { +1.0f, +0.0f };
		vertex_data.m_texture_coordinates[01] = { +1.0f, +1.0f };
		vertex_data.m_texture_coordinates[02] = { +0.0f, +1.0f };
		vertex_data.m_texture_coordinates[03] = { +0.0f, +1.0f };
		vertex_data.m_texture_coordinates[04] = { +0.0f, +0.0f };
		vertex_data.m_texture_coordinates[05] = { +1.0f, +0.0f };

		// front face +z
		vertex_data.m_texture_coordinates[06] = { +0.0f, +0.0f };
		vertex_data.m_texture_coordinates[07] = { +0.0f, +0.0f };
		vertex_data.m_texture_coordinates[8] = { +0.0f, +0.0f };
		vertex_data.m_texture_coordinates[9] = { +0.0f, +0.0f };
		vertex_data.m_texture_coordinates[10] = { +0.0f, +0.0f };
		vertex_data.m_texture_coordinates[11] = { +0.0f, +0.0f };

		// left face
		vertex_data.m_texture_coordinates[12] = { -1.0f, +0.0f };
		vertex_data.m_texture_coordinates[13] = { -1.0f, +0.0f };
		vertex_data.m_texture_coordinates[14] = { -1.0f, +0.0f };
		vertex_data.m_texture_coordinates[15] = { -1.0f, +0.0f };
		vertex_data.m_texture_coordinates[16] = { -1.0f, +0.0f };
		vertex_data.m_texture_coordinates[17] = { -1.0f, +0.0f };

		// right face +x
		vertex_data.m_texture_coordinates[18] = { +1.0f, +0.0f };
		vertex_data.m_texture_coordinates[19] = { +1.0f, +0.0f };
		vertex_data.m_texture_coordinates[20] = { +1.0f, +0.0f };
		vertex_data.m_texture_coordinates[21] = { +1.0f, +0.0f };
		vertex_data.m_texture_coordinates[22] = { +1.0f, +0.0f };
		vertex_data.m_texture_coordinates[23] = { +1.0f, +0.0f };

		// bottom face -y
		vertex_data.m_texture_coordinates[24] = { +1.0f, +0.0f };
		vertex_data.m_texture_coordinates[25] = { +1.0f, +1.0f };
		vertex_data.m_texture_coordinates[26] = { +0.0f, +1.0f };
		vertex_data.m_texture_coordinates[27] = { +0.0f, +1.0f };
		vertex_data.m_texture_coordinates[28] = { +0.0f, +0.0f };
		vertex_data.m_texture_coordinates[29] = { +1.0f, +0.0f };

		// top face +y
		vertex_data.m_texture_coordinates[30] = { +0.0f, +1.0f };
		vertex_data.m_texture_coordinates[31] = { +0.0f, +1.0f };
		vertex_data.m_texture_coordinates[32] = { +0.0f, +1.0f };
		vertex_data.m_texture_coordinates[33] = { +0.0f, +1.0f };
		vertex_data.m_texture_coordinates[34] = { +0.0f, +1.0f };
		vertex_data.m_texture_coordinates[35] = { +0.0f, +1.0f };
	}


	vertex_data.m_normals.resize(36);
	// back face
	vertex_data.m_normals[00] = { +0.0f, +0.0f, -1.0f };
	vertex_data.m_normals[01] = { +0.0f, +0.0f, -1.0f };
	vertex_data.m_normals[02] = { +0.0f, +0.0f, -1.0f };
	vertex_data.m_normals[03] = { +0.0f, +0.0f, -1.0f };
	vertex_data.m_normals[04] = { +0.0f, +0.0f, -1.0f };
	vertex_data.m_normals[05] = { +0.0f, +0.0f, -1.0f };

	// front face
	vertex_data.m_normals[06] = { +0.0f, +0.0f, +1.0f };
	vertex_data.m_normals[07] = { +0.0f, +0.0f, +1.0f };
	vertex_data.m_normals[8] = { +0.0f, +0.0f, +1.0f };
	vertex_data.m_normals[9] = { +0.0f, +0.0f, +1.0f };
	vertex_data.m_normals[10] = { +0.0f, +0.0f, +1.0f };
	vertex_data.m_normals[11] = { +0.0f, +0.0f, +1.0f };

	// left face
	vertex_data.m_normals[12] = { -1.0f, +0.0f, +0.0f };
	vertex_data.m_normals[13] = { -1.0f, +0.0f, +0.0f };
	vertex_data.m_normals[14] = { -1.0f, +0.0f, +0.0f };
	vertex_data.m_normals[15] = { -1.0f, +0.0f, +0.0f };
	vertex_data.m_normals[16] = { -1.0f, +0.0f, +0.0f };
	vertex_data.m_normals[17] = { -1.0f, +0.0f, +0.0f };

	// right face
	vertex_data.m_normals[18] = { +1.0f, +0.0f, +0.0f };
	vertex_data.m_normals[19] = { +1.0f, +0.0f, +0.0f };
	vertex_data.m_normals[20] = { +1.0f, +0.0f, +0.0f };
	vertex_data.m_normals[21] = { +1.0f, +0.0f, +0.0f };
	vertex_data.m_normals[22] = { +1.0f, +0.0f, +0.0f };
	vertex_data.m_normals[23] = { +1.0f, +0.0f, +0.0f };

	// bottom face
	vertex_data.m_normals[24] = { +0.0f,	-1.0f, +0.0f };
	vertex_data.m_normals[25] = { +0.0f,	-1.0f, +0.0f };
	vertex_data.m_normals[26] = { +0.0f,	-1.0f, +0.0f };
	vertex_data.m_normals[27] = { +0.0f,	-1.0f, +0.0f };
	vertex_data.m_normals[28] = { +0.0f,	-1.0f, +0.0f };
	vertex_data.m_normals[29] = { +0.0f,	-1.0f, +0.0f };

	// top face
	vertex_data.m_normals[30] = { -1.0f, +1.0f, -1.0f };
	vertex_data.m_normals[31] = { -1.0f, +1.0f, -1.0f };
	vertex_data.m_normals[32] = { -1.0f, +1.0f, -1.0f };
	vertex_data.m_normals[33] = { -1.0f, +1.0f, -1.0f };
	vertex_data.m_normals[34] = { -1.0f, +1.0f, -1.0f };
	vertex_data.m_normals[35] = { -1.0f, +1.0f, -1.0f };
	return vertex_data;
}
}