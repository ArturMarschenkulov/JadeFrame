#pragma once
#include "JadeFrame/prelude.h"
#include "JadeFrame/math/mat_4.h"
#include <cassert>
#include <variant>
#include <map>

namespace JadeFrame {

class Windows_Window;
class Object;
class RGBAColor;
class IWindow;

struct Image {
    Image() = default;
    ~Image();
    Image(const Image&) = delete;
    auto operator=(const Image&) -> Image& = delete;
    Image(Image&& other) noexcept;
    auto operator=(Image&& other) noexcept -> Image&;

    static auto load(const std::string& path) -> Image;

    u8* data = nullptr;
    i32 width = 0;
    i32 height = 0;
    i32 num_components = 0;
};

enum class GRAPHICS_API {
    UNDEFINED,
    OPENGL,
    VULKAN,
    D3D11,
    D3D12,
    METAL,
    SOFTWARE,
    TERMINAL,
};

inline auto to_string(GRAPHICS_API api) -> const char* {
    switch (api) {
        case GRAPHICS_API::UNDEFINED: return "UNDEFINED";
        case GRAPHICS_API::OPENGL: return "OpenGL";
        case GRAPHICS_API::VULKAN: return "Vulkan";
        case GRAPHICS_API::D3D11: return "D3D11";
        case GRAPHICS_API::D3D12: return "D3D12";
        case GRAPHICS_API::METAL: return "Metal";
        case GRAPHICS_API::SOFTWARE: return "Software";
        case GRAPHICS_API::TERMINAL: return "Terminal";
        default: assert(false); return "UNKNOWN";
    }
}
enum class SHADING_LANGUAGE {
    // High level
    GLSL,
    HLSL,
    // Low level
    SPIRV
};
enum class SHADER_STAGE {
    VERTEX,
    FRAGMENT, // PIXEL
    GEOMETRY,
    TESSELATION,
    COMPUTE,
};

enum class SHADER_TYPE {
    NONE = 0,
    // Scalars
    I32,
    U32,
    F64,
    F32,
    BOOL,

    // Vectors
    V_2_I32,
    V_3_I32,
    V_4_I32,

    V_2_F32,
    V_3_F32,
    V_4_F32,

    // Matrices, for now only nxn matrices are supported
    M_2_2_F32,
    M_3_3_F32,
    M_4_4_F32,

    // Opaque types
    SAMPLER_1D,
    SAMPLER_2D,
    SAMPLER_3D,
    SAMPLER_CUBE,
};

inline auto get_underlying_type(SHADER_TYPE type) -> SHADER_TYPE {
    SHADER_TYPE result = SHADER_TYPE::NONE;
    switch (type) {
        case SHADER_TYPE::I32:
        case SHADER_TYPE::V_2_I32:
        case SHADER_TYPE::V_3_I32:
        case SHADER_TYPE::V_4_I32: result = SHADER_TYPE::I32; break;
        case SHADER_TYPE::U32: result = SHADER_TYPE::U32; break;
        case SHADER_TYPE::F64: result = SHADER_TYPE::F64; break;
        case SHADER_TYPE::F32:
        case SHADER_TYPE::V_2_F32:
        case SHADER_TYPE::V_3_F32:
        case SHADER_TYPE::V_4_F32:
        case SHADER_TYPE::M_2_2_F32:
        case SHADER_TYPE::M_3_3_F32:
        case SHADER_TYPE::M_4_4_F32: result = SHADER_TYPE::F32; break;
        case SHADER_TYPE::BOOL: result = SHADER_TYPE::BOOL; break;
        case SHADER_TYPE::SAMPLER_1D: result = SHADER_TYPE::SAMPLER_1D; break;
        case SHADER_TYPE::SAMPLER_2D: result = SHADER_TYPE::SAMPLER_2D; break;
        case SHADER_TYPE::SAMPLER_3D: result = SHADER_TYPE::SAMPLER_3D; break;
        case SHADER_TYPE::SAMPLER_CUBE: result = SHADER_TYPE::SAMPLER_CUBE; break;
        default:
            assert(false);
            result = SHADER_TYPE::NONE;
            break;
    }
    return result;
}

inline auto is_scalar(SHADER_TYPE type) -> bool {
    switch (type) {
        case SHADER_TYPE::I32:
        case SHADER_TYPE::U32:
        case SHADER_TYPE::F64:
        case SHADER_TYPE::F32:
        case SHADER_TYPE::BOOL: return true;
        default: return false;
    }
}

inline auto get_component_count(const SHADER_TYPE type) -> u32 {
    u32 result = 0;
    switch (type) {
        case SHADER_TYPE::F32:
        case SHADER_TYPE::I32:
        case SHADER_TYPE::BOOL: result = 1; break;

        case SHADER_TYPE::V_2_F32:
        case SHADER_TYPE::V_2_I32: result = 2; break;

        case SHADER_TYPE::V_3_F32:
        case SHADER_TYPE::M_3_3_F32: // 3* float3
        case SHADER_TYPE::V_3_I32: result = 3; break;

        case SHADER_TYPE::V_4_F32:
        case SHADER_TYPE::M_4_4_F32: // 4* float4
        case SHADER_TYPE::V_4_I32: result = 4; break;

        default:
            assert(false);
            result = 0;
            break;
    }
    return result;
}

inline auto is_vector(SHADER_TYPE type) -> bool {
    switch (type) {
        case SHADER_TYPE::V_2_I32:
        case SHADER_TYPE::V_3_I32:
        case SHADER_TYPE::V_4_I32:
        case SHADER_TYPE::V_2_F32:
        case SHADER_TYPE::V_3_F32:
        case SHADER_TYPE::V_4_F32: return true;
        default: return false;
    }
}

auto to_string(SHADER_TYPE type) -> const char*;

struct VertexAttribute {
    std::string name;
    SHADER_TYPE type;
    u32         size;
    size_t      offset;
    bool        normalized;

    VertexAttribute(const std::string& name, SHADER_TYPE type, bool normalized = false);
};

class VertexFormat {
public:

public:
    VertexFormat() = default;
    VertexFormat(const VertexFormat&) = default;
    auto operator=(const VertexFormat&) -> VertexFormat& = default;
    VertexFormat(VertexFormat&&) = default;
    auto operator=(VertexFormat&&) -> VertexFormat& = default;

    static auto default_format() -> VertexFormat;

    VertexFormat(const std::initializer_list<VertexAttribute>& attributes);

    auto calculate_offset_and_stride(std::vector<VertexAttribute>& attributes) -> void;

    std::vector<VertexAttribute> m_attributes;
    u32                          m_stride = 0;
};

// This struct saves the shader code. The common language is SPIRV.
struct ShadingCode {
    struct Module {
        using SPIRV = std::vector<u32>;
        SPIRV        m_code;
        SHADER_STAGE m_stage;
    };

    SHADING_LANGUAGE    m_shading_language;
    std::vector<Module> m_modules;
};

struct TextureHandle {
public:
    TextureHandle() = default;
    ~TextureHandle();
    TextureHandle(const TextureHandle&) = delete;
    auto operator=(const TextureHandle&) -> TextureHandle& = delete;
    TextureHandle(TextureHandle&& other) noexcept;
    auto operator=(TextureHandle&& other) noexcept -> TextureHandle&;

    TextureHandle(const Image& image);

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
    ShaderHandle() = default;
    ~ShaderHandle() = default;
    ShaderHandle(const ShaderHandle&) = delete;
    auto operator=(const ShaderHandle&) -> ShaderHandle& = delete;
    ShaderHandle(ShaderHandle&& other);
    auto operator=(ShaderHandle&& other) -> ShaderHandle&;

    struct Desc {
        ShadingCode  shading_code;
        VertexFormat vertex_format;
    };

    ShaderHandle(const Desc& desc);

    auto set_uniform(const std::string& name, const void* data, size_t size) -> void;

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

// This struct saves the shader code. The common language is SPIRV.

struct ShaderModule {
    SHADING_LANGUAGE shading_language;
};

struct GPUDataMeshHandle {
    GRAPHICS_API m_api = GRAPHICS_API::UNDEFINED;
    void*        m_handle = nullptr;

    mutable bool m_is_initialized = false;
};

class RenderSystem;

class GPUBuffer {
public:
    enum TYPE {
        UNINIT, // TODO: find ways to remove it
        VERTEX,
        INDEX,
        UNIFORM,
        STAGING
    };

    GPUBuffer() = default;
    ~GPUBuffer();
    GPUBuffer(const GPUBuffer&) = delete;
    auto operator=(const GPUBuffer&) -> GPUBuffer& = delete;
    GPUBuffer(GPUBuffer&& other);
    auto operator=(GPUBuffer&& other) -> GPUBuffer&;

    GPUBuffer(RenderSystem* system, void* data, size_t size, TYPE usage);

public:
    RenderSystem* m_system = nullptr;
    GRAPHICS_API  m_api = GRAPHICS_API::UNDEFINED;
    void*         m_handle = nullptr;
    size_t        m_size = 0;
    TYPE          m_type = TYPE::UNINIT;
};
class RenderSystem;
class VertexData;

class GPUMeshData {
public:
    GPUMeshData() = default;
    ~GPUMeshData();
    GPUMeshData(const GPUMeshData&) = delete;
    auto operator=(const GPUMeshData&) -> GPUMeshData& = delete;
    GPUMeshData(GPUMeshData&& other);
    auto operator=(GPUMeshData&& other) -> GPUMeshData&;

    GPUMeshData(
        RenderSystem*      system,
        const VertexData&  vertex_data,
        const VertexFormat vertex_format,
        bool               interleaved = true
    );

public:
    GPUBuffer    m_vertex_buffer;
    GPUBuffer    m_index_buffer;
    VertexFormat m_format;
};

/*
        TODO: Consider whether this is a good way and whether it is worth it to introdcue
   inheritance. Right now, inheritance should be mainly used as a sanity check such that
   all renderers have a common interface.
*/
using ssss = const char*;
template<typename T>
concept is_renderer = requires(T& t) {
    { t.present() } -> std::same_as<void>;
    { t.clear_background() } -> std::same_as<void>;
    { t.render(std::declval<Matrix4x4>()) } -> std::same_as<void>;
    { t.submit(std::declval<Object>()) } -> std::same_as<void>;
    { t.set_clear_color(std::declval<RGBAColor>()) } -> std::same_as<void>;
    { t.set_viewport(u32{}, u32{}, u32{}, u32{}) } -> std::same_as<void>;
    { t.take_screenshot(std::declval<char*>()) } -> std::same_as<void>;
};

class IRenderer {
public:
    IRenderer() = default;
    IRenderer(const IRenderer&) = delete;
    auto operator=(const IRenderer&) -> IRenderer& = delete;
    IRenderer(IRenderer&&) = default;
    auto operator=(IRenderer&&) -> IRenderer& = delete;
    virtual ~IRenderer() = default;

    // client stuff
    virtual auto submit(const Object& obj) -> void = 0;
    virtual auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void = 0;

    virtual auto take_screenshot(const char* filename) -> void = 0;

public: // more internal stuff
    virtual auto set_clear_color(const RGBAColor& color) -> void = 0;
    virtual auto clear_background() -> void = 0;
    virtual auto render(const Matrix4x4& view_projection) -> void = 0;
    virtual auto present() -> void = 0;
};

inline auto SHADER_TYPE_get_size(const SHADER_TYPE type) -> u32 {
    u32 result;
    switch (type) {
        case SHADER_TYPE::F32: result = 4; break;
        case SHADER_TYPE::V_2_F32: result = 4 * 2; break;
        case SHADER_TYPE::V_3_F32: result = 4 * 3; break;
        case SHADER_TYPE::V_4_F32: result = 4 * 4; break;
        case SHADER_TYPE::M_3_3_F32: result = 4 * 3 * 3; break;
        case SHADER_TYPE::M_4_4_F32: result = 4 * 4 * 4; break;
        case SHADER_TYPE::I32: result = 4; break;
        case SHADER_TYPE::V_2_I32: result = 4 * 2; break;
        case SHADER_TYPE::V_3_I32: result = 4 * 3; break;
        case SHADER_TYPE::V_4_I32: result = 4 * 4; break;
        case SHADER_TYPE::BOOL: result = 1; break;
        default:
            assert(false);
            result = 0;
            break;
    }
    return result;
}

class IShader {
public:
    struct Desc {
        SHADING_LANGUAGE shading_language;
        ShadingCode      code;
        VertexFormat     vertex_format;
    };

public:
};

struct MaterialHandle;
struct TextureHandle;
struct ShaderHandle;
class VertexData;

class Object {
public:
    u32            m_vertex_data_id;
    VertexData*    m_vertex_data;
    VertexFormat   m_vertex_format;
    MaterialHandle m_material_handle;
    Matrix4x4      m_transform;
};

class RenderSystem {
public:
    RenderSystem() = default;
    ~RenderSystem();
    RenderSystem(const RenderSystem&) = delete;
    auto operator=(const RenderSystem&) -> RenderSystem& = delete;
    RenderSystem(RenderSystem&&) noexcept;
    auto operator=(RenderSystem&&) noexcept -> RenderSystem&;

    RenderSystem(GRAPHICS_API api, IWindow* window);

    auto init(GRAPHICS_API api, IWindow* window) -> void;

    auto register_texture(TextureHandle&& handle) -> u32;
    auto register_shader(const ShaderHandle::Desc& handle) -> u32;
    auto register_mesh(const VertexFormat& format, const VertexData& data) -> u32;

public:
    GRAPHICS_API m_api = GRAPHICS_API::UNDEFINED;
    IRenderer*   m_renderer = nullptr;

    // NOTE: For now the key/id 0 refers to no texture or default texture
    //     This is a bit of a hack, but it works for now.
    std::map<u32, TextureHandle> m_registered_textures;
    std::map<u32, ShaderHandle>  m_registered_shaders;
};

} // namespace JadeFrame