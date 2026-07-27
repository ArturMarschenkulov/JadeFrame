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
