#include "JadeFrame/math/vec.h"
#include "mesh.h"

#include <cmath>
#include <cstddef>
#include <limits>
#include <numbers>

namespace JadeFrame {

namespace {

auto make_error(const MeshErrorCode code, std::string message) -> MeshError {
    return MeshError{.m_code = code, .m_message = std::move(message)};
}

auto error_result(const MeshErrorCode code, std::string message)
    -> MeshValidationResult {
    return err<bool, MeshError>(make_error(code, std::move(message)));
}

auto supported_component_count(const SHADER_TYPE format) -> size_t {
    // AttributeData currently stores canonical f32 values. Keep this list explicit so
    // integer, double, opaque, and matrix shader types cannot be silently reinterpreted
    // as floats during conversion.
    switch (format) {
        case SHADER_TYPE::F32: return 1;
        case SHADER_TYPE::V_2_F32: return 2;
        case SHADER_TYPE::V_3_F32: return 3;
        case SHADER_TYPE::V_4_F32: return 4;
        default: return 0;
    }
}

auto is_finite(const v3& value) -> bool {
    return std::isfinite(value.x) && std::isfinite(value.y) &&
           std::isfinite(value.z);
}

auto add_attribute(
    Mesh& mesh,
    const Mesh::VertexAttribute& attribute,
    std::vector<f32> values
) -> void {
    mesh.m_attributes.emplace(
        attribute.m_id,
        Mesh::AttributeData{.m_attribute = attribute, .m_data = std::move(values)}
    );
}

auto finish_mesh(Mesh mesh) -> Result<Mesh, MeshError> {
    auto validation = mesh.validate();
    if (validation.is_err()) { return err<Mesh, MeshError>(validation.error()); }
    return ok<Mesh, MeshError>(std::move(mesh));
}

auto validate_rectangle_inputs(
    const v3&              pos,
    const v3&              size,
    const MeshBuilder::Desc& desc
) -> MeshValidationResult {
    if (!desc.has_position) {
        return error_result(
            MeshErrorCode::InvalidDescriptor,
            "A generated rectangle requires a position attribute"
        );
    }
    if (!is_finite(pos) || !is_finite(size)) {
        return error_result(
            MeshErrorCode::InvalidParameter,
            "Rectangle position and size must contain finite values"
        );
    }
    return ok<bool, MeshError>(true);
}

auto build_rectangle(
    const v3&               pos,
    const v3&               size,
    const MeshBuilder::Desc desc,
    const bool              opengl_winding
) -> Result<Mesh, MeshError> {
    auto inputs = validate_rectangle_inputs(pos, size, desc);
    if (inputs.is_err()) { return err<Mesh, MeshError>(inputs.error()); }

    const v3 pos_o = pos;
    const v3 pos_x = pos_o + v3::create(size.x, 0.0F, 0.0F);
    const v3 pos_y = pos_o + v3::create(0.0F, size.y, 0.0F);
    const v3 pos_xy = pos_o + v3::create(size.x, size.y, 0.0F);

    Mesh mesh;
    std::vector<v3> positions;
    std::vector<v2> texture_coordinates;

    if (desc.has_indices) {
        positions = {pos_o, pos_x, pos_y, pos_xy};
        mesh.m_indices = opengl_winding ? std::vector<u32>{0, 3, 1, 3, 0, 2}
                                        : std::vector<u32>{0, 1, 3, 3, 2, 0};
        texture_coordinates = {
            v2::zero(),
            v2::X(),
            v2::Y(),
            v2::splat(1.0F),
        };
    } else if (opengl_winding) {
        positions = {pos_o, pos_xy, pos_x, pos_xy, pos_o, pos_y};
        texture_coordinates = {
            v2::zero(),
            v2::splat(1.0F),
            v2::X(),
            v2::splat(1.0F),
            v2::zero(),
            v2::Y(),
        };
    } else {
        positions = {pos_o, pos_x, pos_xy, pos_xy, pos_y, pos_o};
        texture_coordinates = {
            v2::zero(),
            v2::X(),
            v2::splat(1.0F),
            v2::splat(1.0F),
            v2::Y(),
            v2::zero(),
        };
    }

    add_attribute(mesh, Mesh::POSITION, to_list(positions));

    if (desc.has_texture_coordinates) {
        add_attribute(mesh, Mesh::UV, to_list(texture_coordinates));
    }

    if (desc.has_normals) {
        add_attribute(
            mesh,
            Mesh::NORMAL,
            to_list(std::vector<v3>(positions.size(), v3::Z()))
        );
    }

    return finish_mesh(std::move(mesh));
}

auto append_face_uvs(std::vector<v2>& texture_coordinates) -> void {
    texture_coordinates.insert(
        texture_coordinates.end(),
        {v2::zero(), v2::X(), v2::splat(1.0F), v2::splat(1.0F), v2::Y(), v2::zero()}
    );
}

} // namespace

auto to_string(const MeshErrorCode code) -> const char* {
    switch (code) {
        case MeshErrorCode::MissingPosition: return "MissingPosition";
        case MeshErrorCode::InvalidAttributeId: return "InvalidAttributeId";
        case MeshErrorCode::UnsupportedAttributeFormat:
            return "UnsupportedAttributeFormat";
        case MeshErrorCode::AttributeDataMisaligned: return "AttributeDataMisaligned";
        case MeshErrorCode::AttributeVertexCountMismatch:
            return "AttributeVertexCountMismatch";
        case MeshErrorCode::EmptyMesh: return "EmptyMesh";
        case MeshErrorCode::InvalidTopology: return "InvalidTopology";
        case MeshErrorCode::InvalidIndexCount: return "InvalidIndexCount";
        case MeshErrorCode::InvalidIndex: return "InvalidIndex";
        case MeshErrorCode::InvalidDescriptor: return "InvalidDescriptor";
        case MeshErrorCode::InvalidSegmentCount: return "InvalidSegmentCount";
        case MeshErrorCode::InvalidParameter: return "InvalidParameter";
        default: return "UnknownMeshError";
    }
}

auto Mesh::builder() -> MeshBuilder { return MeshBuilder{}; }

auto Mesh::validate() const -> MeshValidationResult {
    const auto position_it = m_attributes.find(Mesh::POSITION.m_id);
    if (position_it == m_attributes.end()) {
        return error_result(
            MeshErrorCode::MissingPosition,
            "Mesh must contain a POSITION attribute"
        );
    }

    const AttributeData& position_data = position_it->second;
    const size_t         position_components =
        supported_component_count(position_data.m_attribute.m_format);
    if (position_data.m_attribute.m_id != position_it->first) {
        return error_result(
            MeshErrorCode::InvalidAttributeId,
            "Attribute map key does not match the attribute descriptor"
        );
    }
    if (position_data.m_attribute.m_format != Mesh::POSITION.m_format) {
        return error_result(
            MeshErrorCode::UnsupportedAttributeFormat,
            "The POSITION attribute must use the V_3_F32 format"
        );
    }
    if (position_components == 0) {
        return error_result(
            MeshErrorCode::UnsupportedAttributeFormat,
            "POSITION uses an unsupported attribute format"
        );
    }

    const size_t vertex_count = position_data.m_data.size() / position_components;
    for (const auto& [id, data] : m_attributes) {
        if (data.m_attribute.m_name == nullptr || data.m_attribute.m_name[0] == '\0') {
            return error_result(
                MeshErrorCode::InvalidAttributeId,
                "Vertex attributes must have a non-empty name"
            );
        }
        if (id != data.m_attribute.m_id) {
            return error_result(
                MeshErrorCode::InvalidAttributeId,
                "Attribute map key does not match the attribute descriptor"
            );
        }

        const size_t components = supported_component_count(data.m_attribute.m_format);
        if (components == 0) {
            return error_result(
                MeshErrorCode::UnsupportedAttributeFormat,
                std::string("Attribute ") + data.m_attribute.m_name +
                    " is not representable by Mesh::AttributeData"
            );
        }
        if ((data.m_data.size() % components) != 0) {
            return error_result(
                MeshErrorCode::AttributeDataMisaligned,
                std::string("Attribute ") + data.m_attribute.m_name +
                    " contains a partial vertex"
            );
        }
        for (const f32 value : data.m_data) {
            if (!std::isfinite(value)) {
                return error_result(
                    MeshErrorCode::InvalidParameter,
                    std::string("Attribute ") + data.m_attribute.m_name +
                        " contains a non-finite value"
                );
            }
        }

        if (data.count() != vertex_count) {
            return error_result(
                MeshErrorCode::AttributeVertexCountMismatch,
                std::string("Attribute ") + data.m_attribute.m_name +
                    " does not contain the same number of vertices as POSITION"
            );
        }
    }

    if (vertex_count == 0) {
        return error_result(
            MeshErrorCode::EmptyMesh,
            "Mesh must contain at least one vertex"
        );
    }

    const size_t primitive_count =
        m_indices.empty() ? vertex_count : m_indices.size();
    switch (m_topology) {
        case PRIMITIVE_TOPOLOGY::POINT_LIST: break;
        case PRIMITIVE_TOPOLOGY::LINE_LIST:
            if ((primitive_count % 2) != 0) {
                return error_result(
                    MeshErrorCode::InvalidIndexCount,
                    "LINE_LIST requires an even number of vertices or indices"
                );
            }
            break;
        case PRIMITIVE_TOPOLOGY::LINE_STRIP:
            if (primitive_count < 2) {
                return error_result(
                    MeshErrorCode::InvalidIndexCount,
                    "LINE_STRIP requires at least two vertices or indices"
                );
            }
            break;
        case PRIMITIVE_TOPOLOGY::TRIANGLE_LIST:
            if ((primitive_count % 3) != 0) {
                return error_result(
                    MeshErrorCode::InvalidIndexCount,
                    "TRIANGLE_LIST requires a multiple of three vertices or indices"
                );
            }
            break;
        case PRIMITIVE_TOPOLOGY::TRIANGLE_STRIP:
            if (primitive_count < 3) {
                return error_result(
                    MeshErrorCode::InvalidIndexCount,
                    "TRIANGLE_STRIP requires at least three vertices or indices"
                );
            }
            break;
        default:
            return error_result(
                MeshErrorCode::InvalidTopology,
                "Mesh contains an unknown primitive topology"
            );
    }

    for (const u32 index : m_indices) {
        if (static_cast<size_t>(index) >= vertex_count) {
            return error_result(
                MeshErrorCode::InvalidIndex,
                "Mesh index refers to a vertex outside the POSITION attribute"
            );
        }
    }

    return ok<bool, MeshError>(true);
}

auto Mesh::insert_attribute(
    const VertexAttribute& attr,
    std::vector<f32>       values
) -> MeshValidationResult {
    if (attr.m_name == nullptr || attr.m_name[0] == '\0') {
        return error_result(
            MeshErrorCode::InvalidAttributeId,
            "Vertex attributes must have a non-empty name"
        );
    }

    const size_t components = supported_component_count(attr.m_format);
    if (components == 0) {
        return error_result(
            MeshErrorCode::UnsupportedAttributeFormat,
            "Attribute format is not representable by Mesh::AttributeData"
        );
    }
    if ((values.size() % components) != 0) {
        return error_result(
            MeshErrorCode::AttributeDataMisaligned,
            "Attribute data contains a partial vertex"
        );
    }

    for (const f32 value : values) {
        if (!std::isfinite(value)) {
            return error_result(
                MeshErrorCode::InvalidParameter,
                "Attribute data must contain finite values"
            );
        }
    }

    Mesh candidate = *this;
    candidate.m_attributes.insert_or_assign(
        attr.m_id,
        AttributeData{.m_attribute = attr, .m_data = std::move(values)}
    );
    auto validation = candidate.validate();
    if (validation.is_err()) { return err<bool, MeshError>(validation.error()); }

    m_attributes = std::move(candidate.m_attributes);
    return ok<bool, MeshError>(true);
}

auto Mesh::set_color(const RGBAColor& color) -> MeshValidationResult {
    auto validation = this->validate();
    if (validation.is_err()) { return err<bool, MeshError>(validation.error()); }

    const auto* position_data = this->attribute_data(POSITION.m_id);
    // validate() above guarantees that POSITION exists and is well-formed.
    if (position_data == nullptr) { return err<bool, MeshError>(make_error(
        MeshErrorCode::MissingPosition,
        "Cannot add COLOR to a mesh without POSITION"
    )); }

    std::vector<f32> color_data(position_data->count() * 4);
    for (size_t i = 0; i < position_data->count(); ++i) {
        const size_t base_index = i * 4;
        color_data[base_index + 0] = color.r;
        color_data[base_index + 1] = color.g;
        color_data[base_index + 2] = color.b;
        color_data[base_index + 3] = color.a;
    }
    return this->insert_attribute(COLOR, std::move(color_data));
}

auto MeshBuilder::rectangle_opengl(
    const v3& pos,
    const v3& size,
    const Desc desc
) -> Result<Mesh, MeshError> {
    return build_rectangle(pos, size, desc, true);
}

auto MeshBuilder::rectangle(
    const v3& pos,
    const v3& size,
    const Desc desc
) -> Result<Mesh, MeshError> {
    return build_rectangle(pos, size, desc, false);
}

auto MeshBuilder::line(const v3& pos1, const v3& pos2) -> Result<Mesh, MeshError> {
    if (!is_finite(pos1) || !is_finite(pos2)) {
        return err<Mesh, MeshError>(make_error(
            MeshErrorCode::InvalidParameter,
            "Line endpoints must contain finite values"
        ));
    }

    Mesh mesh;
    mesh.m_topology = PRIMITIVE_TOPOLOGY::LINE_LIST;
    add_attribute(mesh, Mesh::POSITION, to_list(std::vector<v3>{pos1, pos2}));
    add_attribute(mesh, Mesh::UV, to_list(std::vector<v2>{v2::zero(), v2::X()}));
    return finish_mesh(std::move(mesh));
}

auto MeshBuilder::triangle(
    const v3& pos1,
    const v3& pos2,
    const v3& pos3
) -> Result<Mesh, MeshError> {
    if (!is_finite(pos1) || !is_finite(pos2) || !is_finite(pos3)) {
        return err<Mesh, MeshError>(make_error(
            MeshErrorCode::InvalidParameter,
            "Triangle vertices must contain finite values"
        ));
    }

    Mesh mesh;
    add_attribute(mesh, Mesh::POSITION, to_list(std::vector<v3>{pos1, pos2, pos3}));
    add_attribute(
        mesh,
        Mesh::UV,
        to_list(std::vector<v2>{v2::zero(), v2::X(), v2::create(0.5F, 1.0F)})
    );
    mesh.m_indices = {0, 1, 2};
    return finish_mesh(std::move(mesh));
}

auto MeshBuilder::circle(
    const v3& position,
    const f32 radius,
    const u32 numSegments
) -> Result<Mesh, MeshError> {
    if (numSegments < 3) {
        return err<Mesh, MeshError>(make_error(
            MeshErrorCode::InvalidSegmentCount,
            "A circle requires at least three segments"
        ));
    }
    if (!is_finite(position) || !std::isfinite(radius) || radius < 0.0F) {
        return err<Mesh, MeshError>(make_error(
            MeshErrorCode::InvalidParameter,
            "Circle position and radius must be finite and radius must be non-negative"
        ));
    }
    if (static_cast<size_t>(numSegments) >
        (std::numeric_limits<size_t>::max() - 1) / 3) {
        return err<Mesh, MeshError>(make_error(
            MeshErrorCode::InvalidSegmentCount,
            "Circle segment count is too large"
        ));
    }

    constexpr f32 pi = std::numbers::pi_v<f32>;
    const f32     angle_step = 2.0F * pi / static_cast<f32>(numSegments);

    std::vector<v3> positions;
    std::vector<v2> texture_coordinates;
    std::vector<v3> normals;
    positions.reserve(static_cast<size_t>(numSegments) + 1);
    texture_coordinates.reserve(static_cast<size_t>(numSegments) + 1);
    normals.reserve(static_cast<size_t>(numSegments) + 1);

    positions.push_back(position);
    texture_coordinates.push_back(v2::splat(0.5F));
    normals.push_back(v3::Z());
    for (u32 i = 0; i < numSegments; ++i) {
        const f32 angle = angle_step * static_cast<f32>(i);
        const f32 x = std::cos(angle) * radius;
        const f32 y = std::sin(angle) * radius;
        positions.push_back(v3::create(position.x + x, position.y + y, position.z));
        texture_coordinates.push_back(
            v2::create(0.5F + (0.5F * std::cos(angle)),
                       0.5F + (0.5F * std::sin(angle)))
        );
        normals.push_back(v3::Z());
    }

    Mesh mesh;
    add_attribute(mesh, Mesh::POSITION, to_list(positions));
    add_attribute(mesh, Mesh::UV, to_list(texture_coordinates));
    add_attribute(mesh, Mesh::NORMAL, to_list(normals));
    mesh.m_indices.reserve(static_cast<size_t>(numSegments) * 3);
    for (u32 i = 0; i < numSegments; ++i) {
        mesh.m_indices.push_back(0);
        mesh.m_indices.push_back(i + 1);
        mesh.m_indices.push_back(((i + 1) % numSegments) + 1);
    }
    return finish_mesh(std::move(mesh));
}

auto MeshBuilder::cube(const v3& pos, const v3& size) -> Result<Mesh, MeshError> {
    if (!is_finite(pos) || !is_finite(size)) {
        return err<Mesh, MeshError>(make_error(
            MeshErrorCode::InvalidParameter,
            "Cube position and size must contain finite values"
        ));
    }

    const v3 x_arrow = v3::create(size.x, 0.0F, 0.0F);
    const v3 y_arrow = v3::create(0.0F, size.y, 0.0F);
    const v3 z_arrow = v3::create(0.0F, 0.0F, size.z);

    const v3 pos_o = pos;
    const v3 pos_x = pos_o + x_arrow;
    const v3 pos_y = pos_o + y_arrow;
    const v3 pos_z = pos_o + z_arrow;
    const v3 pos_xy = pos_o + x_arrow + y_arrow;
    const v3 pos_xz = pos_o + x_arrow + z_arrow;
    const v3 pos_yz = pos_o + y_arrow + z_arrow;
    const v3 pos_xyz = pos_o + x_arrow + y_arrow + z_arrow;

    std::vector<v3> positions = {
        // back face -z
        pos_o, pos_xy, pos_x, pos_xy, pos_o, pos_y,
        // front face +z
        pos_z, pos_xz, pos_xyz, pos_xyz, pos_yz, pos_z,
        // left face -x
        pos_yz, pos_y, pos_o, pos_o, pos_z, pos_yz,
        // right face +x
        pos_xyz, pos_x, pos_xy, pos_x, pos_xyz, pos_xz,
        // bottom face -y
        pos_o, pos_x, pos_xz, pos_xz, pos_z, pos_o,
        // top face +y
        pos_y, pos_xyz, pos_xy, pos_xyz, pos_y, pos_yz,
    };

    const std::array<v3, 6> face_normals = {
        v3::NEG_Z(), v3::Z(), v3::NEG_X(), v3::X(), v3::NEG_Y(), v3::Y()
    };
    std::vector<v3> normals;
    normals.reserve(positions.size());
    for (const v3 normal : face_normals) {
        for (size_t i = 0; i < 6; ++i) { normals.push_back(normal); }
    }

    std::vector<v2> texture_coordinates;
    texture_coordinates.reserve(positions.size());
    for (size_t face = 0; face < 6; ++face) { append_face_uvs(texture_coordinates); }

    Mesh mesh;
    add_attribute(mesh, Mesh::POSITION, to_list(positions));
    add_attribute(mesh, Mesh::UV, to_list(texture_coordinates));
    add_attribute(mesh, Mesh::NORMAL, to_list(normals));
    return finish_mesh(std::move(mesh));
}

auto convert_into_data(const Mesh& mesh, const bool interleaved)
    -> Result<std::vector<f32>, MeshError> {
    auto validation = mesh.validate();
    if (validation.is_err()) { return err<std::vector<f32>, MeshError>(validation.error()); }

    const auto* position_data = mesh.attribute_data(Mesh::POSITION.m_id);
    if (position_data == nullptr) {
        return err<std::vector<f32>, MeshError>(make_error(
            MeshErrorCode::MissingPosition,
            "Mesh must contain a POSITION attribute"
        ));
    }
    const size_t vertex_count = position_data->count();

    size_t components_per_vertex = 0;
    for (const auto& [id, attribute] : mesh.m_attributes) {
        (void)id;
        const size_t components = attribute.m_attribute.count_components();
        if (components_per_vertex > std::numeric_limits<size_t>::max() - components) {
            return err<std::vector<f32>, MeshError>(make_error(
                MeshErrorCode::InvalidParameter,
                "Mesh vertex stride is too large"
            ));
        }
        components_per_vertex += components;
    }
    if (components_per_vertex == 0 ||
        vertex_count > std::numeric_limits<size_t>::max() / components_per_vertex) {
        return err<std::vector<f32>, MeshError>(make_error(
            MeshErrorCode::InvalidParameter,
            "Mesh vertex data is too large to convert"
        ));
    }

    std::vector<f32> data;
    if (interleaved) {
        data.reserve(vertex_count * components_per_vertex);
        for (size_t vertex = 0; vertex < vertex_count; ++vertex) {
            for (const auto& [id, attribute] : mesh.m_attributes) {
                (void)id;
                const size_t components = attribute.m_attribute.count_components();
                const auto   begin = attribute.m_data.begin() +
                                   static_cast<std::ptrdiff_t>(vertex * components);
                data.insert(
                    data.end(),
                    begin,
                    begin + static_cast<std::ptrdiff_t>(components)
                );
            }
        }
    } else {
        data.reserve(vertex_count * components_per_vertex);
        for (const auto& [id, attribute] : mesh.m_attributes) {
            (void)id;
            data.insert(data.end(), attribute.m_data.begin(), attribute.m_data.end());
        }
    }
    return ok<std::vector<f32>, MeshError>(std::move(data));
}

} // namespace JadeFrame
