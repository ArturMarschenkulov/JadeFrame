#include <gtest/gtest.h>

#include <array>

#include "JadeFrame/graphics/reflect.h"

using namespace JadeFrame;

TEST(Reflection, MergesPipelineInterfaceAndPreservesLocations) {
    ReflectedModule vertex = {};
    vertex.m_stage = SHADER_STAGE::VERTEX;
    vertex.m_inputs.push_back(
        ReflectedModule::Input{
            .name = "position",
            .location = 4,
            .size = 12,
            .type = SHADER_TYPE::V_3_F32,
        }
    );
    vertex.m_outputs.push_back(
        ReflectedModule::Output{
            .name = "color",
            .location = 0,
            .size = 16,
            .type = SHADER_TYPE::V_4_F32,
        }
    );

    ReflectedModule::UniformBuffer camera = {};
    camera.name = "Camera";
    camera.size = 128;
    camera.binding = 0;
    camera.set = 0;
    camera.descriptor_count = 1;
    camera.members.push_back(
        ReflectedModule::UniformBuffer::Member{
            .name = "view",
            .offset = 0,
            .size = 64,
            .type = SHADER_TYPE::M_4_4_F32,
        }
    );
    vertex.m_uniform_buffers.push_back(camera);

    ReflectedModule fragment = {};
    fragment.m_stage = SHADER_STAGE::FRAGMENT;
    fragment.m_inputs.push_back(
        ReflectedModule::Input{
            .name = "color",
            .location = 0,
            .size = 16,
            .type = SHADER_TYPE::V_4_F32,
        }
    );
    fragment.m_uniform_buffers.push_back(camera);
    fragment.m_sampled_images.push_back(
        ReflectedModule::SampledImage{
            .name = "albedo",
            .binding = 1,
            .set = 2,
            .size = 0,
            .descriptor_count = 1,
        }
    );

    const std::array<ReflectedModule, 2> modules = {vertex, fragment};
    const ReflectedModule interface = ReflectedModule::into_interface(modules);

    ASSERT_EQ(interface.m_inputs.size(), 1);
    EXPECT_EQ(interface.m_inputs[0].location, 4);

    const VertexFormat vertex_format = interface.get_vertex_format();
    ASSERT_EQ(vertex_format.m_attributes.size(), 1);
    EXPECT_EQ(vertex_format.m_attributes[0].location, 4);
    EXPECT_EQ(vertex_format.m_attributes[0].type, SHADER_TYPE::V_3_F32);

    ASSERT_EQ(interface.m_uniform_buffers.size(), 1);
    EXPECT_EQ(interface.m_uniform_buffers[0].size, 128);
    ASSERT_EQ(interface.m_sampled_images.size(), 1);
    EXPECT_EQ(interface.m_sampled_images[0].set, 2);
}

TEST(Reflection, PipelineOutputsComeOnlyFromFragmentStage) {
    ReflectedModule vertex = {};
    vertex.m_stage = SHADER_STAGE::VERTEX;
    vertex.m_outputs = {
        {.name = "uv", .location = 0, .size = 8, .type = SHADER_TYPE::V_2_F32},
        {.name = "normal", .location = 3, .size = 12, .type = SHADER_TYPE::V_3_F32},
    };

    ReflectedModule fragment = {};
    fragment.m_stage = SHADER_STAGE::FRAGMENT;
    fragment.m_inputs = {
        {.name = "uv", .location = 0, .size = 8, .type = SHADER_TYPE::V_2_F32},
    };
    fragment.m_outputs = {
        {.name = "color", .location = 0, .size = 16, .type = SHADER_TYPE::V_4_F32},
    };

    // Module order must not decide whether location 0 means UV or color.
    for (const auto& modules : {
             std::array<ReflectedModule, 2>{vertex, fragment},
             std::array<ReflectedModule, 2>{fragment, vertex}}) {
        const auto interface = ReflectedModule::into_interface(modules);
        ASSERT_EQ(interface.m_outputs.size(), 1);
        EXPECT_EQ(interface.m_outputs[0].name, "color");
        EXPECT_EQ(interface.m_outputs[0].location, 0);
        EXPECT_EQ(interface.m_outputs[0].size, 16);
        EXPECT_EQ(interface.m_outputs[0].type, SHADER_TYPE::V_4_F32);
    }

    const std::array<ReflectedModule, 1> vertex_only = {vertex};
    EXPECT_TRUE(ReflectedModule::into_interface(vertex_only).m_outputs.empty());
}
