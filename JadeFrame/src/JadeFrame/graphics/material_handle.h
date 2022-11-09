#pragma once
#include "JadeFrame/prelude.h"

#include "shader_loader.h"
#include "graphics_shared.h"
#include <string>

namespace JadeFrame {

struct TextureHandle {
public:
    TextureHandle() = default;
    ~TextureHandle();

    TextureHandle(const TextureHandle&) = delete;
    auto operator=(const TextureHandle&) -> TextureHandle& = delete;

    TextureHandle(TextureHandle&& other);
    auto operator=(TextureHandle&& other) -> TextureHandle&;

    TextureHandle(const std::string& path);

    auto init(void* context) -> void;

public:
    u8*   m_data;
    v2u32 m_size;
    u32   m_num_components;

    GRAPHICS_API m_api = GRAPHICS_API::UNDEFINED;
    void*        m_handle = nullptr;
};
struct ShaderHandle {
public:
    struct DESC {
        ShadingCode  shading_code;
        VertexFormat vertex_format;
    };

    ShaderHandle() = default;
    
    ShaderHandle(ShaderHandle&& other);
    auto operator=(ShaderHandle&& other) -> ShaderHandle&;

    ShaderHandle(const DESC& desc);

    auto init(void* context) -> void;

public:
    ShadingCode  m_code;
    VertexFormat m_vertex_format;

    GRAPHICS_API m_api = GRAPHICS_API::UNDEFINED;
    void*        m_handle = nullptr;
};
struct MaterialHandle {
    u32 m_shader_id = 0;
    u32 m_texture_id = 0;
};

} // namespace JadeFrame
