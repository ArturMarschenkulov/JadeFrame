#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/utils/logger.h"

#include "graphics_shared.h"
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

    if (m_renderer != nullptr) { m_renderer->destroy_buffer(m_handle); }

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

    if (m_renderer != nullptr) {
        m_handle = m_renderer->create_buffer(usage, data, size);
    }
}

GPUMeshData::GPUMeshData(
    RenderSystem* system,
    const Mesh&   vertex_data,
    bool          interleaved
) {

    const std::vector<f32> flat_data = convert_into_data(vertex_data, interleaved);

    void*  data = (void*)flat_data.data();
    size_t size = sizeof(flat_data[0]) * flat_data.size();
    m_vertex_buffer =
        std::make_unique<GPUBuffer>(system, data, size, GPUBuffer::TYPE::VERTEX);

    if (!vertex_data.m_indices.empty()) {
        const auto& indices = vertex_data.m_indices;
        void*       indices_data = (void*)indices.data();
        size_t      indices_size = sizeof(indices[0]) * indices.size();
        m_index_buffer = std::make_unique<GPUBuffer>(
            system, indices_data, indices_size, GPUBuffer::TYPE::INDEX
        );
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
    -> void {
    if (m_renderer == nullptr || m_handle == nullptr) { return; }
    m_renderer->set_shader_uniform(m_handle.get(), name, data, size);
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

RenderSystem::RenderSystem(GRAPHICS_API api, Window* window)
    : m_api(api) {
    m_renderer = create_renderer(api, *this, window);
    if (m_renderer != nullptr) { m_renderer->initialize(); }
}

RenderSystem::~RenderSystem() {
    if (m_renderer != nullptr) { m_renderer->wait_until_idle(); }
}

auto RenderSystem::init(GRAPHICS_API api, Window* window) -> void {
    if (m_renderer != nullptr) { m_renderer->wait_until_idle(); }

    m_registered_meshes.clear();
    m_registered_materials.clear();
    m_registered_shaders.clear();
    m_registered_textures.clear();
    m_render_commands.clear();
    m_renderer.reset();

    m_api = api;
    m_renderer = create_renderer(api, *this, window);
    if (m_renderer == nullptr) {
        Logger::err("Unsupported graphics api: {}", to_string(api));
        return;
    }
    m_renderer->initialize();
}

auto RenderSystem::register_texture(Image& image) -> TextureHandle* {
    if (m_renderer == nullptr) {
        Logger::err("Cannot register a texture without an initialized renderer");
        return nullptr;
    }

    m_registered_textures.emplace_back(image);
    TextureHandle& tex = m_registered_textures.back();
    tex.m_api = m_api;
    tex.m_handle = m_renderer->create_texture(image);
    return &tex;
}

auto RenderSystem::register_shader(const ShaderHandle::Desc& desc) -> ShaderHandle* {

    m_registered_shaders.emplace_back();
    ShaderHandle& shader = m_registered_shaders.back();
    shader.m_code = desc.shading_code;
    shader.m_api = m_api;
    shader.m_renderer = m_renderer.get();
    if (m_renderer != nullptr) {
        shader.m_handle = m_renderer->create_shader(shader.m_code);
    }
    return &shader;
}

/**
 * @brief Registers a mesh on the GPU and returns an id to it.
 *
 * @param format
 * @param data
 * @return u32
 */
auto RenderSystem::register_mesh(const Mesh& data) -> GPUMeshData* {
    m_registered_meshes.emplace_back(this, data);
    return &m_registered_meshes.back();
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
    if (opengl != nullptr) { result.push_back(GRAPHICS_API::OPENGL); }
#endif

#if JADEFRAME_ENABLE_VULKAN
    void* vulkan = nullptr;
#if defined(JF_PLATFORM_LINUX)
    vulkan = load_module("libvulkan.so.1");
#elif defined(JF_PLATFORM_WINDOWS)
    // vulkan = load_module("vulkan-1.dll");
    vulkan = (void*)1;
#else
    JF_UNIMPLEMENTED("");
#endif
    if (vulkan != nullptr) { result.push_back(GRAPHICS_API::VULKAN); }
#endif

    return result;
}

auto RenderSystem::register_material(ShaderHandle* shader, TextureHandle* texture)
    -> MaterialHandle* {
    if (m_renderer == nullptr || shader == nullptr || shader->m_handle == nullptr) {
        Logger::err("Cannot register a material without a valid renderer and shader");
        return nullptr;
    }

    m_registered_materials.emplace_back();
    auto& material = m_registered_materials.back();

    material.m_shader = shader;
    material.m_texture = texture;
    material.m_api = m_api;
    material.m_info = MaterialInfo::default_0();

    if (texture != nullptr && texture->m_handle == nullptr) {
        Logger::err("Cannot register a material with an invalid texture");
        m_registered_materials.pop_back();
        return nullptr;
    }
    material.m_handle = m_renderer->create_material(*shader, texture);

    return &material;
}

auto RenderSystem::submit(const Object& obj) -> void {
    // TODO(artur): Check whether the vertex data is matches the shader's vertex format
    const RenderCommand command = {
        .transform = obj.m_transform.calculate(),
        .vertex_data = obj.m_vertex_data,
        .material = obj.m_material,
        .m_mesh = obj.m_mesh,
    };
    m_render_commands.push_back(command);
}

} // namespace JadeFrame
