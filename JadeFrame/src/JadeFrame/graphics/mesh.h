#pragma once
#include <map>
#include "JadeFrame/math/vec.h"
#include "JadeFrame/types.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/utils/assert.h"
#include "color.h"

namespace JadeFrame {

struct Vertex {
    v3        position;
    RGBAColor color;
    v2        tex_coord;
};

/// To be removed. Use [`Mesh`] instead.
class VertexData {
public:
    std::vector<v3>        m_positions;
    std::vector<RGBAColor> m_colors;
    std::vector<v2>        m_texture_coordinates;
    std::vector<v3>        m_normals;

    std::vector<u32> m_indices;

    struct Desc {
        bool has_position = true; // NOTE: Probably unneccessary
        bool has_texture_coordinates = true;
        bool has_indices = false;
        bool has_normals = true;
    };

    auto set_color(const RGBAColor& color) -> void {
        auto num_vertices = m_positions.size();
        m_colors.clear();
        m_colors.resize(num_vertices, color);
    }

    static auto line(const v3& pos1, const v3& pos2) -> VertexData;

    static auto rectangle(
        const v3&  pos,
        const v3&  size,
        const Desc desc = Desc{true, true, false, true}
    ) -> VertexData;
    static auto triangle(const v3& pos1, const v3& pos2, const v3& pos3) -> VertexData;
    static auto circle(const v3& position, const f32 radius, const u32 numSegments)
        -> VertexData;

    static auto cube(const v3& pos, const v3& size) -> VertexData;
};

auto convert_into_data(const Mesh& mesh, const bool interleaved) -> std::vector<f32>;
auto convert_into_data(const VertexData& vertex_data, const bool interleaved)
    -> std::vector<f32>;

class Mesh {
public:
    using VertexAttributeId = u32;

    struct VertexAttribute {
        using Id = u32;
        /// human readable name of the attribute
        const char* m_name;
        /// the id of the attribute
        Id m_id;
        /// the format of the attribute
        SHADER_TYPE m_format;

        [[nodiscard]] auto count() const -> u32 { return component_count(m_format); }
    };

    struct AttributeData {
        VertexAttribute m_attribute_id;
        // TODO: m_data could also be some other type like u32 or v3
        std::vector<f32> m_data;

        [[nodiscard]] auto count() const -> u32 {
            return u32(m_data.size()) / m_attribute_id.count();
        }
    };

    std::map<VertexAttributeId, AttributeData> m_attributes;
    std::vector<u32>                           m_indices;

    constexpr const static VertexAttribute POSITION =
        {"POSITION", 0, SHADER_TYPE::V_3_F32};
    constexpr const static VertexAttribute COLOR = {"COLOR", 1, SHADER_TYPE::V_4_F32};
    constexpr const static VertexAttribute UV = {"UV", 2, SHADER_TYPE::V_2_F32};
    constexpr const static VertexAttribute NORMAL = {"NORMAL", 3, SHADER_TYPE::V_3_F32};
    constexpr const static VertexAttribute TANGENT = {"TANGENT", 4, SHADER_TYPE::V_4_F32};

    [[nodiscard]] auto has_attribute(const VertexAttribute& attribute) const -> bool {
        return m_attributes.contains(attribute.m_id);
    }

    auto set_color(const RGBAColor& color) -> void {
        auto it = m_attributes.find(POSITION.m_id);
        JF_ASSERT(it != m_attributes.end(), "Mesh has no positions");
        const u32        num_vertices = it->second.count();
        std::vector<f32> color_data;
        color_data.resize(num_vertices * 4);
        for (u32 i = 0; i < num_vertices; i++) {
            auto base_index = i * 4;
            color_data[base_index + 0] = color.r;
            color_data[base_index + 1] = color.g;
            color_data[base_index + 2] = color.b;
            color_data[base_index + 3] = color.a;
        }
        m_attributes[COLOR.m_id] = AttributeData{COLOR, color_data};
        assert(
            (m_attributes[COLOR.m_id].m_data.size() == num_vertices * 4) && "Invalid size"
        );
    }

    struct Desc {
        bool has_position = true; // NOTE: Probably unneccessary
        bool has_texture_coordinates = true;
        bool has_indices = false;
        bool has_normals = true;
    };

    static auto rectangle_(const v3& pos, const v3& size, const Desc desc) -> Mesh;
    static auto rectangle(const v3& pos, const v3& size, const Desc desc) -> Mesh;
};

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

static auto to_list(std::vector<RGBAColor> colors) -> std::vector<f32> {
    std::vector<f32> result;
    result.reserve(colors.size() * 4);
    for (const RGBAColor& color : colors) {
        result.push_back(color.r);
        result.push_back(color.g);
        result.push_back(color.b);
        result.push_back(color.a);
    }
    return result;
}

static auto to_list(v4 v4) -> std::vector<f32> { return {v4.x, v4.y, v4.z, v4.w}; }

static auto to_list(v3 v3) -> std::vector<f32> { return {v3.x, v3.y, v3.z}; }

static auto to_list(v2 v2) -> std::vector<f32> { return {v2.x, v2.y}; }

} // namespace JadeFrame