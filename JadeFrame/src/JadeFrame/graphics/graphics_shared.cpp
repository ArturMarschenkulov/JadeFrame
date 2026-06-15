#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/utils/logger.h"

#include "graphics_shared.h"
#include "JadeFrame/platform/platform_shared.h"
#include "vulkan/renderer.h"
#include "vulkan/shader.h"
#include "opengl/opengl_renderer.h"
#include "graphics_language.h"

#include "JadeFrame/utils/assert.h"
#include "reflect.h"

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
auto to_string(SHADER_TYPE type) -> const char* {
    switch (type) {
        case SHADER_TYPE::NONE: return "NONE";
        case SHADER_TYPE::F32: return "F32";
        case SHADER_TYPE::V_2_F32: return "F32_2";
        case SHADER_TYPE::V_3_F32: return "F32_3";
        case SHADER_TYPE::V_4_F32: return "F32_4";
        case SHADER_TYPE::M_3_3_F32: return "M_F32_3";
        case SHADER_TYPE::M_4_4_F32: return "M_F32_4";
        case SHADER_TYPE::I32: return "I32";
        case SHADER_TYPE::V_2_I32: return "I32_2";
        case SHADER_TYPE::V_3_I32: return "I32_3";
        case SHADER_TYPE::V_4_I32: return "I32_4";
        case SHADER_TYPE::BOOL: return "BOOL";
        case SHADER_TYPE::SAMPLER_1D: return "SAMPLER_1D";
        case SHADER_TYPE::SAMPLER_2D: return "SAMPLER_2D";
        case SHADER_TYPE::SAMPLER_3D: return "SAMPLER_3D";
        case SHADER_TYPE::SAMPLER_CUBE: return "SAMPLER_CUBE";
        default:
            Logger::err("Unknown SHADER_TYPE: {}", (int)type);
            assert(false);
            return "";
            break;
    }
}

static auto to_opengl(GPUBuffer::TYPE type) -> opengl::Buffer::TYPE {
    opengl::Buffer::TYPE result = {};
    switch (type) {
        case GPUBuffer::TYPE::VERTEX: result = opengl::Buffer::TYPE::VERTEX; break;
        case GPUBuffer::TYPE::INDEX: result = opengl::Buffer::TYPE::INDEX; break;
        case GPUBuffer::TYPE::UNIFORM: result = opengl::Buffer::TYPE::UNIFORM; break;
        default: JF_UNREACHABLE();
    }
    return result;
}

static auto to_vulkan(GPUBuffer::TYPE type) -> vulkan::Buffer::TYPE {
    vulkan::Buffer::TYPE result = {};
    switch (type) {
        case GPUBuffer::TYPE::VERTEX: result = vulkan::Buffer::TYPE::VERTEX; break;
        case GPUBuffer::TYPE::INDEX: result = vulkan::Buffer::TYPE::INDEX; break;
        case GPUBuffer::TYPE::UNIFORM: result = vulkan::Buffer::TYPE::UNIFORM; break;
        // case GPUBuffer::TYPE::STORAGE: result = vulkan::Buffer::TYPE::STORAGE; break;
        default: JF_UNREACHABLE();
    }
    return result;
}

static auto delete_vulkan_texture(void* handle) noexcept -> void {
    delete static_cast<vulkan::Vulkan_Texture*>(handle);
}

static auto delete_opengl_shader(void* handle) noexcept -> void {
    delete static_cast<opengl::Shader*>(handle);
}

static auto delete_vulkan_shader(void* handle) noexcept -> void {
    delete static_cast<Vulkan_Shader*>(handle);
}

static auto delete_opengl_material(void* handle) noexcept -> void {
    delete static_cast<opengl::Material*>(handle);
}

static auto delete_vulkan_material(void* handle) noexcept -> void {
    delete static_cast<Vulkan_Material*>(handle);
}

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
    : m_system(other.m_system)
    , m_api(other.m_api)
    , m_handle(other.m_handle)
    , m_size(other.m_size)
    , m_type(other.m_type) {

    other.m_system = nullptr;
    other.m_handle = nullptr;
    other.m_size = 0;
}

auto GPUBuffer::operator=(GPUBuffer&& other) noexcept -> GPUBuffer& {

    if (this == &other) { return *this; }

    this->release();

    m_system = other.m_system;
    m_api = other.m_api;
    m_handle = other.m_handle;
    m_size = other.m_size;
    m_type = other.m_type;

    other.m_system = nullptr;
    other.m_handle = nullptr;
    other.m_size = 0;

    return *this;
}

GPUBuffer::~GPUBuffer() { this->release(); }

auto GPUBuffer::release() -> void {
    if (m_handle == nullptr) { return; }

    switch (m_api) {
        case GRAPHICS_API::VULKAN: {
            if (m_system == nullptr || m_system->m_renderer == nullptr) { break; }
            auto* renderer = dynamic_cast<Vulkan_Renderer*>(m_system->m_renderer.get());
            if (renderer == nullptr || renderer->m_logical_device == nullptr) { break; }
            auto* buffer = static_cast<vulkan::Buffer*>(m_handle);
            renderer->m_logical_device->destroy_buffer(buffer);
        } break;
        case GRAPHICS_API::OPENGL:
            // OpenGL buffers are owned by opengl::Context and released with the context.
            break;
        case GRAPHICS_API::UNDEFINED: break;
        default: assert(false); break;
    }

    m_handle = nullptr;
    m_system = nullptr;
    m_api = GRAPHICS_API::UNDEFINED;
    m_size = 0;
}

GPUBuffer::GPUBuffer(RenderSystem* system, void* data, size_t size, TYPE usage)
    : m_system(system)
    , m_api(system->m_api)
    , m_size(size)
    , m_type(usage) {

    switch (system->m_api) {
        case GRAPHICS_API::OPENGL: {
            opengl::Context* device =
                &(dynamic_cast<OpenGL_Renderer*>(system->m_renderer.get()))->m_context;

            m_handle = device->create_buffer(to_opengl(usage), data, size);
        } break;
        case GRAPHICS_API::VULKAN: {
            vulkan::LogicalDevice* device =
                (dynamic_cast<Vulkan_Renderer*>(system->m_renderer.get()))
                    ->m_logical_device;

            m_handle = device->create_buffer(to_vulkan(usage), data, size);
        } break;
        default: assert(false);
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
    , m_handle(std::move(other.m_handle)) {

    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_handle = nullptr;
}

auto ShaderHandle::operator=(ShaderHandle&& other) noexcept -> ShaderHandle& {
    if (this == &other) { return *this; }

    this->release();

    m_code = std::move(other.m_code);
    m_api = other.m_api;
    m_handle = std::move(other.m_handle);

    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_handle = nullptr;
    return *this;
}

ShaderHandle::~ShaderHandle() { this->release(); }

auto ShaderHandle::release() -> void {
    if (m_handle == nullptr) { return; }

    m_handle.reset();
    m_api = GRAPHICS_API::UNDEFINED;
}

auto ShaderHandle::set_uniform(const std::string& name, const void* data, size_t size)
    -> void {
    (void)data;
    (void)size;
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto* shader = static_cast<opengl::Shader*>(m_handle.get());
            // shader->set_uniform(name, data, size);
        } break;
        case GRAPHICS_API::VULKAN: {
            auto* shader = static_cast<Vulkan_Shader*>(m_handle.get());
            auto [set, binding] = shader->get_location(name);
            // shader->bind_buffer(set, binding, data, size);
            // shader->m_sets[set].bind_uniform_buffer(binding, data, size);
            // shader->set_uniform(name, data, size);

        } break;
        default: assert(false);
    }
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
    VertexAttribute
---------------------------*/

/*---------------------------
    VertexFormat
---------------------------*/
VertexFormat::VertexFormat(const std::vector<VertexAttribute>& attributes)
    : m_attributes(attributes) {
    for (const VertexAttribute& attribute : attributes) {
        m_stride += get_size(attribute.type);
    }
}

/*---------------------------
    RenderSystem
---------------------------*/

RenderSystem::RenderSystem(GRAPHICS_API api, Window* window)
    : m_api(api) {

    switch (api) {
        case GRAPHICS_API::OPENGL: {
            m_renderer = std::make_unique<OpenGL_Renderer>(*this, window);

        } break;
        case GRAPHICS_API::VULKAN: {
            m_renderer = std::make_unique<Vulkan_Renderer>(*this, window);
        } break;
        default: assert(false);
    }
}

RenderSystem::~RenderSystem() = default;

auto RenderSystem::init(GRAPHICS_API api, Window* window) -> void {
    m_registered_meshes.clear();
    m_registered_materials.clear();
    m_registered_shaders.clear();
    m_registered_textures.clear();
    m_render_commands.clear();
    m_renderer.reset();

    m_api = api;
    switch (api) {
        case GRAPHICS_API::OPENGL: {
            m_renderer = std::make_unique<OpenGL_Renderer>(*this, window);
        } break;
        case GRAPHICS_API::VULKAN: {
            m_renderer = std::make_unique<Vulkan_Renderer>(*this, window);
        } break;
        default: {
            Logger::err("Unsupported graphics api: {}", to_string(api));
            assert(false);
        }
    }
}

auto RenderSystem::register_texture(Image& image) -> TextureHandle* {

    m_registered_textures.emplace_back();
    TextureHandle& tex = m_registered_textures.back();
    tex.m_size.x = image.width;
    tex.m_size.y = image.height;
    tex.m_num_components = image.num_components;
    tex.m_api = m_api;
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto*            renderer = dynamic_cast<OpenGL_Renderer*>(m_renderer.get());
            opengl::Context* device = &renderer->m_context;
            tex.m_handle = NativeHandle(
                device->create_texture(
                    image.data.data(), tex.m_size, tex.m_num_components
                ),
                noop_native_handle_deleter
            );

        } break;
        case GRAPHICS_API::VULKAN: {
            auto* renderer = dynamic_cast<Vulkan_Renderer*>(m_renderer.get());
            vulkan::LogicalDevice* device = renderer->m_logical_device;

            VkFormat format = {};
            switch (tex.m_num_components) {
                case 3: format = VK_FORMAT_R8G8B8_SRGB; break;
                case 4: format = VK_FORMAT_R8G8B8A8_SRGB; break;
                default:
                    Logger::err(
                        "Unsupported number of components: {}", tex.m_num_components
                    );
                    assert(false);
            }
            tex.m_handle = NativeHandle(
                new vulkan::Vulkan_Texture(
                    *device, image.data.data(), tex.m_size, format
                ),
                delete_vulkan_texture
            );

        } break;
        default: assert(false);
    }
    return &tex;
}

auto RenderSystem::register_shader(const ShaderHandle::Desc& desc) -> ShaderHandle* {

    m_registered_shaders.emplace_back();
    ShaderHandle& shader = m_registered_shaders.back();
    shader.m_code = desc.shading_code;
    shader.m_api = m_api;

    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto*            ren = dynamic_cast<OpenGL_Renderer*>(m_renderer.get());
            opengl::Context* ctx = &ren->m_context;

            opengl::Shader::Desc shader_desc;

            // TODO: Move this whole remapping thing into the OpenGL Shader class. This is
            // not a thing which should be known by a shared layer.

            ShadingCode::Module  mod_0;
            ShadingCode::Module& vert_mod = shader.m_code.m_modules[0];
            mod_0.m_code = remap_for_opengl(vert_mod.m_code, vert_mod.m_stage, nullptr);
            mod_0.m_stage = vert_mod.m_stage;

            ShadingCode::Module  mod_1;
            ShadingCode::Module& frag_mod = shader.m_code.m_modules[1];
            mod_1.m_code = remap_for_opengl(frag_mod.m_code, frag_mod.m_stage, nullptr);
            mod_1.m_stage = frag_mod.m_stage;

            shader_desc.code.m_modules.resize(2);
            shader_desc.code.m_modules[0] = std::move(mod_0);
            shader_desc.code.m_modules[1] = std::move(mod_1);

            shader.m_handle =
                NativeHandle(new opengl::Shader(*ctx, shader_desc), delete_opengl_shader);

            // Logger::warn("Vertex source:\n {}", v_source);
            // Logger::warn("Fragment source:\n {}", f_source);
        } break;
        case GRAPHICS_API::VULKAN: {
            auto*                  ren = dynamic_cast<Vulkan_Renderer*>(m_renderer.get());
            vulkan::LogicalDevice* ctx = ren->m_logical_device;

            Vulkan_Shader::Desc shader_desc;
            shader_desc.code = shader.m_code;

            shader.m_handle = NativeHandle(
                new Vulkan_Shader(*ctx, *ren, shader_desc), delete_vulkan_shader
            );
        } break;
        default: assert(false);
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

    void* opengl = nullptr;
#if defined(JF_PLATFORM_LINUX)
    opengl = load_module("libGL.so.1");
#elif defined(JF_PLATFORM_WINDOWS)
    opengl = load_module("opengl32.dll");
#else
    JF_UNIMPLEMENTED("");
#endif
    if (opengl != nullptr) { result.push_back(GRAPHICS_API::OPENGL); }

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

    return result;
}

auto RenderSystem::register_material(ShaderHandle* shader, TextureHandle* texture)
    -> MaterialHandle* {
    m_registered_materials.emplace_back();
    auto& material = m_registered_materials.back();

    material.m_shader = shader;
    material.m_texture = texture;
    material.m_api = m_api;
    material.m_info = MaterialInfo::default_0();

    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            //  NOTE: AFAIK, nothing needs to be done
            auto* sh = static_cast<opengl::Shader*>(shader->m_handle.get());
            auto* tex = texture == nullptr
                            ? nullptr
                            : static_cast<opengl::Texture*>(texture->m_handle.get());
            material.m_handle = NativeHandle(
                new opengl::Material(
                    (dynamic_cast<OpenGL_Renderer*>(m_renderer.get()))->m_context,
                    *sh,
                    tex
                ),
                delete_opengl_material
            );

        } break;
        case GRAPHICS_API::VULKAN: {
            auto* sh = static_cast<Vulkan_Shader*>(shader->m_handle.get());

            auto* tex = texture == nullptr
                            ? nullptr
                            : static_cast<vulkan::Vulkan_Texture*>(
                                  texture->m_handle.get()
                              );

            material.m_handle = NativeHandle(
                new Vulkan_Material(
                    *(dynamic_cast<Vulkan_Renderer*>(m_renderer.get()))->m_logical_device,
                    *sh,
                    tex
                ),
                delete_vulkan_material
            );
            if (texture != nullptr) {}
        } break;
        default: assert(false);
    }

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
