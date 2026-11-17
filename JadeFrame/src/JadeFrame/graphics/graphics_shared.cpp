#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/utils/logger.h"

#include "graphics_shared.h"
#include <limits>
#include <algorithm>
#include "JadeFrame/platform/platform_shared.h"
#include "renderer_factory.h"

#include "JadeFrame/utils/assert.h"

#define STB_IMAGE_IMPLEMENTATION
#undef __OPTIMIZE__
JF_PRAGMA_NO_WARNINGS_PUSH
#include "stb/stb_image.h"
JF_PRAGMA_NO_WARNINGS_POP

/*
Shader conventions:
Opengl: Program consists of shaders.
Vulkan: Pipeline consists of shader modules.
Metal:  Render Pipeline consists of shader functions.
D3D11: Pipeline consists of shader stages.
D3D12: Pipeline consists of shader stages.

*/

namespace JadeFrame {

/*---------------------------
    Image
---------------------------*/

Image::Image(Image&& other) noexcept
    : data(std::move(other.data))
    , width(other.width)
    , height(other.height)
    , num_components(other.num_components) {}

auto Image::operator=(Image&& other) noexcept -> Image& {
    data = std::move(other.data);
    width = other.width;
    height = other.height;
    num_components = other.num_components;
    return *this;
}

auto Image::load_from_path(const std::string& path) -> Image {
    stbi_set_flip_vertically_on_load(true);
    i32 width = 0;
    i32 height = 0;
    i32 num_components = 0;

    i32 desired_components = 4;
    u8* stb_data =
        stbi_load(path.c_str(), &width, &height, &num_components, desired_components);
    if (stb_data == nullptr) {
        Logger::err("Failed to load image {}: {}", path, stbi_failure_reason());
        return {};
    }

    if (num_components == 0) {
        stbi_image_free(stb_data);
        Logger::err("Failed to load image: {}", path);
        return {};
    }

    size_t num_bytes = (size_t)width * (size_t)height * (size_t)desired_components;
    std::vector<u8> data = std::vector<u8>(num_bytes);
    std::memcpy(data.data(), stb_data, num_bytes);
    stbi_image_free(stb_data);

    Image img;
    img.data = data;
    img.width = width;
    img.height = height;
    img.num_components = desired_components;
    return img;
}

/*---------------------------
    GPUBuffer
----------------------------*/

GPUBuffer::GPUBuffer(GPUBuffer&& other) noexcept
    : m_renderer(other.m_renderer)
    , m_renderer_lifetime(std::move(other.m_renderer_lifetime))
    , m_api(other.m_api)
    , m_handle(other.m_handle)
    , m_size(other.m_size)
    , m_type(other.m_type) {

    other.m_renderer = nullptr;
    other.m_handle = nullptr;
    other.m_size = 0;
}

auto GPUBuffer::operator=(GPUBuffer&& other) noexcept -> GPUBuffer& {

    if (this == &other) { return *this; }

    this->release();

    m_renderer = other.m_renderer;
    m_renderer_lifetime = std::move(other.m_renderer_lifetime);
    m_api = other.m_api;
    m_handle = other.m_handle;
    m_size = other.m_size;
    m_type = other.m_type;

    other.m_renderer = nullptr;
    other.m_handle = nullptr;
    other.m_size = 0;

    return *this;
}

GPUBuffer::~GPUBuffer() { this->release(); }

auto GPUBuffer::release() -> void {
    if (m_handle == nullptr) { return; }

    if (m_renderer != nullptr && !m_renderer_lifetime.expired()) {
        m_renderer->destroy_buffer(m_handle);
    }

    m_handle = nullptr;
    m_renderer = nullptr;
    m_api = GRAPHICS_API::UNDEFINED;
    m_size = 0;
}

GPUBuffer::GPUBuffer(RenderSystem* system, void* data, size_t size, TYPE usage)
    : m_renderer(system != nullptr && system->m_renderer != nullptr
                     ? system->m_renderer.get()
                     : nullptr)
    , m_api(system != nullptr ? system->m_api : GRAPHICS_API::UNDEFINED)
    , m_size(size)
    , m_type(usage) {

    if (m_renderer) { m_renderer_lifetime = m_renderer->resource_lifetime(); }
    if (m_renderer && size > 0 && usage <= TYPE::STAGING &&
        ((usage != TYPE::VERTEX && usage != TYPE::INDEX) || data != nullptr) &&
        (usage != TYPE::INDEX || size % sizeof(u32) == 0)) {
        m_handle = m_renderer->create_buffer(usage, data, size);
    }
    if (!m_handle) {
        m_renderer = nullptr;
        m_api = GRAPHICS_API::UNDEFINED;
        m_size = 0;
    }
}

auto GPUMeshData::cache_draw_data(const Mesh& mesh, bool interleaved)
    -> Result<void, RendererError> {
    const size_t vertices = mesh.attribute_data(Mesh::POSITION.m_id)->count();
    // Both backends share this upload path; OpenGL draw counts are signed 32-bit.
    constexpr auto max_count = static_cast<size_t>(std::numeric_limits<i32>::max());
    if (vertices > max_count || mesh.m_indices.size() > max_count) {
        return err<void, RendererError>(RendererError{
            RendererErrorCode::INVALID_STATE, "Mesh draw count exceeds supported limits"
        });
    }
    std::vector<VertexAttribute> attributes;
    attributes.reserve(mesh.m_attributes.size());
    for (const auto& [id, attribute] : mesh.m_attributes) {
        (void)id;
        attributes.push_back({attribute.m_attribute.m_format,
                              static_cast<u32>(attributes.size())});
    }
    m_vertex_format = VertexFormat(attributes);
    m_vertex_count = static_cast<u32>(vertices);
    m_index_count = static_cast<u32>(mesh.m_indices.size());
    m_interleaved = interleaved;
    return ok<void, RendererError>();
}

GPUMeshData::GPUMeshData(
    RenderSystem* system,
    const Mesh&   vertex_data,
    bool          interleaved
) {

    auto flat_data_result = convert_into_data(vertex_data, interleaved);
    if (flat_data_result.is_err()) {
        Logger::err(
            "Cannot create GPU mesh from invalid mesh: {}",
            flat_data_result.error().m_message
        );
        return;
    }
    auto cached = cache_draw_data(vertex_data, interleaved);
    if (cached.is_err()) {
        Logger::err("Cannot create GPU mesh: {}", cached.error().m_message);
        return;
    }
    std::vector<f32> flat_data = std::move(flat_data_result).value();

    void*  data = flat_data.empty() ? nullptr : static_cast<void*>(flat_data.data());
    size_t size = sizeof(f32) * flat_data.size();
    m_vertex_buffer =
        std::make_unique<GPUBuffer>(system, data, size, GPUBuffer::TYPE::VERTEX);

    if (!m_vertex_buffer->m_handle) {
        m_vertex_buffer.reset();
        return;
    }
    if (!vertex_data.m_indices.empty()) {
        const auto& indices = vertex_data.m_indices;
        void*       indices_data = (void*)indices.data();
        size_t      indices_size = sizeof(indices[0]) * indices.size();
        m_index_buffer = std::make_unique<GPUBuffer>(
            system, indices_data, indices_size, GPUBuffer::TYPE::INDEX
        );
        if (!m_index_buffer->m_handle) {
            m_index_buffer.reset();
            m_vertex_buffer.reset();
        }
    }
}

GPUMeshData::~GPUMeshData() = default;
GPUMeshData::GPUMeshData(GPUMeshData&& other) noexcept = default;
auto GPUMeshData::operator=(GPUMeshData&& other) noexcept -> GPUMeshData& = default;

/*---------------------------
    Texture Handle
---------------------------*/

TextureHandle::TextureHandle(const Image& img)
    : m_size(v2u32::create((u32)img.width, (u32)img.height))
    , m_num_components(img.num_components) {}

TextureHandle::TextureHandle(TextureHandle&& other) noexcept
    : m_size(other.m_size)
    , m_num_components(other.m_num_components)
    , m_api(other.m_api)
    , m_handle(std::move(other.m_handle)) {
    other.m_size = v2u32::create(0, 0);
    other.m_num_components = 0;
    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_handle = nullptr;
    // *this = std::move(other);
}

auto TextureHandle::operator=(TextureHandle&& other) noexcept -> TextureHandle& {
    if (this == &other) { return *this; }

    this->release();

    m_size = other.m_size;
    m_num_components = other.m_num_components;
    m_api = other.m_api;
    m_handle = std::move(other.m_handle);

    other.m_size = v2u32::create(0, 0);
    other.m_num_components = 0;
    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_handle = nullptr;
    return *this;
}

TextureHandle::~TextureHandle() { release(); }

auto TextureHandle::release() -> void {
    if (m_handle == nullptr) { return; }

    m_handle.reset();
    m_api = GRAPHICS_API::UNDEFINED;
    m_size = v2u32::create(0, 0);
    m_num_components = 0;
}

/*---------------------------
    ShaderHandle
---------------------------*/

ShaderHandle::ShaderHandle(const Desc& desc)
    : m_code(desc.shading_code) {}

ShaderHandle::ShaderHandle(ShaderHandle&& other) noexcept
    : m_code(std::move(other.m_code))
    , m_api(other.m_api)
    , m_handle(std::move(other.m_handle))
    , m_renderer(other.m_renderer) {

    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_handle = nullptr;
    other.m_renderer = nullptr;
}

auto ShaderHandle::operator=(ShaderHandle&& other) noexcept -> ShaderHandle& {
    if (this == &other) { return *this; }

    this->release();

    m_code = std::move(other.m_code);
    m_api = other.m_api;
    m_handle = std::move(other.m_handle);
    m_renderer = other.m_renderer;

    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_handle = nullptr;
    other.m_renderer = nullptr;
    return *this;
}

ShaderHandle::~ShaderHandle() { this->release(); }

auto ShaderHandle::release() -> void {
    if (m_handle == nullptr) { return; }

    m_handle.reset();
    m_api = GRAPHICS_API::UNDEFINED;
    m_renderer = nullptr;
}

auto ShaderHandle::set_uniform(const std::string& name, const void* data, size_t size)
    -> Result<void, RendererError> {
    if (m_renderer == nullptr || m_handle == nullptr) {
        return err<void, RendererError>(RendererError{
            RendererErrorCode::INVALID_STATE,
            "The shader is not registered with a renderer",
        });
    }
    return m_renderer->set_shader_uniform(m_handle.get(), name, data, size);
}

/*---------------------------
    MaterialHandle
---------------------------*/

MaterialHandle::MaterialHandle(MaterialHandle&& other) noexcept
    : m_shader(std::exchange(other.m_shader, nullptr))
    , m_texture(std::exchange(other.m_texture, nullptr))
    , m_handle(std::move(other.m_handle))
    , m_info(std::move(other.m_info))
    , m_api(std::exchange(other.m_api, GRAPHICS_API::UNDEFINED)) {}

auto MaterialHandle::operator=(MaterialHandle&& other) noexcept -> MaterialHandle& {
    if (this == &other) { return *this; }

    this->release();

    m_shader = std::exchange(other.m_shader, nullptr);
    m_texture = std::exchange(other.m_texture, nullptr);
    m_handle = std::move(other.m_handle);
    m_info = std::move(other.m_info);
    m_api = std::exchange(other.m_api, GRAPHICS_API::UNDEFINED);
    return *this;
}

MaterialHandle::~MaterialHandle() { this->release(); }

auto MaterialHandle::release() -> void {
    if (m_handle == nullptr) { return; }

    m_handle.reset();
    m_shader = nullptr;
    m_texture = nullptr;
    m_api = GRAPHICS_API::UNDEFINED;
}

/*---------------------------
    RenderSystem
---------------------------*/

auto to_string(const InitError& error) -> std::string {
    std::string result = to_string(error.m_code);
    if (error.m_api != GRAPHICS_API::UNDEFINED) {
        result += " [";
        result += to_string(error.m_api);
        result += "]";
    }
    result += ": ";
    result += error.m_message;
    return result;
}

RenderSystem::~RenderSystem() { this->shutdown(); }

auto RenderSystem::shutdown() -> void {
    // `m_renderer` is only ever set to a successfully initialized renderer, so it is
    // valid to use here.
    if (m_renderer != nullptr) { m_renderer->wait_until_idle(); }

    // The registered resources hold native handles that are released through the
    // renderer's graphics context: they have to go first.
    m_render_commands.clear();
    m_registered_meshes.clear();
    m_registered_materials.clear();
    m_registered_shaders.clear();
    m_registered_textures.clear();

    m_renderer.reset();
    m_api = GRAPHICS_API::UNDEFINED;
}

auto RenderSystem::check_graphics_api(GRAPHICS_API api) -> Result<void, InitError> {
    return check_renderer_support(api);
}

auto RenderSystem::init(GRAPHICS_API api, Window* window) -> Result<void, InitError> {
    this->shutdown();

    auto renderer = create_renderer(api, *this, window);
    if (renderer.is_err()) {
        return err<void, InitError>(std::move(renderer).error());
    }
    return this->init_with_renderer(api, std::move(renderer).value());
}

auto RenderSystem::init_with_renderer(
    GRAPHICS_API               api,
    std::unique_ptr<IRenderer> renderer
) -> Result<void, InitError> {
    this->shutdown();

    if (renderer == nullptr) {
        return err<void, InitError>(InitError{
            InitErrorCode::INVALID_ARGUMENT,
            api,
            "the render system cannot be initialized without a renderer",
        });
    }

    // The renderer stays local until it is fully initialized, so a partially
    // initialized one cannot be reached through the render system. If it fails, it
    // goes out of scope here and its destructor releases what was created so far.
    auto initialized = renderer->initialize();
    if (initialized.is_err()) { return initialized; }

    m_renderer = std::move(renderer);
    m_api = api;
    return ok<void, InitError>();
}

namespace {
template<class T>
auto registration_error(
    std::string       message,
    RendererErrorCode code = RendererErrorCode::INVALID_STATE
) -> Result<T*, RendererError> {
    return err<T*, RendererError>(RendererError{code, std::move(message)});
}

template<class T>
auto registered(const std::deque<T>& registry, const T* resource) -> bool {
    return std::any_of(registry.begin(), registry.end(), [resource](const T& entry) {
        return &entry == resource;
    });
}

template<class T>
auto legacy_registration(Result<T*, RendererError> result) -> T* {
    if (result.is_err()) {
        Logger::err("{}", result.error().m_message);
        return nullptr;
    }
    return result.value();
}
} // namespace

auto validate_texture_image(const Image& image) -> Result<void, RendererError> {
    if (image.width <= 0 || image.height <= 0 ||
        (image.num_components != 3 && image.num_components != 4)) {
        return err<void, RendererError>(RendererError{
            RendererErrorCode::INVALID_STATE,
            "Texture upload requires positive dimensions and packed RGB8 or RGBA8 pixels",
        });
    }
    const auto width = static_cast<size_t>(image.width);
    const auto height = static_cast<size_t>(image.height);
    const auto components = static_cast<size_t>(image.num_components);
    if (width > std::numeric_limits<size_t>::max() / height / components ||
        image.data.size() < width * height * components) {
        return err<void, RendererError>(RendererError{
            RendererErrorCode::INVALID_STATE,
            "Texture pixel buffer is smaller than width * height * components",
        });
    }
    return ok<void, RendererError>();
}

auto RenderSystem::try_register_texture(const Image& image)
    -> Result<TextureHandle*, RendererError> {
    if (!m_renderer) {
        return registration_error<TextureHandle>(
            "Texture registration requires an initialized renderer"
        );
    }
    auto validation = validate_texture_image(image);
    if (validation.is_err()) {
        return err<TextureHandle*, RendererError>(validation.error());
    }
    TextureHandle texture(image);
    texture.m_handle = m_renderer->create_texture(image);
    if (!texture.m_handle) {
        return registration_error<TextureHandle>(
            "Backend failed to allocate or upload texture",
            RendererErrorCode::BACKEND_FAILURE
        );
    }
    texture.m_api = m_api;
    m_registered_textures.push_back(std::move(texture));
    return ok<TextureHandle*, RendererError>(&m_registered_textures.back());
}

auto RenderSystem::try_register_shader(const ShaderHandle::Desc& desc)
    -> Result<ShaderHandle*, RendererError> {
    if (!m_renderer) {
        return registration_error<ShaderHandle>(
            "Shader registration requires an initialized renderer"
        );
    }
    ShaderHandle shader(desc);
    auto         native = m_renderer->try_create_shader(shader.m_code);
    if (native.is_err()) {
        return err<ShaderHandle*, RendererError>(std::move(native).error());
    }
    shader.m_handle = std::move(native).value();
    if (!shader.m_handle) {
        return registration_error<ShaderHandle>(
            "Backend returned an empty shader", RendererErrorCode::BACKEND_FAILURE
        );
    }
    shader.m_api = m_api;
    shader.m_renderer = m_renderer.get();
    m_registered_shaders.push_back(std::move(shader));
    return ok<ShaderHandle*, RendererError>(&m_registered_shaders.back());
}

auto RenderSystem::try_register_mesh(const Mesh& data)
    -> Result<GPUMeshData*, RendererError> {
    if (!m_renderer) {
        return registration_error<GPUMeshData>(
            "Mesh registration requires an initialized renderer"
        );
    }
    auto flat = convert_into_data(data, true);
    if (flat.is_err()) { return registration_error<GPUMeshData>(flat.error().m_message); }
    GPUMeshData mesh;
    auto cached = mesh.cache_draw_data(data, true);
    if (cached.is_err()) { return err<GPUMeshData*, RendererError>(cached.error()); }
    auto&       vertices = flat.value();
    mesh.m_vertex_buffer = std::make_unique<GPUBuffer>(
        this, vertices.data(), vertices.size() * sizeof(f32), GPUBuffer::VERTEX
    );
    if (!mesh.m_vertex_buffer->m_handle) {
        return registration_error<GPUMeshData>(
            "Backend failed to allocate or upload vertex buffer",
            RendererErrorCode::BACKEND_FAILURE
        );
    }
    if (!data.m_indices.empty()) {
        mesh.m_index_buffer = std::make_unique<GPUBuffer>(
            this,
            const_cast<u32*>(data.m_indices.data()),
            data.m_indices.size() * sizeof(u32),
            GPUBuffer::INDEX
        );
        if (!mesh.m_index_buffer->m_handle) {
            return registration_error<GPUMeshData>(
                "Backend failed to allocate or upload index buffer; vertex allocation "
                "rolled back",
                RendererErrorCode::BACKEND_FAILURE
            );
        }
    }
    m_registered_meshes.push_back(std::move(mesh));
    return ok<GPUMeshData*, RendererError>(&m_registered_meshes.back());
}

auto RenderSystem::register_texture(Image& image) -> TextureHandle* {
    return legacy_registration(try_register_texture(image));
}

auto RenderSystem::register_shader(const ShaderHandle::Desc& desc) -> ShaderHandle* {
    return legacy_registration(try_register_shader(desc));
}
auto RenderSystem::register_mesh(const Mesh& data) -> GPUMeshData* {
    return legacy_registration(try_register_mesh(data));
}

auto RenderSystem::list_available_graphics_apis() -> std::vector<GRAPHICS_API> {
    std::vector<GRAPHICS_API> result;

#if JADEFRAME_ENABLE_OPENGL
    void* opengl = nullptr;
#if defined(JF_PLATFORM_LINUX)
    opengl = load_module("libGL.so.1");
#elif defined(JF_PLATFORM_WINDOWS)
    opengl = load_module("opengl32.dll");
#else
    JF_UNIMPLEMENTED("");
#endif
    if (opengl != nullptr) {
        result.push_back(GRAPHICS_API::OPENGL);
        if (!unload_module(opengl)) {
            Logger::warn("Failed to unload the OpenGL module after probing it.");
        }
    }
#endif

#if JADEFRAME_ENABLE_VULKAN
    void* vulkan = nullptr;
#if defined(JF_PLATFORM_LINUX)
    vulkan = load_module("libvulkan.so.1");
#elif defined(JF_PLATFORM_WINDOWS)
    vulkan = load_module("vulkan-1.dll");
#else
    JF_UNIMPLEMENTED("");
#endif
    if (vulkan != nullptr) {
        result.push_back(GRAPHICS_API::VULKAN);
        if (!unload_module(vulkan)) {
            Logger::warn("Failed to unload the Vulkan module after probing it.");
        }
    }
#endif

    return result;
}

auto RenderSystem::try_register_material(ShaderHandle* shader, TextureHandle* texture)
    -> Result<MaterialHandle*, RendererError> {
    // Check membership before dereferencing dependencies, including foreign pointers.
    if (!m_renderer || !registered(m_registered_shaders, shader) || !shader->m_handle ||
        shader->m_renderer != m_renderer.get() || shader->m_api != m_api) {
        return registration_error<MaterialHandle>(
            "Material requires a usable shader registered with this render system"
        );
    }
    if (texture && (!registered(m_registered_textures, texture) || !texture->m_handle ||
                    texture->m_api != m_api)) {
        return registration_error<MaterialHandle>(
            "Material texture must be usable and registered with this render system"
        );
    }
    MaterialHandle material;
    material.m_shader = shader;
    material.m_texture = texture;
    material.m_info = MaterialInfo::default_0();
    material.m_handle = m_renderer->create_material(*shader, texture);
    if (!material.m_handle) {
        return registration_error<MaterialHandle>(
            "Backend failed to create material bindings or uniform buffers",
            RendererErrorCode::BACKEND_FAILURE
        );
    }
    material.m_api = m_api;
    m_registered_materials.push_back(std::move(material));
    return ok<MaterialHandle*, RendererError>(&m_registered_materials.back());
}

auto RenderSystem::register_material(ShaderHandle* shader, TextureHandle* texture)
    -> MaterialHandle* {
    return legacy_registration(try_register_material(shader, texture));
}

auto RenderSystem::submit(const Object& obj) -> void {
    if (!m_renderer ||
        !registered(m_registered_meshes, obj.m_mesh) ||
        !registered(m_registered_materials, obj.m_material) ||
        !obj.m_material->m_handle ||
        !registered(m_registered_shaders, obj.m_material->m_shader) ||
        !obj.m_material->m_shader->m_handle) {
        Logger::err("Cannot submit an object with an incomplete render resource set");
        return;
    }

    const auto usable_buffer = [&](const std::unique_ptr<GPUBuffer>& buffer,
                                   GPUBuffer::TYPE type) {
        return buffer && buffer->m_handle && buffer->m_type == type &&
               buffer->m_renderer == m_renderer.get() &&
               !buffer->m_renderer_lifetime.expired();
    };
    if (!usable_buffer(obj.m_mesh->m_vertex_buffer, GPUBuffer::VERTEX) ||
        obj.m_mesh->vertex_count() == 0 || !obj.m_mesh->is_interleaved()) {
        Logger::err("Cannot submit an object without a valid vertex buffer");
        return;
    }
    if (obj.m_mesh->index_count() != 0 &&
        !usable_buffer(obj.m_mesh->m_index_buffer, GPUBuffer::INDEX)) {
        Logger::err("Cannot submit indexed geometry without a usable index buffer");
        return;
    }
    const RenderCommand command = {
        .transform = obj.m_transform.calculate(),
        .material = obj.m_material,
        .m_mesh = obj.m_mesh,
    };
    m_render_commands.push_back(command);
}

} // namespace JadeFrame
