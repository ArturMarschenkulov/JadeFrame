#pragma once
#include "JadeFrame/prelude.h"
#include "graphics_shared.h"

namespace JadeFrame {

struct ReflectedModule {

    struct Input {
        std::string name;
        u32         location;
        u32         size; // in bytes
        SHADER_TYPE type;
    };

    struct Output {
        std::string name;
        u32         location;
        u32         size; // in bytes
        SHADER_TYPE type;
    };

    struct SampledImage {
        std::string name;
        u32         binding;
        u32         set;
        u32         size;
    };

    struct UniformBuffer {
        std::string name;
        u32         size;
        u32         binding;
        u32         set;

        struct Member {
            std::string name;
            u32         offset;
            u32         size;
            SHADER_TYPE type;
        };

        std::vector<Member> members;
    };

    SHADER_STAGE m_stage;

    std::vector<Input>         m_inputs;
    std::vector<Output>        m_outputs;
    std::vector<UniformBuffer> m_uniform_buffers;
    std::vector<SampledImage>  m_sampled_images;
    // std::vector<VkPushConstantRange> m_push_constant_ranges;
};

struct ReflectedCode {
    struct Input {
        std::string name;
        u32         location;
        u32         size; // in bytes
        SHADER_TYPE type;
    };

    struct Output {
        std::string name;
        u32         location;
        u32         size; // in bytes
        SHADER_TYPE type;
    };

    struct SampledImage {
        std::string name;
        u32         binding;
        u32         set;
        u32         size;
    };

    struct UniformBuffer {
        std::string name;
        u32         size;
        u32         binding;
        u32         set;

        struct Member {
            std::string name;
            u32         offset;
            u32         size;
            SHADER_TYPE type;
        };

        std::vector<Member> members;
    };

    struct Module {
        SHADER_STAGE m_stage;

        std::vector<Input>         m_inputs;
        std::vector<Output>        m_outputs;
        std::vector<UniformBuffer> m_uniform_buffers;
        std::vector<SampledImage>  m_sampled_images;
        // std::vector<VkPushConstantRange> m_push_constant_ranges;
    };

    std::vector<Module> m_modules;
};

auto reflect(const ShadingCode& code) -> ReflectedCode;
auto reflect(const ShadingCode::Module::SPIRV& code) -> ReflectedModule;
auto convert_SPIRV_to_opengl(const ShadingCode& code) -> ShadingCode;
} // namespace JadeFrame