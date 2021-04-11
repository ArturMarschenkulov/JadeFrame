#include "mesh.h"


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
	vertex_data.tex_coords[00] = { +1.0f, +0.0f };
	vertex_data.tex_coords[01] = { +1.0f, +1.0f };
	vertex_data.tex_coords[02] = { +0.0f, +1.0f };
	vertex_data.tex_coords[03] = { +0.0f, +1.0f };
	vertex_data.tex_coords[04] = { +0.0f, +0.0f };
	vertex_data.tex_coords[05] = { +1.0f, +0.0f };

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

auto VertexDataFactory::make_circle(const Vec3& position, const float radius, const int numSegments) -> VertexData {
	const float theta = 2.0f * 3.1415926f / float(numSegments);//get the current angle 
	const float cos = cosf(theta);//calculate the x component 
	const float sin = sinf(theta);//calculate the y component 


	VertexData vertex_data;
	vertex_data.positions.resize(numSegments + 1);
	vertex_data.positions[0] = position;
	float x = radius;
	float y = 0;
	for (int i = 1; i < numSegments + 1; i++) {
		vertex_data.positions[i] = Vec3{ x + position.x, y + position.y, position.z };//output vertex 

		const float t = x;
		x = cos * x - sin * y;
		y = sin * t + cos * y;
	}

	const uint32_t num_index = (numSegments * 3);
	vertex_data.indices.resize(num_index);

	for (int i = 0; i < numSegments; i++) {
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

//namespace A {
//struct AABB {
//	AABB() = default;
//	AABB(const Vec3& minimum, const Vec3& maximum);
//	Vec3 base;
//	Vec3 offset;
//
//	Vec3 center() const;
//	AABB transform(const Mat4& mat) const;
//	Vec3 corner(unsigned corner) const;
//};
//
//struct BoundingSphere {
//	BoundingSphere() = default;
//	explicit BoundingSphere(const AABB& aabb);
//
//	Vec4 pos_radius;
//};
//
//struct Frustum {
//	Frustum() = default;
//	Frustum(const Mat4& view_proj);
//	Vec4 planes[6];
//
//	// Used for frustum culling.
//	bool intersects_with_sphere(const BoundingSphere& sphere) const;
//};
//
//AABB::AABB(const Vec3& minimum, const Vec3& maximum)    {
//	base = minimum;
//	offset = maximum - minimum;
//}
//
//Vec3 AABB::center() const {
//	return base + Vec3(0.5f) * offset;
//}
//
//// Assumes we're not using projective geometry ... It gets troublesome to handle flipped-sign W.
//// Transform all corners of the AABB then create a new AABB based on the transformed result.
//AABB AABB::transform(const Mat4& mat) const    {
//
//	Vec3 corners[8];
//	for (unsigned i = 0; i < 8; i++)       {
//		Vec3 c = corner(i);
//		Vec4 c_trans = mat * Vec4(c, 1.0f);
//		corners[i] = Vec3(c_trans) / Vec3(c_trans.w);
//	}
//
//	Vec3 minimum = corners[0];
//	Vec3 maximum = minimum;
//	for (auto& v : corners)       {
//		minimum = std::min(minimum, v);
//		maximum = std::max(maximum, v);
//	}
//
//	AABB aabb;
//	aabb.base = minimum;
//	aabb.offset = maximum - minimum;
//	return aabb;
//}
//
//Vec3 AABB::corner(unsigned corner) const    {
//	Vec3 b = base;
//	if (corner & 4)
//		b.z += offset.z;
//	if (corner & 2)
//		b.y += offset.y;
//	if (corner & 1)
//		b.x += offset.x;
//
//	return b;
//}
//
//BoundingSphere::BoundingSphere(const AABB& aabb)    {
//	pos_radius = Vec4(aabb.center(), length(aabb.offset * Vec3(0.5f)));
//}
//
//Frustum::Frustum(const Mat4& view_proj)    {
//	auto inv_view_proj = inverse(view_proj);
//
//	// Get world-space coordinates for clip-space bounds.
//	auto lbn = inv_view_proj * Vec4(-1, -1, -1, 1);
//	auto ltn = inv_view_proj * Vec4(-1, 1, -1, 1);
//	auto lbf = inv_view_proj * Vec4(-1, -1, 1, 1);
//	auto rbn = inv_view_proj * Vec4(1, -1, -1, 1);
//	auto rtn = inv_view_proj * Vec4(1, 1, -1, 1);
//	auto rbf = inv_view_proj * Vec4(1, -1, 1, 1);
//	auto rtf = inv_view_proj * Vec4(1, 1, 1, 1);
//
//	auto lbn_pos = lbn.xyz / lbn.www;
//	auto ltn_pos = ltn.xyz / ltn.www;
//	auto lbf_pos = lbf.xyz / lbf.www;
//	auto rbn_pos = rbn.xyz / rbn.www;
//	auto rtn_pos = rtn.xyz / rtn.www;
//	auto rbf_pos = rbf.xyz / rbf.www;
//	auto rtf_pos = rtf.xyz / rtf.www;
//
//	// Get plane equations for all sides of frustum.
//	auto left_normal = normalize(cross(lbf_pos - lbn_pos, ltn_pos - lbn_pos));
//	auto right_normal = normalize(cross(rtn_pos - rbn_pos, rbf_pos - rbn_pos));
//	auto top_normal = normalize(cross(ltn_pos - rtn_pos, rtf_pos - rtn_pos));
//	auto bottom_normal = normalize(cross(rbf_pos - rbn_pos, lbn_pos - rbn_pos));
//	auto near_normal = normalize(cross(ltn_pos - lbn_pos, rbn_pos - lbn_pos));
//	auto far_normal = normalize(cross(rtf_pos - rbf_pos, lbf_pos - rbf_pos));
//
//	planes[0] = Vec4(near_normal, -dot(near_normal, lbn_pos)); // Near
//	planes[1] = Vec4(far_normal, -dot(far_normal, lbf_pos)); // Far
//	planes[2] = Vec4(left_normal, -dot(left_normal, lbn_pos)); // Left
//	planes[3] = Vec4(right_normal, -dot(right_normal, rbn_pos)); // Right
//	planes[4] = Vec4(top_normal, -dot(top_normal, ltn_pos)); // Top
//	planes[5] = Vec4(bottom_normal, -dot(bottom_normal, lbn_pos)); // Top
//}
//
//bool Frustum::intersects_with_sphere(const BoundingSphere& sphere) const    {
//	Vec4 pos = Vec4(sphere.pos_radius.xyz, 1.0f);
//	for (auto& plane : planes)
//		if (dot(pos, plane) < -sphere.pos_radius.w)
//			return false;
//	return true;
//}
//}