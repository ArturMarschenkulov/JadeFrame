#pragma once
#include <cassert>
#include <deque>
#include <memory>
#include <string>

#include "camera.h"
#include "JadeFrame/utils/result.h"

namespace JadeFrame {

class Windows_Window;
class Object;
class RGBAColor;
class Window;
class IRenderer;

struct Image {
    Image() = default;
    ~Image() = default;
    Image(const Image&) = delete;
    auto operator=(const Image&) -> Image& = delete;
    Image(Image&& other) noexcept;
    auto operator=(Image&& other) noexcept -> Image&;

    static auto load_from_path(const std::string& path) -> Image;

    std::vector<u8> data;
    i32             width = 0;
    i32             height = 0;
    i32             num_components = 0;
};

/// Writes `image` as an 8-bit PNG file.
///
/// `Image` rows are stored bottom-to-top (see `Image::load_from_path`), the written file
/// has the usual top-to-bottom orientation. Returns false if the image is empty, has an
/// unsupported component count, or the file could not be written.
[[nodiscard]] auto write_png(const Image& image, const char* path) -> bool;

enum class RendererErrorCode : u8 {
    /// The backend does not provide the operation at all.
    UNSUPPORTED,
    /// The renderer or one of the involved resources is not in a usable state.
    INVALID_STATE,
    /// The operation needs a rendered, not yet presented frame and there is none.
    FRAME_NOT_AVAILABLE,
    /// The graphics API reported a failure.
    BACKEND_FAILURE,
    /// Reading or writing a file failed.
    IO_FAILURE,
};

struct RendererError {
    RendererErrorCode m_code;
    std::string       m_message;
};

inline auto to_string(RendererErrorCode code) -> const char* {
    switch (code) {
        case RendererErrorCode::UNSUPPORTED: return "UNSUPPORTED";
        case RendererErrorCode::INVALID_STATE: return "INVALID_STATE";
        case RendererErrorCode::FRAME_NOT_AVAILABLE: return "FRAME_NOT_AVAILABLE";
        case RendererErrorCode::BACKEND_FAILURE: return "BACKEND_FAILURE";
        case RendererErrorCode::IO_FAILURE: return "IO_FAILURE";
        default: assert(false); return "UNKNOWN";
    }
}

/// Validates the packed RGB8/RGBA8 upload contract without touching a GPU.
[[nodiscard]] auto validate_texture_image(const Image& image)
    -> Result<void, RendererError>;

enum class GRAPHICS_API : u8 {
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

/// Why bringing up a window, a graphics backend or the application failed.
enum class InitErrorCode : u8 {
    /// The caller passed something initialization cannot work with, e.g. no window.
    INVALID_ARGUMENT,
    /// An operation that needs a successful initialization was called without one.
    NOT_INITIALIZED,
    /// The platform window could not be created.
    WINDOW_CREATION_FAILED,
    /// JadeFrame has no renderer for the requested graphics API.
    BACKEND_UNSUPPORTED,
    /// The renderer for the requested graphics API was disabled at build time.
    BACKEND_DISABLED,
    /// The graphics context (OpenGL context; Vulkan instance, surface or device) could
    /// not be created or does not meet the requirements of the renderer.
    CONTEXT_CREATION_FAILED,
    /// The context exists, but a resource the renderer cannot work without
    /// (framebuffer, swapchain, internal shader, ...) could not be created.
    RESOURCE_CREATION_FAILED,
};

/// An initialization failure. Initialization never falls back to another backend, so
/// `m_api` is always the API that was explicitly requested (or `UNDEFINED` if the
/// failure is not related to a graphics API).
struct InitError {
    InitErrorCode m_code;
    GRAPHICS_API  m_api = GRAPHICS_API::UNDEFINED;
    /// What failed and, where the platform or graphics API reported one, why.
    std::string m_message;
};

inline auto to_string(InitErrorCode code) -> const char* {
    switch (code) {
        case InitErrorCode::INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case InitErrorCode::NOT_INITIALIZED: return "NOT_INITIALIZED";
        case InitErrorCode::WINDOW_CREATION_FAILED: return "WINDOW_CREATION_FAILED";
        case InitErrorCode::BACKEND_UNSUPPORTED: return "BACKEND_UNSUPPORTED";
        case InitErrorCode::BACKEND_DISABLED: return "BACKEND_DISABLED";
        case InitErrorCode::CONTEXT_CREATION_FAILED: return "CONTEXT_CREATION_FAILED";
        case InitErrorCode::RESOURCE_CREATION_FAILED: return "RESOURCE_CREATION_FAILED";
    }
    return "UNKNOWN";
}

/// One line for logs and error output: "<CODE> [<api>]: <message>".
auto to_string(const InitError& error) -> std::string;

// NOTE: Since GLSL and SPIRV are slightly different for both opengl and vulkan, they are
// treated like different shading languages here.
enum class SHADING_LANGUAGE : u8 {
    // High level
    GLSL_VULKAN,
    GLSL_OPENGL,
    HLSL,
    // Low level
    SPIRV_VULKAN,
    SPIRV_OPENGL,
};
enum class SHADER_STAGE : u8 {
    VERTEX,
    FRAGMENT, // PIXEL
    GEOMETRY,
    TESSELATION,
    COMPUTE,
};

inline auto to_string(SHADER_STAGE stage) -> const char* {
    switch (stage) {
        case SHADER_STAGE::VERTEX: return "VERTEX";
        case SHADER_STAGE::FRAGMENT: return "FRAGMENT";
        case SHADER_STAGE::GEOMETRY: return "GEOMETRY";
        case SHADER_STAGE::TESSELATION: return "TESSELATION";
        case SHADER_STAGE::COMPUTE: return "COMPUTE";
        default: assert(false); return "UNKNOWN";
    }
}

enum class SHADER_TYPE : u8 {
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

inline auto component_count(const SHADER_TYPE type) -> u32 {
    u32 result = 0;
    switch (type) {
        case SHADER_TYPE::F32:
        case SHADER_TYPE::I32:
        case SHADER_TYPE::U32:
        case SHADER_TYPE::BOOL: result = 1; break;

        case SHADER_TYPE::V_2_F32:
        case SHADER_TYPE::V_2_I32: result = 2; break;

        case SHADER_TYPE::V_3_F32:
        case SHADER_TYPE::V_3_I32: result = 3; break;

        case SHADER_TYPE::V_4_F32:
        case SHADER_TYPE::V_4_I32: result = 4; break;

        case SHADER_TYPE::M_2_2_F32: result = 4; break;
        case SHADER_TYPE::M_3_3_F32: result = 9; break;
        case SHADER_TYPE::M_4_4_F32: result = 16; break;

        default:
            result = 0;
            break;
    }
    return result;
}

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

    assert(component_count(result) == 1);
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

/// A no-op handle borrows a context-owned object; it must never delete it.
inline auto noop_native_handle_deleter(void* /*unused*/) noexcept -> void {}

using NativeHandle = std::unique_ptr<void, decltype(&noop_native_handle_deleter)>;

struct VertexAttribute {
    SHADER_TYPE type = SHADER_TYPE::NONE;
    u32         location = 0;
};

class VertexFormat {
public:

public:
    VertexFormat() = default;
    VertexFormat(const VertexFormat&) = default;
    auto operator=(const VertexFormat&) -> VertexFormat& = default;
    VertexFormat(VertexFormat&&) = default;
    auto operator=(VertexFormat&&) -> VertexFormat& = default;
    ~VertexFormat() = default;

    explicit VertexFormat(const std::vector<VertexAttribute>& attributes);

    std::vector<VertexAttribute> m_attributes;
    u32                          m_stride = 0;
};

// This struct saves the shader code. The common language is SPIRV.
enum class ShaderErrorCode : u8 {
    /// No built-in shader is registered under the requested name.
    UNKNOWN_SHADER_NAME,
    /// The shader stage cannot be compiled by this build.
    UNSUPPORTED_STAGE,
    /// There is no compilation target for the requested graphics API.
    UNSUPPORTED_API,
    /// The GLSL source was rejected by the compiler.
    COMPILATION_FAILED,
    /// The SPIR-V module is malformed or uses something the engine does not support.
    INVALID_SPIRV,
};

struct ShaderError {
    ShaderErrorCode m_code;
    /// Human readable description, including the compiler's diagnostics if any.
    std::string m_message;
};

inline auto to_string(ShaderErrorCode code) -> const char* {
    switch (code) {
        case ShaderErrorCode::UNKNOWN_SHADER_NAME: return "UNKNOWN_SHADER_NAME";
        case ShaderErrorCode::UNSUPPORTED_STAGE: return "UNSUPPORTED_STAGE";
        case ShaderErrorCode::UNSUPPORTED_API: return "UNSUPPORTED_API";
        case ShaderErrorCode::COMPILATION_FAILED: return "COMPILATION_FAILED";
        case ShaderErrorCode::INVALID_SPIRV: return "INVALID_SPIRV";
    }
    return "UNKNOWN";
}

struct ShadingCode {
    struct Module {
        using SPIRV = std::vector<u32>;
        SPIRV        m_code;
        SHADER_STAGE m_stage;
    };

    SHADING_LANGUAGE    m_shading_language = SHADING_LANGUAGE::SPIRV_VULKAN;
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

    explicit TextureHandle(const Image& image);

public:
    v2u32 m_size;
    u32   m_num_components = 0;

    GRAPHICS_API m_api = GRAPHICS_API::UNDEFINED;
    NativeHandle m_handle = {nullptr, noop_native_handle_deleter};

private:
    auto release() -> void;
};

struct ShaderHandle {
public:
    ShaderHandle() = default;
    ~ShaderHandle();
    ShaderHandle(const ShaderHandle&) = delete;
    auto operator=(const ShaderHandle&) -> ShaderHandle& = delete;
    ShaderHandle(ShaderHandle&& other) noexcept;
    auto operator=(ShaderHandle&& other) noexcept -> ShaderHandle&;

    struct Desc {
        ShadingCode shading_code;
    };

    explicit ShaderHandle(const Desc& desc);

    /// Forwards to `IRenderer::set_shader_uniform`, see there for the contract. Fails
    /// with `INVALID_STATE` if the shader is not registered with a renderer.
    auto set_uniform(const std::string& name, const void* data, size_t size)
        -> Result<void, RendererError>;

public:
    ShadingCode m_code;

    GRAPHICS_API m_api = GRAPHICS_API::UNDEFINED;
    NativeHandle m_handle = {nullptr, noop_native_handle_deleter};
    IRenderer*   m_renderer = nullptr;

private:
    auto release() -> void;
};

struct MaterialInfo {
    struct BindGroup {
        std::string m_name;
        u32         m_set;
        u32         m_binding;
    };

    std::string            m_name;
    std::vector<BindGroup> m_groups;

    static auto default_0() -> MaterialInfo {
        auto camera = BindGroup{
            .m_name = "Camera",
            .m_set = 0,
            .m_binding = 0,
        };
        auto transform = BindGroup{
            .m_name = "Transform",
            .m_set = 3,
            .m_binding = 0,
        };

        std::vector<BindGroup> groups;
        groups.push_back(camera);
        groups.push_back(transform);

        auto material_info =
            MaterialInfo{.m_name = "Default_0", .m_groups = std::move(groups)};
        return material_info;
    }

    [[nodiscard]] auto get_bind_group_by_name(const std::string& name) const
        -> const BindGroup* {
        for (const auto& group : m_groups) {
            if (group.m_name == name) { return &group; }
        }
        return nullptr;
    }
};

struct MaterialHandle {
    MaterialHandle() = default;
    ~MaterialHandle();
    MaterialHandle(const MaterialHandle&) = delete;
    auto operator=(const MaterialHandle&) -> MaterialHandle& = delete;
    MaterialHandle(MaterialHandle&& other) noexcept;
    auto operator=(MaterialHandle&& other) noexcept -> MaterialHandle&;

    ShaderHandle*  m_shader = nullptr;
    TextureHandle* m_texture = nullptr;
    /// Native handle
    NativeHandle m_handle = {nullptr, noop_native_handle_deleter};
    MaterialInfo m_info;
    GRAPHICS_API m_api = GRAPHICS_API::UNDEFINED;

private:
    auto release() -> void;
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
    enum TYPE : u8 {
        VERTEX,
        INDEX,
        UNIFORM,
        STAGING
    };

    GPUBuffer() = default;
    ~GPUBuffer();
    GPUBuffer(const GPUBuffer&) = delete;
    auto operator=(const GPUBuffer&) -> GPUBuffer& = delete;
    GPUBuffer(GPUBuffer&& other) noexcept;
    auto operator=(GPUBuffer&& other) noexcept -> GPUBuffer&;

    GPUBuffer(RenderSystem* system, void* data, size_t size, TYPE usage);

public:
    IRenderer*    m_renderer = nullptr;
    // Native buffers are tracked by the context, which releases any survivors at shutdown.
    std::weak_ptr<void> m_renderer_lifetime;
    GRAPHICS_API  m_api = GRAPHICS_API::UNDEFINED;
    void*         m_handle = nullptr;
    size_t        m_size = 0;
    TYPE          m_type = TYPE::VERTEX;

private:
    auto release() -> void;
};
class RenderSystem;
class Mesh;

/// Owns uploaded buffers and a snapshot of their layout and draw counts. No CPU
/// geometry is retained. Registered meshes belong to RenderSystem until shutdown;
/// do not move them or replace their buffers while objects/commands borrow them.
class GPUMeshData {
public:
    GPUMeshData() = default;
    ~GPUMeshData();
    GPUMeshData(const GPUMeshData&) = delete;
    auto operator=(const GPUMeshData&) -> GPUMeshData& = delete;
    GPUMeshData(GPUMeshData&& other) noexcept;
    auto operator=(GPUMeshData&& other) noexcept -> GPUMeshData&;

    GPUMeshData(RenderSystem* system, const Mesh& vertex_data, bool interleaved = true);

    [[nodiscard]] auto vertex_count() const -> u32 { return m_vertex_count; }
    [[nodiscard]] auto index_count() const -> u32 { return m_index_count; }
    /// Attributes are packed in Mesh attribute order at consecutive shader locations.
    /// The shader/pipeline must use the same format, as with the existing upload API.
    [[nodiscard]] auto vertex_format() const -> const VertexFormat& { return m_vertex_format; }
    [[nodiscard]] auto is_interleaved() const -> bool { return m_interleaved; }

public:
    std::unique_ptr<GPUBuffer> m_vertex_buffer;
    std::unique_ptr<GPUBuffer> m_index_buffer;

private:
    friend class RenderSystem;
    /// Called only after convert_into_data has validated the CPU geometry.
    auto cache_draw_data(const Mesh& mesh, bool interleaved) -> Result<void, RendererError>;
    VertexFormat m_vertex_format;
    u32 m_vertex_count = 0;
    u32 m_index_count = 0;
    bool m_interleaved = true;
};
class Mesh;

struct RenderCommand {
    mat4x4          transform = {};
    MaterialHandle* material = nullptr;
    /// Borrows registry resources through render(); shutdown clears commands before
    /// releasing resources and waits for in-flight GPU work before destroying them.
    const GPUMeshData* m_mesh = nullptr;
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
    { t.render(std::declval<Camera>()) } -> std::same_as<void>;
    { t.set_clear_color(std::declval<RGBAColor>()) } -> std::same_as<void>;
    { t.set_viewport(u32{}, u32{}, u32{}, u32{}) } -> std::same_as<void>;
    {
        t.take_screenshot(std::declval<const char*>())
    } -> std::same_as<Result<Image, RendererError>>;
    { t.wait_until_idle() } -> std::same_as<void>;
};

/// The common renderer contract.
///
/// Every operation declared here either behaves as documented on every backend or
/// reports a `RendererError`. A backend must never accept a request it does not carry
/// out.
///
/// Frame protocol, per frame and in this order:
///   `clear_background()` -> `render()` -> `present()`
/// A frame is "rendered" from the end of a successful `render()` until `present()`.
class IRenderer {
public:
    IRenderer() = default;
    IRenderer(const IRenderer&) = delete;
    auto operator=(const IRenderer&) -> IRenderer& = delete;
    IRenderer(IRenderer&&) = default;
    auto operator=(IRenderer&&) -> IRenderer& = delete;
    virtual ~IRenderer() = default;

    [[nodiscard]] auto resource_lifetime() const -> std::weak_ptr<void> {
        return m_resource_lifetime;
    }

private:
    std::shared_ptr<void> m_resource_lifetime = std::make_shared<int>(0);

public:
    // Resource operations are deliberately expressed in terms of façade types. The
    // concrete backend owns the native casts and lifetime rules behind this interface.

    /// Second construction phase: creates the graphics context and every resource
    /// rendering needs. Constructors only store their arguments; they cannot fail and
    /// do not touch the graphics API.
    ///
    /// Called exactly once, by `RenderSystem`, before anything else. On success every
    /// other operation of this interface may be used. On failure the renderer is not
    /// usable: the only valid operation left is its destruction, which releases
    /// whatever `initialize()` had created up to the failing step.
    [[nodiscard]] virtual auto initialize() -> Result<void, InitError> = 0;

    /// Returns nullptr on failure. A non-null handle must be released with
    /// `destroy_buffer` while the renderer is alive.
    virtual auto create_buffer(
        GPUBuffer::TYPE type,
        const void*     data,
        size_t          size
    ) -> void* = 0;
    /// Releases the GPU buffer behind `handle`. nullptr is ignored.
    virtual auto destroy_buffer(void* handle) -> void = 0;
    /// Returns a null handle on failure.
    virtual auto create_texture(const Image& image) -> NativeHandle = 0;
    /// Returns a null handle if the code cannot be turned into a usable shader.
    virtual auto create_shader(const ShadingCode& code) -> NativeHandle = 0;

    /// Backends can preserve compiler diagnostics while older implementations keep
    /// the null-handle API. Registration uses this result-returning boundary.
    virtual auto try_create_shader(const ShadingCode& code)
        -> Result<NativeHandle, RendererError> {
        auto handle = create_shader(code);
        if (!handle) {
            return err<NativeHandle, RendererError>(RendererError{
                RendererErrorCode::BACKEND_FAILURE,
                "Backend failed to compile or link shader",
            });
        }
        return ok<NativeHandle, RendererError>(std::move(handle));
    }
    /// Returns a null handle on failure.
    virtual auto create_material(
        const ShaderHandle&  shader,
        const TextureHandle* texture
    ) -> NativeHandle = 0;

    /// Uploads the value of the uniform `name` of `shader`.
    ///
    /// No backend supports this at the moment and every call fails with
    /// `UNSUPPORTED`: shaders are SPIR-V whose uniforms live in uniform blocks, and the
    /// buffers backing those blocks are owned per material (and filled by `render()`),
    /// not per shader. There is no per-shader storage a named value could be written
    /// to. The operation is kept so that callers get an explicit answer.
    [[nodiscard]] virtual auto set_shader_uniform(
        void*              shader,
        const std::string& name,
        const void*        data,
        size_t             size
    ) -> Result<void, RendererError> = 0;

    // client stuff

    /// Sets the rectangle of the window's framebuffer that normalized device coordinates
    /// are mapped to by the following `render()` calls.
    ///
    /// The rectangle is given in pixels with the origin in the lower-left corner and y
    /// pointing up. The initial viewport is the full window as it was when the renderer
    /// was created; it does not follow window resizes on its own. Nothing is drawn
    /// outside of the viewport, the rest of the framebuffer shows the clear color. A
    /// rectangle that reaches beyond the framebuffer is allowed and clipped, an empty
    /// one draws nothing.
    virtual auto set_viewport(u32 x, u32 y, u32 width, u32 height) -> void = 0;

    /// Reads back the current viewport rectangle (clipped to the framebuffer) of the
    /// rendered frame.
    ///
    /// Precondition: must be called after `render()` and before `present()`, anything
    /// else fails with `FRAME_NOT_AVAILABLE`. On backends that draw directly into the
    /// window's back buffer the result also contains what was drawn on top after
    /// `render()`.
    ///
    /// The image is owned by the caller, tightly packed with 3 components (RGB, 8 bit
    /// each) and its rows are ordered bottom-to-top like `Image::load_from_path`. The
    /// values are the display-encoded ones stored in the framebuffer. The call blocks
    /// until the frame is finished on the GPU.
    ///
    /// If `filename` is neither null nor empty the image is additionally written to that
    /// path as PNG; a failed write is reported as `IO_FAILURE`.
    [[nodiscard]] virtual auto take_screenshot(const char* filename)
        -> Result<Image, RendererError> = 0;

public: // more internal stuff
    virtual auto set_clear_color(const RGBAColor& color) -> void = 0;
    /// Clears the currently bound framebuffer to the clear color. Backends that can only
    /// clear as part of `render()` (Vulkan clears when its render pass begins) have
    /// nothing to do here; either way the frame produced by `render()` starts cleared.
    virtual auto clear_background() -> void = 0;
    /// Draws and consumes the render commands submitted to the `RenderSystem`.
    virtual auto render(const Camera& cam) -> void = 0;
    virtual auto present() -> void = 0;
    /// Blocks until the GPU has finished all work submitted so far.
    virtual auto wait_until_idle() -> void = 0;
};

inline auto get_size(const SHADER_TYPE type) -> u32 {
    u32 result = 0;
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
    };

public:
};

struct MaterialHandle;
struct TextureHandle;
struct ShaderHandle;

class Transform {
public:
    [[nodiscard]] auto calculate() const -> mat4x4 {
        return m_translation * m_rotation * m_scale;
    }

public:
    mat4x4 m_scale = mat4x4::identity();
    mat4x4 m_rotation = mat4x4::identity();
    mat4x4 m_translation = mat4x4::identity();
};

class Object {
public:
    GPUMeshData*    m_mesh = nullptr;
    /// Legacy optional CPU geometry reference; ignored by submission and rendering.
    /// CPU geometry may be changed or destroyed immediately after registration.
    Mesh*           m_vertex_data = nullptr;
    MaterialHandle* m_material = nullptr;
    Transform       m_transform;
};

class RenderSystem {
public:
    RenderSystem() = default;
    ~RenderSystem();
    RenderSystem(const RenderSystem&) = delete;
    auto operator=(const RenderSystem&) -> RenderSystem& = delete;
    RenderSystem(RenderSystem&&) = delete;
    auto operator=(RenderSystem&&) -> RenderSystem& = delete;

    /// Creates and initializes the renderer for `api` on `window`.
    ///
    /// Invariant: `m_renderer` is non-null if and only if the last `init*()` call
    /// succeeded; a renderer that failed to initialize is destroyed before the call
    /// returns and is never observable. `m_api` is `UNDEFINED` while there is no
    /// renderer.
    ///
    /// A system that is already initialized is shut down first (see `shutdown()`),
    /// also if the new initialization then fails. The backend is the requested one or
    /// none: there is no fallback to another graphics API.
    [[nodiscard]] auto init(GRAPHICS_API api, Window* window) -> Result<void, InitError>;

    /// Like `init()`, but with a renderer supplied by the caller instead of one of the
    /// built-in backends. `renderer` must be constructed and not yet initialized; this
    /// calls `IRenderer::initialize()` on it.
    [[nodiscard]] auto init_with_renderer(
        GRAPHICS_API               api,
        std::unique_ptr<IRenderer> renderer
    ) -> Result<void, InitError>;

    /// Releases all registered resources and then the renderer. Safe to call in any
    /// state, including after a failed `init()` and repeatedly. The destructor does
    /// the same.
    auto shutdown() -> void;

    [[nodiscard]] auto is_initialized() const -> bool { return m_renderer != nullptr; }

    /// Transactional alternatives: on error no registry entry is added. Returned
    /// pointers borrow registry storage and remain valid until shutdown(). Do not
    /// move/erase registered resources. Mesh upload copies data synchronously; the
    /// source Mesh need not survive registration.
    [[nodiscard]] auto try_register_texture(const Image& image)
        -> Result<TextureHandle*, RendererError>;
    [[nodiscard]] auto try_register_shader(const ShaderHandle::Desc& desc)
        -> Result<ShaderHandle*, RendererError>;
    [[nodiscard]] auto try_register_mesh(const Mesh& data)
        -> Result<GPUMeshData*, RendererError>;
    [[nodiscard]] auto try_register_material(ShaderHandle* shader, TextureHandle* texture)
        -> Result<MaterialHandle*, RendererError>;

    /// Compatibility wrappers: log the error once and return nullptr on failure.
    auto register_texture(Image& image) -> TextureHandle*;
    auto register_shader(const ShaderHandle::Desc& desc) -> ShaderHandle*;
    auto register_mesh(const Mesh& data) -> GPUMeshData*;
    auto register_material(ShaderHandle* shader, TextureHandle* texture)
        -> MaterialHandle*;

    auto submit(const Object& obj) -> void;

public:
    [[nodiscard]] static auto list_available_graphics_apis() -> std::vector<GRAPHICS_API>;
    /// Whether this build of JadeFrame has a renderer for `api`. Fails with
    /// `BACKEND_UNSUPPORTED` or `BACKEND_DISABLED`. Needs neither a window nor the
    /// graphics API itself, so it does not tell whether initialization will succeed.
    [[nodiscard]] static auto check_graphics_api(GRAPHICS_API api)
        -> Result<void, InitError>;

public:
    GRAPHICS_API               m_api = GRAPHICS_API::UNDEFINED;
    std::unique_ptr<IRenderer> m_renderer;

    mutable std::deque<RenderCommand> m_render_commands;

    std::deque<TextureHandle>  m_registered_textures;
    std::deque<ShaderHandle>   m_registered_shaders;
    std::deque<MaterialHandle> m_registered_materials;
    std::deque<GPUMeshData>    m_registered_meshes;
};

} // namespace JadeFrame
