#pragma once
#include "JadeFrame/prelude.h"
#include "graphics_shared.h"
#include <span>

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
    static auto reflect(const ShadingCode::Module::SPIRV& code, SHADER_STAGE stage)
        -> ReflectedModule;
    static auto into_interface(const std::span<const ReflectedModule>& modules)
        -> ReflectedModule;
    [[nodiscard]] auto get_vertex_format() const -> VertexFormat;
};

struct ReflectedCode {
    std::vector<ReflectedModule> m_modules;
};

auto reflect(const ShadingCode& code) -> ReflectedCode;
} // namespace JadeFrame