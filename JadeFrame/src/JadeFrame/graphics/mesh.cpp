#include "pch.h"
#include "mesh.h"
#include "graphics_shared.h"

namespace JadeFrame {

class Mesh {
public:
    using AttributeId = u32;

    struct VertexAttribute {
        using Id = u32;
        /// human readable name of the attribute
        const char* m_name;
        /// the id of the attribute
        Id m_id;
        /// the format of the attribute
        SHADER_TYPE m_format;
    };

    std::map<AttributeId, VertexAttribute> m_attributes;
    std::vector<u32>                       m_indices;

    constexpr static VertexAttribute POSITION = {"POSITION", 0, SHADER_TYPE::V_3_F32};
    constexpr static VertexAttribute NORMAL = {"NORMAL", 1, SHADER_TYPE::V_3_F32};
    constexpr static VertexAttribute UV = {"UV", 2, SHADER_TYPE::V_2_F32};
    constexpr static VertexAttribute TANGENT = {"TANGENT", 3, SHADER_TYPE::V_4_F32};
    constexpr static VertexAttribute COLOR = {"COLOR", 4, SHADER_TYPE::V_4_F32};
};

// Mesh::Mesh(const VertexData& vertex_data) {
//	this->add_to_data(vertex_data);
// }

// auto Mesh::add_to_data(const VertexData& vertex_data) -> void {
//	m_positions.resize(vertex_data.m_positions.size());
//	m_colors.resize(vertex_data.m_colors.size());
//	m_tex_coords.resize(vertex_data.m_texture_coordinates.size());
//
//	for (size_t i = 0; i < vertex_data.m_positions.size(); i++) {
//		m_positions[i] = vertex_data.m_positions[i];
//		if (vertex_data.m_colors.size() > 0) {
//			m_colors[i] = vertex_data.m_colors[i];
//		}
//		if (vertex_data.m_texture_coordinates.size() > 0) {
//			m_tex_coords[i] = vertex_data.m_texture_coordinates[i];
//		}
//	}
//
//	m_normals.resize(vertex_data.m_normals.size());
//	for (size_t i = 0; i < vertex_data.m_normals.size(); i++) {
//		m_normals[i] = vertex_data.m_normals[i];
//	}
//
//
//	m_indices.resize(vertex_data.m_indices.size());
//	for (size_t i = 0; i < vertex_data.m_indices.size(); i++) {
//		m_indices[i] = vertex_data.m_indices[i];
//	}
//
// }

auto convert_into_data(const VertexData& vertex_data, const bool interleaved)
    -> std::vector<f32> {
    // assert(mesh.m_positions.size() == mesh.m_normals.size());
    const u64 size =
        vertex_data.m_positions.size() * 3 + vertex_data.m_colors.size() * 4 +
        vertex_data.m_texture_coordinates.size() * 2 + vertex_data.m_normals.size() * 3;

    std::vector<f32> data;
    data.reserve(size);
    if (interleaved) {
        for (size_t i = 0; i < vertex_data.m_positions.size(); i++) {
            data.push_back(vertex_data.m_positions[i].x);
            data.push_back(vertex_data.m_positions[i].y);
            data.push_back(vertex_data.m_positions[i].z);
            if (!vertex_data.m_colors.empty()) {
                data.push_back(vertex_data.m_colors[i].r);
                data.push_back(vertex_data.m_colors[i].g);
                data.push_back(vertex_data.m_colors[i].b);
                data.push_back(vertex_data.m_colors[i].a);
            }
            if (!vertex_data.m_texture_coordinates.empty()) {
                data.push_back(vertex_data.m_texture_coordinates[i].x);
                data.push_back(vertex_data.m_texture_coordinates[i].y);
            }
            if (!vertex_data.m_normals.empty()) {
                data.push_back(vertex_data.m_normals[i].x);
                data.push_back(vertex_data.m_normals[i].y);
                data.push_back(vertex_data.m_normals[i].z);
            }
        }
    } else {

        assert(false);
        for (size_t i = 0; i < vertex_data.m_positions.size(); i++) {
            data.push_back(vertex_data.m_positions[i].x);
            data.push_back(vertex_data.m_positions[i].y);
            data.push_back(vertex_data.m_positions[i].z);
        }
        for (size_t i = 0; i < vertex_data.m_colors.size(); i++) {
            data.push_back(vertex_data.m_colors[i].r);
            data.push_back(vertex_data.m_colors[i].g);
            data.push_back(vertex_data.m_colors[i].b);
            data.push_back(vertex_data.m_colors[i].a);
        }
        for (size_t i = 0; i < vertex_data.m_texture_coordinates.size(); i++) {
            data.push_back(vertex_data.m_texture_coordinates[i].x);
            data.push_back(vertex_data.m_texture_coordinates[i].y);
        }
    }

    return data;
}

auto VertexData::line(const v3& pos1, const v3& pos2) -> VertexData {
    VertexData vertex_data;
    vertex_data.m_positions.resize(2);
    vertex_data.m_positions[0] = pos1;
    vertex_data.m_positions[1] = pos2;

    vertex_data.m_texture_coordinates.resize(2);
    vertex_data.m_texture_coordinates[0] = v2::zero();
    vertex_data.m_texture_coordinates[1] = v2::zero();

    vertex_data.m_indices.reserve(2);
    vertex_data.m_indices = {0, 1};
    return vertex_data;
}

auto VertexData::rectangle(const v3& pos, const v3& size, const Desc desc) -> VertexData {
    VertexData vertex_data;
    vertex_data.m_positions.resize(6);
    vertex_data.m_positions[00] = v3::create(pos.x, pos.y, pos.z);
    vertex_data.m_positions[01] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
    vertex_data.m_positions[02] = v3::create(pos.x + size.x, pos.y, pos.z);
    vertex_data.m_positions[03] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
    vertex_data.m_positions[04] = v3::create(pos.x, pos.y, pos.z);
    vertex_data.m_positions[05] = v3::create(pos.x, pos.y + size.y, pos.z);

    if (desc.has_texture_coordinates) {
        vertex_data.m_texture_coordinates.resize(6);
        vertex_data.m_texture_coordinates[00] = v2::zero();
        vertex_data.m_texture_coordinates[01] = v2::splat(1.0F);
        vertex_data.m_texture_coordinates[02] = v2::X();
        vertex_data.m_texture_coordinates[03] = v2::splat(1.0F);
        vertex_data.m_texture_coordinates[04] = v2::zero();
        vertex_data.m_texture_coordinates[05] = v2::Y();
    }

    if (desc.has_normals) {
        vertex_data.m_normals.resize(6);
        vertex_data.m_normals[00] = v3::X();
        vertex_data.m_normals[01] = v3::X();
        vertex_data.m_normals[02] = v3::X();
        vertex_data.m_normals[03] = v3::X();
        vertex_data.m_normals[04] = v3::X();
        vertex_data.m_normals[05] = v3::X();
    }

    if (desc.has_indices) {
        vertex_data.m_indices.reserve(6);
        vertex_data.m_indices = {0, 1, 3, 1, 2, 3};
    }

    return vertex_data;
}

auto VertexData::triangle(const v3& pos1, const v3& pos2, const v3& pos3) -> VertexData {
    VertexData vertex_data;
    vertex_data.m_positions.resize(3);
    vertex_data.m_positions[0] = v3::create(pos1.x, pos1.y, pos1.z);
    vertex_data.m_positions[1] = v3::create(pos2.x, pos2.y, pos2.z);
    vertex_data.m_positions[2] = v3::create(pos3.x, pos3.y, pos3.z);

    // vertex_data.m_indices.reserve(3);
    // vertex_data.m_indices = {
    //	0, 1, 2
    // };
    return vertex_data;
}

auto VertexData::circle(const v3& position, const f32 radius, const u32 numSegments)
    -> VertexData {
    const f32 theta = 2.0F * 3.1415926F / f32(numSegments); // get the current angle
    const f32 cos = cosf(theta);                            // calculate the x component
    const f32 sin = sinf(theta);                            // calculate the y component

    VertexData vertex_data;
    vertex_data.m_positions.resize(numSegments + 1);
    vertex_data.m_positions[0] = position;
    f32 x = radius;
    f32 y = 0;
    for (u32 i = 1; i < numSegments + 1; i++) {
        vertex_data.m_positions[i] =
            v3::create(x + position.x, y + position.y, position.z); // output
                                                                    // vertex

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

auto VertexData::cube(const v3& pos, const v3& size) -> VertexData {
    VertexData vertex_data;

    vertex_data.m_positions.resize(36);
    // back face -z
    vertex_data.m_positions[00] = v3::create(pos.x, pos.y, pos.z);
    vertex_data.m_positions[01] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
    vertex_data.m_positions[02] = v3::create(pos.x + size.x, pos.y, pos.z);
    vertex_data.m_positions[03] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
    vertex_data.m_positions[04] = v3::create(pos.x, pos.y, pos.z);
    vertex_data.m_positions[05] = v3::create(pos.x, pos.y + size.y, pos.z);

    // front face +z
    vertex_data.m_positions[06] = v3::create(pos.x, pos.y, pos.z + size.z);
    vertex_data.m_positions[07] = v3::create(pos.x + size.x, pos.y, pos.z + size.z);
    vertex_data.m_positions[8l] =
        v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    vertex_data.m_positions[9l] =
        v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    vertex_data.m_positions[10] = v3::create(pos.x, pos.y + size.y, pos.z + size.z);
    vertex_data.m_positions[11] = v3::create(pos.x, pos.y, pos.z + size.z);

    // left face -x
    vertex_data.m_positions[12] = v3::create(pos.x, pos.y + size.y, pos.z + size.z);
    vertex_data.m_positions[13] = v3::create(pos.x, pos.y + size.y, pos.z);
    vertex_data.m_positions[14] = v3::create(pos.x, pos.y, pos.z);
    vertex_data.m_positions[15] = v3::create(pos.x, pos.y, pos.z);
    vertex_data.m_positions[16] = v3::create(pos.x, pos.y, pos.z + size.z);
    vertex_data.m_positions[17] = v3::create(pos.x, pos.y + size.y, pos.z + size.z);

    // right face +x
    vertex_data.m_positions[18] =
        v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    vertex_data.m_positions[19] = v3::create(pos.x + size.x, pos.y, pos.z);
    vertex_data.m_positions[20] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
    vertex_data.m_positions[21] = v3::create(pos.x + size.x, pos.y, pos.z);
    vertex_data.m_positions[22] =
        v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    vertex_data.m_positions[23] = v3::create(pos.x + size.x, pos.y, pos.z + size.z);

    // bottom face
    vertex_data.m_positions[24] = v3::create(pos.x, pos.y, pos.z);
    vertex_data.m_positions[25] = v3::create(pos.x + size.x, pos.y, pos.z);
    vertex_data.m_positions[26] = v3::create(pos.x + size.x, pos.y, pos.z + size.z);
    vertex_data.m_positions[27] = v3::create(pos.x + size.x, pos.y, pos.z + size.z);
    vertex_data.m_positions[28] = v3::create(pos.x, pos.y, pos.z + size.z);
    vertex_data.m_positions[29] = v3::create(pos.x, pos.y, pos.z);

    // top face
    vertex_data.m_positions[30] = v3::create(pos.x, pos.y + size.y, pos.z);
    vertex_data.m_positions[31] =
        v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    vertex_data.m_positions[32] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
    vertex_data.m_positions[33] =
        v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    vertex_data.m_positions[34] = v3::create(pos.x, pos.y + size.y, pos.z);
    vertex_data.m_positions[35] = v3::create(pos.x, pos.y + size.y, pos.z + size.z);

    vertex_data.m_texture_coordinates.resize(36);
    {
        // back face -z
        vertex_data.m_texture_coordinates[00] = v2::X();
        vertex_data.m_texture_coordinates[01] = v2::splat(1.0F);
        vertex_data.m_texture_coordinates[02] = v2::Y();
        vertex_data.m_texture_coordinates[03] = v2::Y();
        vertex_data.m_texture_coordinates[04] = v2::zero();
        vertex_data.m_texture_coordinates[05] = v2::X();

        // front face +z
        vertex_data.m_texture_coordinates[06] = v2::zero();
        vertex_data.m_texture_coordinates[07] = v2::zero();
        vertex_data.m_texture_coordinates[+8] = v2::zero();
        vertex_data.m_texture_coordinates[+9] = v2::zero();
        vertex_data.m_texture_coordinates[10] = v2::zero();
        vertex_data.m_texture_coordinates[11] = v2::zero();

        // left face
        vertex_data.m_texture_coordinates[12] = v2::NEG_X();
        vertex_data.m_texture_coordinates[13] = v2::NEG_X();
        vertex_data.m_texture_coordinates[14] = v2::NEG_X();
        vertex_data.m_texture_coordinates[15] = v2::NEG_X();
        vertex_data.m_texture_coordinates[16] = v2::NEG_X();
        vertex_data.m_texture_coordinates[17] = v2::NEG_X();

        // right face +x
        vertex_data.m_texture_coordinates[18] = v2::X();
        vertex_data.m_texture_coordinates[19] = v2::X();
        vertex_data.m_texture_coordinates[20] = v2::X();
        vertex_data.m_texture_coordinates[21] = v2::X();
        vertex_data.m_texture_coordinates[22] = v2::X();
        vertex_data.m_texture_coordinates[23] = v2::X();

        // bottom face -y
        vertex_data.m_texture_coordinates[24] = v2::X();
        vertex_data.m_texture_coordinates[25] = v2::splat(1.0F);
        vertex_data.m_texture_coordinates[26] = v2::Y();
        vertex_data.m_texture_coordinates[27] = v2::Y();
        vertex_data.m_texture_coordinates[28] = v2::zero();
        vertex_data.m_texture_coordinates[29] = v2::X();

        // top face +y
        vertex_data.m_texture_coordinates[30] = v2::Y();
        vertex_data.m_texture_coordinates[31] = v2::Y();
        vertex_data.m_texture_coordinates[32] = v2::Y();
        vertex_data.m_texture_coordinates[33] = v2::Y();
        vertex_data.m_texture_coordinates[34] = v2::Y();
        vertex_data.m_texture_coordinates[35] = v2::Y();
    }

    vertex_data.m_normals.resize(36);
    for (size_t i = 0; i < vertex_data.m_normals.size(); i++) {
        std::array<v3, 6> s;
        s[0] = v3::create(+0.0f, +0.0f, -1.0f); // back
        s[1] = v3::create(+0.0f, +0.0f, +1.0f); // front
        s[2] = v3::create(-1.0f, +0.0f, +0.0f); // left
        s[3] = v3::create(+1.0f, +0.0f, +0.0f); // right
        s[4] = v3::create(+0.0f, -1.0f, +0.0f); // bottom
        s[5] = v3::create(+0.0f, +1.0f, +0.0f); // up

        vertex_data.m_normals[i] = s[i / 6];
    };
    return vertex_data;
}
} // namespace JadeFrame