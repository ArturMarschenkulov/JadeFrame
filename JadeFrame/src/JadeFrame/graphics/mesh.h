#pragma once
#include <map>
#include "JadeFrame/math/vec.h"
#include "JadeFrame/types.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/utils/assert.h"
#include "color.h"

namespace JadeFrame {

class MeshBuilder;

enum class PRIMITIVE_TOPOLOGY : u8 {
    POINT_LIST,
    LINE_LIST,
    LINE_STRIP,
    TRIANGLE_LIST,
    TRIANGLE_STRIP,
};

// NOTE(artur): This type should be very ergonomic to use, as it is client facing. The
// goal is not to be the most efficient, but the most ergonomic. As this will describe
// other structures down the line.
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

        [[nodiscard]] auto count_components() const -> u32 {
            return component_count(m_format);
        }
    };

    struct AttributeData {
        VertexAttribute m_attribute;
        // TODO: m_data could also be some other type like u32 or v3
        std::vector<f32> m_data;

        [[nodiscard]] auto count() const -> u32 {
            return u32(m_data.size()) / m_attribute.count_components();
        }
    };

    PRIMITIVE_TOPOLOGY m_topology = PRIMITIVE_TOPOLOGY::TRIANGLE_LIST;
    std::map<VertexAttributeId, AttributeData> m_attributes;
    std::vector<u32>                           m_indices;

    constexpr const static VertexAttribute POSITION =
        {.m_name = "POSITION", .m_id = 0, .m_format = SHADER_TYPE::V_3_F32};
    constexpr const static VertexAttribute COLOR =
        {.m_name = "COLOR", .m_id = 1, .m_format = SHADER_TYPE::V_4_F32};
    constexpr const static VertexAttribute UV =
        {.m_name = "UV", .m_id = 2, .m_format = SHADER_TYPE::V_2_F32};
    constexpr const static VertexAttribute NORMAL =
        {.m_name = "NORMAL", .m_id = 3, .m_format = SHADER_TYPE::V_3_F32};
    constexpr const static VertexAttribute TANGENT =
        {.m_name = "TANGENT", .m_id = 4, .m_format = SHADER_TYPE::V_4_F32};

    [[nodiscard]] auto has_attribute(const VertexAttribute& attribute) const -> bool {
        return m_attributes.contains(attribute.m_id);
    }

    auto set_color(const RGBAColor& color) -> void {

        const auto* pos = this->attribute_data(POSITION.m_id);
        JF_ASSERT(pos != nullptr, "Mesh has no positions");

        const u32        num_vertices = pos->count();
        std::vector<f32> color_data;
        color_data.resize(num_vertices * 4);
        for (u32 i = 0; i < num_vertices; i++) {
            auto base_index = i * 4;
            color_data[base_index + 0] = color.r;
            color_data[base_index + 1] = color.g;
            color_data[base_index + 2] = color.b;
            color_data[base_index + 3] = color.a;
        }
        bool insert_success = this->insert_attribute(COLOR, std::move(color_data));
        assert((insert_success) && "Failed to insert color attribute");
    }

    static auto builder() -> MeshBuilder;

    // Returns false if format mismatch or other failure
    auto insert_attribute(const VertexAttribute& attr, std::vector<f32> values) -> bool {
        // Optional: validate that values.size() % attr.count_components() == 0
        const u32 comps = attr.count_components();
        if (comps == 0 || (values.size() % comps) != 0) {
            // log or assert; return false instead of throwing
            return false;
        }

        AttributeData data{.m_attribute = attr, .m_data = std::move(values)};
        m_attributes[attr.m_id] = std::move(data);
        return true;
    }

    [[nodiscard]] auto
    with_inserted_attribute(const VertexAttribute& attr, std::vector<f32> values) const
        -> Mesh {
        Mesh copy = *this;
        copy.insert_attribute(attr, std::move(values));
        return copy;
    }

    [[nodiscard]] auto attribute_values(VertexAttributeId id) const
        -> const std::vector<f32>* {
        auto it = m_attributes.find(id);
        return (it == m_attributes.end()) ? nullptr : &it->second.m_data;
    }

    [[nodiscard]] auto attribute_data(VertexAttributeId id) const
        -> const AttributeData* {
        auto it = m_attributes.find(id);
        return (it == m_attributes.end()) ? nullptr : &it->second;
    }

    auto attribute_data_mut(VertexAttributeId id) -> AttributeData* {
        auto it = m_attributes.find(id);
        return (it == m_attributes.end()) ? nullptr : &it->second;
    }

    [[nodiscard]] auto contains_attribute(VertexAttributeId id) const -> bool {
        return m_attributes.contains(id);
    }
};

class MeshBuilder {
public:
    struct Desc {
        bool has_position = true; // NOTE: Probably unneccessary
        bool has_texture_coordinates = true;
        bool has_indices = false;
        bool has_normals = true;
    };

    // NOTE(artur): Rectangle for internal OpenGL code.
    static auto rectangle_opengl(const v3& pos, const v3& size, const Desc desc) -> Mesh;
    static auto rectangle(const v3& pos, const v3& size, const Desc desc) -> Mesh;
    static auto line(const v3& pos1, const v3& pos2) -> Mesh;
    static auto triangle(const v3& pos1, const v3& pos2, const v3& pos3) -> Mesh;
    static auto circle(const v3& position, const f32 radius, const u32 numSegments)
        -> Mesh;
    static auto cube(const v3& pos, const v3& size) -> Mesh;

private:
};

static auto to_list(const std::vector<v2>& v2s) -> std::vector<f32> {
    std::vector<f32> result;
    result.reserve(v2s.size() * 2);
    for (const v2& v : v2s) {
        result.push_back(v.x);
        result.push_back(v.y);
    }
    return result;
}

static auto to_list(const std::vector<v4>& v2s) -> std::vector<f32> {
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

static auto to_list(const std::vector<v3>& v3s) -> std::vector<f32> {
    std::vector<f32> result;
    result.reserve(v3s.size() * 3);
    for (const v3& v : v3s) {
        result.push_back(v.x);
        result.push_back(v.y);
        result.push_back(v.z);
    }
    return result;
}

static auto to_list(const std::vector<RGBAColor>& colors) -> std::vector<f32> {
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

auto convert_into_data(const Mesh& mesh, const bool interleaved) -> std::vector<f32>;

} // namespace JadeFrame