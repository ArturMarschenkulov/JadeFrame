#include "JadeFrame/math/vec.h"
#include "JadeFrame/utils/assert.h"
#include "JadeFrame/utils/logger.h"
#include "mesh.h"

#include <numbers>
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

auto MeshBuilder::rectangle_(const v3& pos, const v3& size, const Desc desc) -> Mesh {
    Mesh mesh;
    if (desc.has_indices) {
        std::vector<v3> positions;
        if (desc.has_position) {
            positions.resize(4);
            positions[0] = v3::create(pos.x, pos.y, pos.z);
            positions[1] = v3::create(pos.x + size.x, pos.y, pos.z);
            positions[2] = v3::create(pos.x, pos.y + size.y, pos.z);
            positions[3] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
            std::vector<f32> data = to_list(positions);
            mesh.m_attributes[Mesh::POSITION.m_id] =
                Mesh::AttributeData{Mesh::POSITION, data};
        }

        u32 num_vertices = positions.size();

        std::vector<v2> texture_coordinates;
        if (desc.has_texture_coordinates) {
            texture_coordinates.resize(num_vertices);
            texture_coordinates[0] = v2::zero();
            texture_coordinates[1] = v2::X();
            texture_coordinates[2] = v2::Y();
            texture_coordinates[3] = v2::splat(1.0F);
            std::vector<f32> data = to_list(texture_coordinates);
            mesh.m_attributes[Mesh::UV.m_id] = Mesh::AttributeData{Mesh::UV, data};
        }

        std::vector<v3> normals;
        if (desc.has_normals) {
            v3 normal_up = v3::X();
            normals.resize(num_vertices);
            normals[0] = normal_up;
            normals[1] = normal_up;
            normals[2] = normal_up;
            normals[3] = normal_up;
            std::vector<f32> data = to_list(normals);
            mesh.m_attributes[Mesh::NORMAL.m_id] =
                Mesh::AttributeData{Mesh::NORMAL, data};
        }

        std::vector<u32> indices = {0, 3, 1, 3, 0, 2};
        mesh.m_indices = indices;

    } else {
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
            mesh.m_attributes[Mesh::POSITION.m_id] =
                Mesh::AttributeData{Mesh::POSITION, data};
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
            mesh.m_attributes[Mesh::UV.m_id] = Mesh::AttributeData{Mesh::UV, data};
        }

        std::vector<v3> normals;
        if (desc.has_normals) {
            v3 normal_up = v3::X();
            normals.resize(6);
            normals[0] = normal_up;
            normals[1] = normal_up;
            normals[2] = normal_up;
            normals[3] = normal_up;
            normals[4] = normal_up;
            normals[5] = normal_up;
            std::vector<f32> data = to_list(normals);
            mesh.m_attributes[Mesh::NORMAL.m_id] =
                Mesh::AttributeData{Mesh::NORMAL, data};
        }
    }

    return mesh;
}

auto MeshBuilder::rectangle(const v3& pos, const v3& size, const Desc desc) -> Mesh {
    Mesh mesh;
    if (desc.has_indices) {
        std::vector<v3> positions;
        if (desc.has_position) {
            positions.resize(4);
            positions[0] = v3::create(pos.x, pos.y, pos.z);
            positions[1] = v3::create(pos.x + size.x, pos.y, pos.z);
            positions[2] = v3::create(pos.x, pos.y + size.y, pos.z);
            positions[3] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
            std::vector<f32> data = to_list(positions);
            mesh.m_attributes[Mesh::POSITION.m_id] =
                Mesh::AttributeData{Mesh::POSITION, data};
        }

        u32 num_vertices = positions.size();

        if (desc.has_texture_coordinates) {
            std::vector<v2> texture_coordinates;
            texture_coordinates.resize(num_vertices);
            texture_coordinates[0] = v2::zero();
            texture_coordinates[1] = v2::X();
            texture_coordinates[2] = v2::Y();
            texture_coordinates[3] = v2::splat(1.0F);
            std::vector<f32> data = to_list(texture_coordinates);
            mesh.m_attributes[Mesh::UV.m_id] = Mesh::AttributeData{Mesh::UV, data};
        }

        if (desc.has_normals) {
            v3 normal_up = v3::X();

            std::vector<v3> normals;
            normals.resize(num_vertices);
            normals[0] = normal_up;
            normals[1] = normal_up;
            normals[2] = normal_up;
            normals[3] = normal_up;
            std::vector<f32> data = to_list(normals);
            mesh.m_attributes[Mesh::NORMAL.m_id] =
                Mesh::AttributeData{Mesh::NORMAL, data};
        }
        // std::vector<u32> indices = {0, 3, 1, 3, 0, 2};
        std::vector<u32> indices = {0, 1, 3, 3, 2, 0};
        mesh.m_indices = indices;
    } else {
        std::vector<v3> positions;
        if (desc.has_position) {
            positions.resize(6);
            positions[0] = v3::create(pos.x, pos.y, pos.z);
            positions[1] = v3::create(pos.x + size.x, pos.y, pos.z);
            positions[2] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);

            positions[3] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
            positions[4] = v3::create(pos.x, pos.y + size.y, pos.z);
            positions[5] = v3::create(pos.x, pos.y, pos.z);
            std::vector<f32> data = to_list(positions);
            mesh.m_attributes[Mesh::POSITION.m_id] =
                Mesh::AttributeData{Mesh::POSITION, data};
        }
        std::vector<v2> texture_coordinates;
        if (desc.has_texture_coordinates) {
            texture_coordinates.resize(6);
            texture_coordinates[0] = v2::zero();
            texture_coordinates[1] = v2::X();
            texture_coordinates[2] = v2::splat(1.0F);
            texture_coordinates[3] = v2::splat(1.0F);
            texture_coordinates[4] = v2::Y();
            texture_coordinates[5] = v2::zero();
            std::vector<f32> data = to_list(texture_coordinates);
            mesh.m_attributes[Mesh::UV.m_id] = Mesh::AttributeData{Mesh::UV, data};
        }

        std::vector<v3> normals;
        if (desc.has_normals) {
            v3 normal_up = v3::X();
            normals.resize(6);
            normals[0] = normal_up;
            normals[1] = normal_up;
            normals[2] = normal_up;
            normals[3] = normal_up;
            normals[4] = normal_up;
            normals[5] = normal_up;
            std::vector<f32> data = to_list(normals);
            mesh.m_attributes[Mesh::NORMAL.m_id] =
                Mesh::AttributeData{Mesh::NORMAL, data};
        }
    }
    return mesh;
}

auto MeshBuilder::line(const v3& pos1, const v3& pos2) -> Mesh {
    Mesh            mesh;
    std::vector<v3> positions;
    positions.resize(2);
    positions[0] = pos1;
    positions[1] = pos2;
    std::vector<f32> position_data = to_list(positions);
    mesh.m_attributes[Mesh::POSITION.m_id] =
        Mesh::AttributeData{Mesh::POSITION, position_data};

    std::vector<v2> texture_coordinates;
    texture_coordinates.resize(2);
    texture_coordinates[0] = v2::zero();
    texture_coordinates[1] = v2::zero();
    std::vector<f32> uv_data = to_list(texture_coordinates);
    mesh.m_attributes[Mesh::UV.m_id] = Mesh::AttributeData{Mesh::UV, uv_data};

    std::vector<u32> indices = {0, 1};
    mesh.m_indices = indices;

    return mesh;
}

auto MeshBuilder::triangle(const v3& pos1, const v3& pos2, const v3& pos3) -> Mesh {
    Mesh mesh;

    std::vector<v3> positions;
    positions.resize(3);
    positions[0] = pos1;
    positions[1] = pos2;
    positions[2] = pos3;
    std::vector<f32> position_data = to_list(positions);
    mesh.m_attributes[Mesh::POSITION.m_id] =
        Mesh::AttributeData{Mesh::POSITION, position_data};

    std::vector<u32> indices = {0, 1, 2};
    mesh.m_indices = indices;

    return mesh;
}

auto MeshBuilder::circle(const v3& position, const f32 radius, const u32 numSegments)
    -> Mesh {

    constexpr const f64 PI = std::numbers::pi;

    const f32 theta = 2.0F * static_cast<const f32>(PI) / static_cast<f32>(numSegments);
    const f32 cos = cosf(theta); // calculate the x component
    const f32 sin = sinf(theta); // calculate the y component

    Mesh mesh;

    std::vector<v3> positions;
    positions.resize(numSegments + 1);
    positions[0] = position;
    f32 x = radius;
    f32 y = 0;
    for (u32 i = 1; i < numSegments + 1; i++) {
        positions[i] =
            v3::create(x + position.x, y + position.y, position.z); // output vertex

        const f32 t = x;
        x = (cos * x) - (sin * y);
        y = (sin * t) + (cos * y);
    }
    std::vector<f32> position_data = to_list(positions);
    mesh.m_attributes[Mesh::POSITION.m_id] =
        Mesh::AttributeData{Mesh::POSITION, position_data};

    auto             num_index = (numSegments * 3);
    std::vector<u32> indices;
    indices.resize(num_index);

    for (u32 i = 0; i < numSegments; i++) {
        auto base_index = 3 * i;
        indices[base_index + 0] = 0;
        indices[base_index + 1] = i + 1;
        indices[base_index + 2] = (i + 2);
    }
    indices[num_index - 1] = indices[1];

    mesh.m_indices = indices;
    return mesh;
}

auto MeshBuilder::cube(const v3& pos, const v3& size) -> Mesh {
    Mesh mesh;

    std::vector<v3> positions;
    positions.resize(36);

    // back face -z
    positions[0] = v3::create(pos.x, pos.y, pos.z);
    positions[1] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
    positions[2] = v3::create(pos.x + size.x, pos.y, pos.z);
    positions[3] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
    positions[4] = v3::create(pos.x, pos.y, pos.z);
    positions[5] = v3::create(pos.x, pos.y + size.y, pos.z);

    // front face +z
    positions[6] = v3::create(pos.x, pos.y, pos.z + size.z);
    positions[7] = v3::create(pos.x + size.x, pos.y, pos.z + size.z);
    positions[8] = v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    positions[9] = v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    positions[10] = v3::create(pos.x, pos.y + size.y, pos.z + size.z);
    positions[11] = v3::create(pos.x, pos.y, pos.z + size.z);

    // left face -x
    positions[12] = v3::create(pos.x, pos.y + size.y, pos.z + size.z);
    positions[13] = v3::create(pos.x, pos.y + size.y, pos.z);
    positions[14] = v3::create(pos.x, pos.y, pos.z);
    positions[15] = v3::create(pos.x, pos.y, pos.z);
    positions[16] = v3::create(pos.x, pos.y, pos.z + size.z);
    positions[17] = v3::create(pos.x, pos.y + size.y, pos.z + size.z);

    // right face +x
    positions[18] = v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    positions[19] = v3::create(pos.x + size.x, pos.y, pos.z);
    positions[20] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
    positions[21] = v3::create(pos.x + size.x, pos.y, pos.z);
    positions[22] = v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    positions[23] = v3::create(pos.x + size.x, pos.y, pos.z + size.z);

    // bottom face
    positions[24] = v3::create(pos.x, pos.y, pos.z);
    positions[25] = v3::create(pos.x + size.x, pos.y, pos.z);
    positions[26] = v3::create(pos.x + size.x, pos.y, pos.z + size.z);
    positions[27] = v3::create(pos.x + size.x, pos.y, pos.z + size.z);
    positions[28] = v3::create(pos.x, pos.y, pos.z + size.z);
    positions[29] = v3::create(pos.x, pos.y, pos.z);

    // top face
    positions[30] = v3::create(pos.x, pos.y + size.y, pos.z);
    positions[31] = v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    positions[32] = v3::create(pos.x + size.x, pos.y + size.y, pos.z);
    positions[33] = v3::create(pos.x + size.x, pos.y + size.y, pos.z + size.z);
    positions[34] = v3::create(pos.x, pos.y + size.y, pos.z);
    positions[35] = v3::create(pos.x, pos.y + size.y, pos.z + size.z);

    std::vector<f32> position_data = to_list(positions);
    mesh.m_attributes[Mesh::POSITION.m_id] =
        Mesh::AttributeData{Mesh::POSITION, position_data};

    std::vector<v2> texture_coordinates;
    texture_coordinates.resize(36);
    // back face -z
    texture_coordinates[0] = v2::X();
    texture_coordinates[1] = v2::splat(1.0F);
    texture_coordinates[2] = v2::Y();
    texture_coordinates[3] = v2::Y();
    texture_coordinates[4] = v2::zero();
    texture_coordinates[5] = v2::X();

    // front face +z
    texture_coordinates[6] = v2::zero();
    texture_coordinates[7] = v2::zero();
    texture_coordinates[8] = v2::zero();
    texture_coordinates[9] = v2::zero();
    texture_coordinates[10] = v2::zero();
    texture_coordinates[11] = v2::zero();

    // left face
    texture_coordinates[12] = v2::NEG_X();
    texture_coordinates[13] = v2::NEG_X();
    texture_coordinates[14] = v2::NEG_X();
    texture_coordinates[15] = v2::NEG_X();
    texture_coordinates[16] = v2::NEG_X();
    texture_coordinates[17] = v2::NEG_X();

    // right face +x
    texture_coordinates[18] = v2::X();
    texture_coordinates[19] = v2::X();
    texture_coordinates[20] = v2::X();
    texture_coordinates[21] = v2::X();
    texture_coordinates[22] = v2::X();
    texture_coordinates[23] = v2::X();

    // bottom face -y
    texture_coordinates[24] = v2::X();
    texture_coordinates[25] = v2::splat(1.0F);
    texture_coordinates[26] = v2::Y();
    texture_coordinates[27] = v2::Y();
    texture_coordinates[28] = v2::zero();
    texture_coordinates[29] = v2::X();

    // top face +y
    texture_coordinates[30] = v2::Y();
    texture_coordinates[31] = v2::Y();
    texture_coordinates[32] = v2::Y();
    texture_coordinates[33] = v2::Y();
    texture_coordinates[34] = v2::Y();
    texture_coordinates[35] = v2::Y();

    std::vector<f32> uv_data = to_list(texture_coordinates);
    mesh.m_attributes[Mesh::UV.m_id] = Mesh::AttributeData{Mesh::UV, uv_data};

    std::vector<v3> normals;
    normals.resize(36);
    for (size_t i = 0; i < normals.size(); i++) {
        std::array<v3, 6> s;
        s[0] = v3::create(+0.0f, +0.0f, -1.0f); // back
        s[1] = v3::create(+0.0f, +0.0f, +1.0f); // front
        s[2] = v3::create(-1.0f, +0.0f, +0.0f); // left
        s[3] = v3::create(+1.0f, +0.0f, +0.0f); // right
        s[4] = v3::create(+0.0f, -1.0f, +0.0f); // bottom
        s[5] = v3::create(+0.0f, +1.0f, +0.0f); // up

        normals[i] = s[i / 6];
    };
    std::vector<f32> normal_data = to_list(normals);
    mesh.m_attributes[Mesh::NORMAL.m_id] = Mesh::AttributeData{Mesh::NORMAL, normal_data};

    return mesh;
}

struct MeshVertex {
    std::map<Mesh::VertexAttributeId, Mesh::AttributeData> m_attributes;
};

auto convert_into_data(const Mesh& mesh, const bool interleaved) -> std::vector<f32> {
    if (interleaved) {
        auto calc_combined_attribute_count = [&](const Mesh& mesh) -> u32 {
            u32 combined_attribute_count = 0;
            for (const auto& [id, data] : mesh.m_attributes) {
                u32 size = component_count(data.m_attribute_id.m_format);
                u32 amount = data.m_data.size() / size;
                combined_attribute_count += amount * size;
            }
            return combined_attribute_count;
        };
        u32 combined_attribute_count = calc_combined_attribute_count(mesh);

        u32 position_count = mesh.m_attributes.at(Mesh::POSITION.m_id).m_data.size() /
                             component_count(Mesh::POSITION.m_format);

        std::vector<MeshVertex> vertices;
        vertices.resize(position_count);
        for (const auto& [id, data] : mesh.m_attributes) {
            u32 size = component_count(data.m_attribute_id.m_format);
            u32 amount = data.m_data.size() / size;
            for (u32 i = 0; i < amount; i++) {
                MeshVertex& vertex = vertices[i];
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

} // namespace JadeFrame