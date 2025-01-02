#include "JadeFrame/math/vec.h"
#include "JadeFrame/utils/assert.h"
#include "JadeFrame/utils/logger.h"
#include "pch.h"
#include "mesh.h"
#include "graphics_shared.h"

namespace JadeFrame {

enum class PRIMITIVE_TOPOLOGY {
    POINT_LIST,
    LINE_LIST,
    LINE_STRIP,
    TRIANGLE_LIST,
    TRIANGLE_STRIP,
};

static auto is_strip(PRIMITIVE_TOPOLOGY topology) -> bool {
    return topology == PRIMITIVE_TOPOLOGY::LINE_STRIP ||
           topology == PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP;
}

static auto to_list(std::vector<v2> v2s) -> std::vector<f32> {
    std::vector<f32> result;
    result.reserve(v2s.size() * 2);
    for (const v2& v : v2s) {
        result.push_back(v.x);
        result.push_back(v.y);
    }
    return result;
}

static auto to_list(std::vector<v4> v2s) -> std::vector<f32> {
    std::vector<f32> result;
    result.reserve(v2s.size() * 4);
    for (const v4& v : v2s) {
        result.push_back(v.x);
        result.push_back(v.y);
        result.push_back(v.z);
        result.push_back(v.w);
    }
    return result;
}

static auto to_list(std::vector<v3> v3s) -> std::vector<f32> {
    std::vector<f32> result;
    result.reserve(v3s.size() * 3);
    for (const v3& v : v3s) {
        result.push_back(v.x);
        result.push_back(v.y);
        result.push_back(v.z);
    }
    return result;
}

static auto to_list(v4 v4) -> std::vector<f32> { return {v4.x, v4.y, v4.z, v4.w}; }

static auto to_list(v3 v3) -> std::vector<f32> { return {v3.x, v3.y, v3.z}; }

static auto to_list(v2 v2) -> std::vector<f32> { return {v2.x, v2.y}; }

auto Mesh::rectangle(const v3& pos, const v3& size, const Desc desc) -> Mesh {
    Mesh            mesh;
    std::vector<v3> positions;
    if (desc.has_position) {
        positions.resize(6);
        positions[0] = v3::create(pos.x, pos.y, pos.z);
        positions[1] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
        positions[2] = v3::create(pos.x + size.x, pos.y, pos.z);
        positions[3] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
        positions[4] = v3::create(pos.x, pos.y, pos.z);
        positions[5] = v3::create(pos.x, pos.y + size.y, pos.z);
        std::vector<f32> data = to_list(positions);
        mesh.m_attributes[Mesh::POSITION.m_id] = AttributeData{Mesh::POSITION, data};
    }
    std::vector<v2> texture_coordinates;
    if (desc.has_texture_coordinates) {
        texture_coordinates.resize(6);
        texture_coordinates[0] = v2::zero();
        texture_coordinates[1] = v2::splat(1.0F);
        texture_coordinates[2] = v2::X();
        texture_coordinates[3] = v2::splat(1.0F);
        texture_coordinates[4] = v2::zero();
        texture_coordinates[5] = v2::Y();
        std::vector<f32> data = to_list(texture_coordinates);
        mesh.m_attributes[Mesh::UV.m_id] = AttributeData{Mesh::UV, data};
    }

    std::vector<v3> normals;
    if (desc.has_normals) {
        normals.resize(6);
        normals[0] = v3::X();
        normals[1] = v3::X();
        normals[2] = v3::X();
        normals[3] = v3::X();
        normals[4] = v3::X();
        normals[5] = v3::X();
        std::vector<f32> data = to_list(normals);
        mesh.m_attributes[Mesh::NORMAL.m_id] = AttributeData{Mesh::NORMAL, data};
    }
    if (desc.has_indices) {
        std::vector<u32> indices = {0, 1, 3, 1, 2, 3};
        mesh.m_indices = indices;
    }
    return mesh;
}

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

struct MeshVertex {

    std::map<Mesh::VertexAttributeId, Mesh::AttributeData> m_attributes;
};

auto convert_into_data(const Mesh& mesh, const bool interleaved) -> std::vector<f32> {
    if (interleaved) {
        u32 combined_attribute_count = 0;
        for (const auto& [id, data] : mesh.m_attributes) {
            Logger::err("It has attrib {}", data.m_attribute_id.m_name);
            u32 amount =
                data.m_data.size() / component_count(data.m_attribute_id.m_format);
            Logger::err("- amount: {}", amount);
            u32 size = component_count(data.m_attribute_id.m_format);
            Logger::err("- size: {}", size);
            combined_attribute_count += amount * size;
            Logger::err("- combined_attribute_count: {}", amount * size);
        }
        Logger::err("combined_attribute_count: {}", combined_attribute_count);
        u32 position_count = mesh.m_attributes.at(Mesh::POSITION.m_id).m_data.size() /
                             component_count(Mesh::POSITION.m_format);

        std::vector<MeshVertex> vertices;
        vertices.resize(position_count);
        for (const auto& [id, data] : mesh.m_attributes) {
            u32 amount =
                data.m_data.size() / component_count(data.m_attribute_id.m_format);
            u32 size = component_count(data.m_attribute_id.m_format);
            for (u32 i = 0; i < amount; i++) {
                auto& vertex = vertices[i];
                vertex.m_attributes[id].m_attribute_id = data.m_attribute_id;

                for (u32 j = 0; j < size; j++) {
                    vertex.m_attributes[id].m_data.push_back(data.m_data[i * size + j]);
                }
            }
        }
        std::vector<f32> data;
        data.reserve(combined_attribute_count);
        auto vertex_count = vertices.size();
        for (u32 i = 0; i < vertex_count; i++) {
            auto& vertex = vertices[i];
            for (const auto& [id, attribute_data] : vertex.m_attributes) {
                for (f32 f : attribute_data.m_data) { data.push_back(f); }
            }
        }
        return data;
    } else {
        JF_UNIMPLEMENTED("Non interleaved data is not implemented yet.");
    }
}

auto convert_into_data(const VertexData& vertex_data, const bool interleaved)
    -> std::vector<f32> {
    // assert(mesh.m_positions.size() == mesh.m_normals.size());
    const u64 size =
        vertex_data.m_positions.size() * 3 + vertex_data.m_colors.size() * 4 +
        vertex_data.m_texture_coordinates.size() * 2 + vertex_data.m_normals.size() * 3;

    std::vector<f32> data;
    data.reserve(size);
    if (interleaved) {
        u32 vertex_count = vertex_data.m_positions.size();
        for (size_t i = 0; i < vertex_count; i++) {
            auto temp = to_list(vertex_data.m_positions[i]);
            for (f32 f : temp) { data.push_back(f); }
            if (!vertex_data.m_colors.empty()) {
                RGBAColor        color = vertex_data.m_colors[i];
                v4               v = {color.r, color.g, color.b, color.a};
                std::vector<f32> temp = to_list(v);
                for (f32 f : temp) { data.push_back(f); }
            }
            if (!vertex_data.m_texture_coordinates.empty()) {
                std::vector<f32> temp = to_list(vertex_data.m_texture_coordinates[i]);
                for (f32 f : temp) { data.push_back(f); }
            }
            if (!vertex_data.m_normals.empty()) {
                std::vector<f32> temp = to_list(vertex_data.m_normals[i]);
                for (f32 f : temp) { data.push_back(f); }
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