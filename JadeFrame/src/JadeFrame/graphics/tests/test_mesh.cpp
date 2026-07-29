#include <gtest/gtest.h>

#include "JadeFrame/graphics/mesh.h"

using namespace JadeFrame;

namespace {

auto make_two_vertex_mesh() -> Mesh {
    Mesh mesh;
    mesh.m_topology = PRIMITIVE_TOPOLOGY::LINE_LIST;
    mesh.m_attributes.emplace(
        Mesh::POSITION.m_id,
        Mesh::AttributeData{Mesh::POSITION, {0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F}}
    );
    return mesh;
}

} // namespace

TEST(MeshValidation, MissingPositionIsAnError) {
    const Mesh mesh;

    auto result = convert_into_data(mesh, true);

    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, MeshErrorCode::MissingPosition);
}

TEST(MeshValidation, MismatchedAttributeCountsAreAnError) {
    Mesh mesh = make_two_vertex_mesh();
    mesh.m_attributes.emplace(
        Mesh::UV.m_id,
        Mesh::AttributeData{Mesh::UV, {0.0F, 0.0F}}
    );

    auto result = convert_into_data(mesh, true);

    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, MeshErrorCode::AttributeVertexCountMismatch);
}

TEST(MeshValidation, UnsupportedAttributeFormatsAreAnError) {
    Mesh mesh = make_two_vertex_mesh();
    mesh.m_attributes.emplace(
        7,
        Mesh::AttributeData{
            Mesh::VertexAttribute{"CUSTOM", 7, SHADER_TYPE::F64},
            {1.0F, 2.0F}
        }
    );

    auto result = mesh.validate();

    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, MeshErrorCode::UnsupportedAttributeFormat);
}

TEST(MeshBuilder, RejectsPositionlessDescriptorsBeforeBuilding) {
    MeshBuilder::Desc desc;
    desc.has_position = false;

    auto result = MeshBuilder::rectangle(v3::zero(), v3::one(), desc);

    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, MeshErrorCode::InvalidDescriptor);
}

TEST(MeshValidation, InvalidIndicesAreRejected) {
    Mesh mesh = make_two_vertex_mesh();
    mesh.m_topology = PRIMITIVE_TOPOLOGY::POINT_LIST;
    mesh.m_indices = {0, 2};

    auto result = convert_into_data(mesh, false);

    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, MeshErrorCode::InvalidIndex);
}

TEST(MeshValidation, InsertionFailureDoesNotModifyTheMesh) {
    Mesh mesh = make_two_vertex_mesh();
    const auto original_attribute_count = mesh.m_attributes.size();

    auto result = mesh.with_inserted_attribute(Mesh::UV, {0.0F, 0.0F});

    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, MeshErrorCode::AttributeVertexCountMismatch);
    EXPECT_EQ(mesh.m_attributes.size(), original_attribute_count);
    EXPECT_FALSE(mesh.contains_attribute(Mesh::UV.m_id));
}

TEST(MeshBuilder, RejectsZeroCircleSegments) {
    auto result = MeshBuilder::circle(v3::zero(), 1.0F, 0);

    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, MeshErrorCode::InvalidSegmentCount);
}

TEST(MeshBuilder, GeneratedMeshesHaveCompleteUvData) {
    auto rectangle_result = MeshBuilder::rectangle(
        v3::zero(),
        v3::one(),
        MeshBuilder::Desc{.has_position = true,
                          .has_texture_coordinates = true,
                          .has_indices = false,
                          .has_normals = true}
    );
    ASSERT_TRUE(rectangle_result.is_ok());
    const Mesh& rectangle = rectangle_result.value();
    EXPECT_EQ(rectangle.attribute_data(Mesh::POSITION.m_id)->count(), 6);
    EXPECT_EQ(rectangle.attribute_data(Mesh::UV.m_id)->count(), 6);
    EXPECT_EQ(rectangle.attribute_data(Mesh::NORMAL.m_id)->count(), 6);

    auto cube_result = MeshBuilder::cube(v3::zero(), v3::one());
    ASSERT_TRUE(cube_result.is_ok());
    const Mesh& cube = cube_result.value();
    EXPECT_EQ(cube.attribute_data(Mesh::POSITION.m_id)->count(), 36);
    EXPECT_EQ(cube.attribute_data(Mesh::UV.m_id)->count(), 36);
    EXPECT_EQ(cube.attribute_data(Mesh::NORMAL.m_id)->count(), 36);
}

TEST(MeshConversion, SupportsInterleavedAndNonInterleavedLayouts) {
    Mesh mesh = make_two_vertex_mesh();
    mesh.m_attributes.emplace(
        Mesh::UV.m_id,
        Mesh::AttributeData{Mesh::UV, {0.0F, 0.0F, 1.0F, 1.0F}}
    );

    auto interleaved_result = convert_into_data(mesh, true);
    ASSERT_TRUE(interleaved_result.is_ok());
    EXPECT_EQ(
        interleaved_result.value(),
        (std::vector<f32>{0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 1.0F, 1.0F})
    );

    auto non_interleaved_result = convert_into_data(mesh, false);
    ASSERT_TRUE(non_interleaved_result.is_ok());
    EXPECT_EQ(
        non_interleaved_result.value(),
        (std::vector<f32>{0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F})
    );
}
