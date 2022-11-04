#pragma once
#include "JadeFrame/prelude.h"

#include "shader_loader.h"
#include "graphics_shared.h"
#include <string>

namespace JadeFrame {

struct TextureHandle {
public:
    TextureHandle(const TextureHandle&) = delete;
    auto operator=(const TextureHandle&) -> TextureHandle& = delete;
    auto operator=(TextureHandle&&) -> TextureHandle& = delete;

    TextureHandle() = default;
    TextureHandle(TextureHandle&&) = default;
    TextureHandle(const std::string& path);

    ~TextureHandle();

    auto init(void* context) -> void;

public:
    u8*   m_data;
    v2i32 m_size;
    i32   m_num_components;

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
    ShaderHandle(const DESC& desc);
    auto init(void* context) -> void;

public:
    ShadingCode  m_code;
    VertexFormat m_vertex_format;

    GRAPHICS_API m_api = GRAPHICS_API::UNDEFINED;
    void*        m_handle = nullptr;
};
struct MaterialHandle {
    ShaderHandle*  m_shader_handle;
    TextureHandle* m_texture_handle;

    auto init(void* context) const -> void;

    bool m_is_initialized = false;
};

} // namespace JadeFrame
