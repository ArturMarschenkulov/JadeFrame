#pragma once
#include <map>
#include "JadeFrame/math/vec.h"
#include "JadeFrame/types.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/utils/result.h"
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

enum class MeshErrorCode : u8 {
    MissingPosition,
    InvalidAttributeId,
    UnsupportedAttributeFormat,
    AttributeDataMisaligned,
    AttributeVertexCountMismatch,
    EmptyMesh,
    InvalidTopology,
    InvalidIndexCount,
    InvalidIndex,
    InvalidDescriptor,
    InvalidSegmentCount,
    InvalidParameter,
};

struct MeshError {
    MeshErrorCode m_code;
    std::string   m_message;
};

using MeshValidationResult = Result<bool, MeshError>;

[[nodiscard]] auto to_string(MeshErrorCode code) -> const char*;

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
        // Mesh conversion currently uses canonical f32 storage. Unsupported shader
        // formats are rejected by validate() instead of being reinterpreted.
        std::vector<f32> m_data;

        [[nodiscard]] auto count() const -> size_t {
            const u32 components = m_attribute.count_components();
            return components == 0 ? 0 : m_data.size() / components;
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

    [[nodiscard]] auto set_color(const RGBAColor& color) -> MeshValidationResult;

    [[nodiscard]] auto validate() const -> MeshValidationResult;

    static auto builder() -> MeshBuilder;

    // Returns an error if the values do not describe a complete, valid attribute for
    // this mesh. The mesh is left unchanged on failure.
    [[nodiscard]] auto insert_attribute(
        const VertexAttribute& attr,
        std::vector<f32>       values
    ) -> MeshValidationResult;

    [[nodiscard]] auto
    with_inserted_attribute(const VertexAttribute& attr, std::vector<f32> values) const
        -> Result<Mesh, MeshError> {
        Mesh copy = *this;
        auto insertion = copy.insert_attribute(attr, std::move(values));
        if (insertion.is_err()) { return err<Mesh, MeshError>(insertion.error()); }
        return ok<Mesh, MeshError>(std::move(copy));
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
        bool has_position = true;
        bool has_texture_coordinates = true;
        bool has_indices = false;
        bool has_normals = true;
    };

    // NOTE(artur): Rectangle for internal OpenGL code.
    [[nodiscard]] static auto rectangle_opengl(
        const v3& pos,
        const v3& size,
        const Desc desc
    ) -> Result<Mesh, MeshError>;
    [[nodiscard]] static auto rectangle(
        const v3& pos,
        const v3& size,
        const Desc desc
    ) -> Result<Mesh, MeshError>;
    [[nodiscard]] static auto line(const v3& pos1, const v3& pos2)
        -> Result<Mesh, MeshError>;
    [[nodiscard]] static auto triangle(const v3& pos1, const v3& pos2, const v3& pos3)
        -> Result<Mesh, MeshError>;
    [[nodiscard]] static auto circle(
        const v3& position,
        const f32 radius,
        const u32 numSegments
    ) -> Result<Mesh, MeshError>;
    [[nodiscard]] static auto cube(const v3& pos, const v3& size)
        -> Result<Mesh, MeshError>;

private:
};

inline auto to_list(const std::vector<v2>& v2s) -> std::vector<f32> {
    std::vector<f32> result;
    result.reserve(v2s.size() * 2);
    for (const v2& v : v2s) {
        result.push_back(v.x);
        result.push_back(v.y);
    }
    return result;
}

inline auto to_list(const std::vector<v4>& v2s) -> std::vector<f32> {
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

inline auto to_list(const std::vector<v3>& v3s) -> std::vector<f32> {
    std::vector<f32> result;
    result.reserve(v3s.size() * 3);
    for (const v3& v : v3s) {
        result.push_back(v.x);
        result.push_back(v.y);
        result.push_back(v.z);
    }
    return result;
}

inline auto to_list(const std::vector<RGBAColor>& colors) -> std::vector<f32> {
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

inline auto to_list(v4 value) -> std::vector<f32> {
    return {value.x, value.y, value.z, value.w};
}

inline auto to_list(v3 value) -> std::vector<f32> {
    return {value.x, value.y, value.z};
}

inline auto to_list(v2 value) -> std::vector<f32> { return {value.x, value.y}; }

[[nodiscard]] auto convert_into_data(const Mesh& mesh, const bool interleaved)
    -> Result<std::vector<f32>, MeshError>;

} // namespace JadeFrame
