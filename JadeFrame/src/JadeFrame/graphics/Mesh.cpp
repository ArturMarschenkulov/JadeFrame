#include "mesh.h"


Mesh::Mesh(const VertexData& vertex_data) {
	this->add_to_data(vertex_data);
}

auto Mesh::add_to_data(const VertexData& vertex_data) -> void {

	m_positions.resize(vertex_data.positions.size());
	m_colors.resize(vertex_data.positions.size());
	m_tex_coords.resize(vertex_data.tex_coords.size());

	for (size_t i = 0; i < vertex_data.positions.size(); i++) {
		m_positions[i] = vertex_data.positions[i];
		m_colors[i] = current_color;
		m_tex_coords[i] = vertex_data.tex_coords[i];
	}

	m_normals.resize(vertex_data.normals.size());
	for (size_t i = 0; i < vertex_data.normals.size(); i++) {
		m_normals[i] = vertex_data.normals[i];
	}


	m_indices.resize(vertex_data.indices.size());
	for (size_t i = 0; i < vertex_data.indices.size(); i++) {
		m_indices[i] = vertex_data.indices[i];
	}

}

auto Mesh::set_color(const Color color) -> void {
	if (current_color != color) {
		current_color = color;
		for (size_t i = 0; i < m_positions.size(); i++) {
			m_colors[i] = current_color;
		}
	}
}

auto VertexDataFactory::make_line(const Vec3& pos1, const Vec3& pos2) -> VertexData {
	VertexData vertex_data;
	vertex_data.positions.resize(2);
	vertex_data.positions[0] = pos1;
	vertex_data.positions[1] = pos2;

	vertex_data.tex_coords.resize(2);
	vertex_data.tex_coords[0] = { 0.0f, 0.0f };
	vertex_data.tex_coords[1] = { 0.0f, 0.0f };

	vertex_data.indices.reserve(2);
	vertex_data.indices = {
		0, 1
	};
	return vertex_data;
}

auto VertexDataFactory::make_rectangle(const Vec3& pos, const Vec3& size) -> VertexData {
	VertexData vertex_data;
	vertex_data.positions.resize(6);
	vertex_data.positions[00] = { pos.x			, pos.y			, pos.z };
	vertex_data.positions[01] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.positions[02] = { pos.x + size.x, pos.y			, pos.z };
	vertex_data.positions[03] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.positions[04] = { pos.x			, pos.y			, pos.z };
	vertex_data.positions[05] = { pos.x			, pos.y + size.y, pos.z };

	vertex_data.tex_coords.resize(6);
	vertex_data.tex_coords[00] = { +0.0f, +0.0f };
	vertex_data.tex_coords[01] = { +1.0f, +1.0f };
	vertex_data.tex_coords[02] = { +1.0f, +0.0f };
	vertex_data.tex_coords[03] = { +1.0f, +1.0f };
	vertex_data.tex_coords[04] = { +0.0f, +0.0f };
	vertex_data.tex_coords[05] = { +0.0f, +1.0f };

	vertex_data.normals.resize(6);
	vertex_data.normals[00] = { +0.0f, +0.0f, +1.0f };
	vertex_data.normals[01] = { +0.0f, +0.0f, +1.0f };
	vertex_data.normals[02] = { +0.0f, +0.0f, +1.0f };
	vertex_data.normals[03] = { +0.0f, +0.0f, +1.0f };
	vertex_data.normals[04] = { +0.0f, +0.0f, +1.0f };
	vertex_data.normals[05] = { +0.0f, +0.0f, +1.0f };
	return vertex_data;


	//VertexData vertex_data;
	//vertex_data.positions.resize(4);
	//vertex_data.positions[0] = Vec3{ pos.x, pos.y, pos.z };
	//vertex_data.positions[1] = Vec3{ pos.x + size.x, pos.y, pos.z };
	//vertex_data.positions[2] = Vec3{ pos.x + size.x, pos.y + size.y, pos.z };
	//vertex_data.positions[3] = Vec3{ pos.x, pos.y + size.y, pos.z };

	//vertex_data.tex_coords.resize(4);
	//vertex_data.tex_coords[0] = { 1.0, 1.0 };
	//vertex_data.tex_coords[1] = { 1.0, 0.0 };
	//vertex_data.tex_coords[2] = { 0.0, 0.0 };
	//vertex_data.tex_coords[3] = { 0.0, 1.0 };


	//vertex_data.indices.reserve(6);
	//vertex_data.indices = {
	//	0, 1, 3,
	//	1, 2, 3
	//};
	//return vertex_data;
}

auto VertexDataFactory::make_triangle(const Vec3& pos1, const Vec3& pos2, const Vec3& pos3) -> VertexData {
	VertexData vertex_data;
	vertex_data.positions.resize(3);
	vertex_data.positions[0] = Vec3{ pos1.x, pos1.y, pos1.z };
	vertex_data.positions[1] = Vec3{ pos2.x, pos2.y, pos2.z };
	vertex_data.positions[2] = Vec3{ pos3.x, pos3.y, pos3.z };

	vertex_data.indices.reserve(3);
	vertex_data.indices = {
		0, 1, 2
	};
	return vertex_data;
}

auto VertexDataFactory::make_circle(const Vec3& position, const f32 radius, const u32 numSegments) -> VertexData {
	const f32 theta = 2.0f * 3.1415926f / f32(numSegments);//get the current angle 
	const f32 cos = cosf(theta);//calculate the x component 
	const f32 sin = sinf(theta);//calculate the y component 


	VertexData vertex_data;
	vertex_data.positions.resize(numSegments + 1);
	vertex_data.positions[0] = position;
	f32 x = radius;
	f32 y = 0;
	for (u32 i = 1; i < numSegments + 1; i++) {
		vertex_data.positions[i] = Vec3{ x + position.x, y + position.y, position.z };//output vertex 

		const f32 t = x;
		x = cos * x - sin * y;
		y = sin * t + cos * y;
	}

	const u32 num_index = (numSegments * 3);
	vertex_data.indices.resize(num_index);

	for (u32 i = 0; i < numSegments; i++) {
		vertex_data.indices[(3 * i + 0)] = 0;
		vertex_data.indices[(3 * i + 1)] = 1 + i;
		vertex_data.indices[(3 * i + 2)] = 2 + i;
	}
	vertex_data.indices[num_index - 1] = vertex_data.indices[1];

	return vertex_data;
}

auto VertexDataFactory::make_cube(const Vec3& pos, const Vec3& size) -> VertexData {
	VertexData vertex_data;

	vertex_data.positions.resize(36);
	// back face -z
	vertex_data.positions[00] = { pos.x			, pos.y			, pos.z };
	vertex_data.positions[01] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.positions[02] = { pos.x + size.x, pos.y			, pos.z };
	vertex_data.positions[03] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.positions[04] = { pos.x			, pos.y			, pos.z };
	vertex_data.positions[05] = { pos.x			, pos.y + size.y, pos.z };

	// front face +z
	vertex_data.positions[06] = { pos.x			, pos.y			, pos.z + size.z };
	vertex_data.positions[07] = { pos.x + size.x, pos.y			, pos.z + size.z };
	vertex_data.positions[8l] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.positions[9l] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.positions[10] = { pos.x			, pos.y + size.y, pos.z + size.z };
	vertex_data.positions[11] = { pos.x			, pos.y			, pos.z + size.z };

	// left face -x
	vertex_data.positions[12] = { pos.x			, pos.y + size.y, pos.z + size.z };
	vertex_data.positions[13] = { pos.x			, pos.y + size.y, pos.z };
	vertex_data.positions[14] = { pos.x			, pos.y			, pos.z };
	vertex_data.positions[15] = { pos.x			, pos.y			, pos.z };
	vertex_data.positions[16] = { pos.x			, pos.y			, pos.z + size.z };
	vertex_data.positions[17] = { pos.x			, pos.y + size.y, pos.z + size.z };

	// right face +x
	vertex_data.positions[18] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.positions[19] = { pos.x + size.x, pos.y			, pos.z };
	vertex_data.positions[20] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.positions[21] = { pos.x + size.x, pos.y			, pos.z };
	vertex_data.positions[22] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.positions[23] = { pos.x + size.x, pos.y			, pos.z + size.z };

	// bottom face
	vertex_data.positions[24] = { pos.x			, pos.y			, pos.z };
	vertex_data.positions[25] = { pos.x + size.x, pos.y			, pos.z };
	vertex_data.positions[26] = { pos.x + size.x, pos.y			, pos.z + size.z };
	vertex_data.positions[27] = { pos.x + size.x, pos.y			, pos.z + size.z };
	vertex_data.positions[28] = { pos.x			, pos.y			, pos.z + size.z };
	vertex_data.positions[29] = { pos.x			, pos.y			, pos.z };

	// top face
	vertex_data.positions[30] = { pos.x			, pos.y + size.y, pos.z };
	vertex_data.positions[31] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.positions[32] = { pos.x + size.x, pos.y + size.y, pos.z };
	vertex_data.positions[33] = { pos.x + size.x, pos.y + size.y, pos.z + size.z };
	vertex_data.positions[34] = { pos.x			, pos.y + size.y, pos.z };
	vertex_data.positions[35] = { pos.x			, pos.y + size.y, pos.z + size.z };



	vertex_data.tex_coords.resize(36);
	{
		// back face -z
		vertex_data.tex_coords[00] = { +1.0f, +0.0f };
		vertex_data.tex_coords[01] = { +1.0f, +1.0f };
		vertex_data.tex_coords[02] = { +0.0f, +1.0f };
		vertex_data.tex_coords[03] = { +0.0f, +1.0f };
		vertex_data.tex_coords[04] = { +0.0f, +0.0f };
		vertex_data.tex_coords[05] = { +1.0f, +0.0f };

		// front face +z
		vertex_data.tex_coords[06] = { +0.0f, +0.0f };
		vertex_data.tex_coords[07] = { +0.0f, +0.0f };
		vertex_data.tex_coords[8] = { +0.0f, +0.0f };
		vertex_data.tex_coords[9] = { +0.0f, +0.0f };
		vertex_data.tex_coords[10] = { +0.0f, +0.0f };
		vertex_data.tex_coords[11] = { +0.0f, +0.0f };

		// left face
		vertex_data.tex_coords[12] = { -1.0f, +0.0f };
		vertex_data.tex_coords[13] = { -1.0f, +0.0f };
		vertex_data.tex_coords[14] = { -1.0f, +0.0f };
		vertex_data.tex_coords[15] = { -1.0f, +0.0f };
		vertex_data.tex_coords[16] = { -1.0f, +0.0f };
		vertex_data.tex_coords[17] = { -1.0f, +0.0f };

		// right face +x
		vertex_data.tex_coords[18] = { +1.0f, +0.0f };
		vertex_data.tex_coords[19] = { +1.0f, +0.0f };
		vertex_data.tex_coords[20] = { +1.0f, +0.0f };
		vertex_data.tex_coords[21] = { +1.0f, +0.0f };
		vertex_data.tex_coords[22] = { +1.0f, +0.0f };
		vertex_data.tex_coords[23] = { +1.0f, +0.0f };

		// bottom face -y
		vertex_data.tex_coords[24] = { +1.0f, +0.0f };
		vertex_data.tex_coords[25] = { +1.0f, +1.0f };
		vertex_data.tex_coords[26] = { +0.0f, +1.0f };
		vertex_data.tex_coords[27] = { +0.0f, +1.0f };
		vertex_data.tex_coords[28] = { +0.0f, +0.0f };
		vertex_data.tex_coords[29] = { +1.0f, +0.0f };

		// top face +y
		vertex_data.tex_coords[30] = { +0.0f, +1.0f };
		vertex_data.tex_coords[31] = { +0.0f, +1.0f };
		vertex_data.tex_coords[32] = { +0.0f, +1.0f };
		vertex_data.tex_coords[33] = { +0.0f, +1.0f };
		vertex_data.tex_coords[34] = { +0.0f, +1.0f };
		vertex_data.tex_coords[35] = { +0.0f, +1.0f };
	}


	vertex_data.normals.resize(36);
	// back face
	vertex_data.normals[00] = { +0.0f, +0.0f, -1.0f };
	vertex_data.normals[01] = { +0.0f, +0.0f, -1.0f };
	vertex_data.normals[02] = { +0.0f, +0.0f, -1.0f };
	vertex_data.normals[03] = { +0.0f, +0.0f, -1.0f };
	vertex_data.normals[04] = { +0.0f, +0.0f, -1.0f };
	vertex_data.normals[05] = { +0.0f, +0.0f, -1.0f };

	// front face
	vertex_data.normals[06] = { +0.0f, +0.0f, +1.0f };
	vertex_data.normals[07] = { +0.0f, +0.0f, +1.0f };
	vertex_data.normals[8] = { +0.0f, +0.0f, +1.0f };
	vertex_data.normals[9] = { +0.0f, +0.0f, +1.0f };
	vertex_data.normals[10] = { +0.0f, +0.0f, +1.0f };
	vertex_data.normals[11] = { +0.0f, +0.0f, +1.0f };

	// left face
	vertex_data.normals[12] = { -1.0f, +0.0f, +0.0f };
	vertex_data.normals[13] = { -1.0f, +0.0f, +0.0f };
	vertex_data.normals[14] = { -1.0f, +0.0f, +0.0f };
	vertex_data.normals[15] = { -1.0f, +0.0f, +0.0f };
	vertex_data.normals[16] = { -1.0f, +0.0f, +0.0f };
	vertex_data.normals[17] = { -1.0f, +0.0f, +0.0f };

	// right face
	vertex_data.normals[18] = { +1.0f, +0.0f, +0.0f };
	vertex_data.normals[19] = { +1.0f, +0.0f, +0.0f };
	vertex_data.normals[20] = { +1.0f, +0.0f, +0.0f };
	vertex_data.normals[21] = { +1.0f, +0.0f, +0.0f };
	vertex_data.normals[22] = { +1.0f, +0.0f, +0.0f };
	vertex_data.normals[23] = { +1.0f, +0.0f, +0.0f };

	// bottom face
	vertex_data.normals[24] = { +0.0f,	-1.0f, +0.0f };
	vertex_data.normals[25] = { +0.0f,	-1.0f, +0.0f };
	vertex_data.normals[26] = { +0.0f,	-1.0f, +0.0f };
	vertex_data.normals[27] = { +0.0f,	-1.0f, +0.0f };
	vertex_data.normals[28] = { +0.0f,	-1.0f, +0.0f };
	vertex_data.normals[29] = { +0.0f,	-1.0f, +0.0f };

	// top face
	vertex_data.normals[30] = { -1.0f, +1.0f, -1.0f };
	vertex_data.normals[31] = { -1.0f, +1.0f, -1.0f };
	vertex_data.normals[32] = { -1.0f, +1.0f, -1.0f };
	vertex_data.normals[33] = { -1.0f, +1.0f, -1.0f };
	vertex_data.normals[34] = { -1.0f, +1.0f, -1.0f };
	vertex_data.normals[35] = { -1.0f, +1.0f, -1.0f };
	return vertex_data;
}