#include "JadeFrame/utils/logger.h"
#include "pch.h"
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

namespace JadeFrame {

/*---------------------------
    Image
---------------------------*/

Image::~Image() {
    if (data != nullptr) { stbi_image_free(data); }
}

Image::Image(Image&& other) noexcept
    : data(other.data)
    , width(other.width)
    , height(other.height)
    , num_components(other.num_components) {

    other.data = nullptr;
}

auto Image::operator=(Image&& other) noexcept -> Image& {
    data = other.data;
    width = other.width;
    height = other.height;
    num_components = other.num_components;
    other.data = nullptr;
    return *this;
}

static auto
add_fourth_components(const u8* data, i32 width, i32 height, i32 num_components) -> u8* {
    u32 size = width * height;
    u32 size_in_bytes = size * 4;
    u8* new_data = new u8[size_in_bytes];
    for (i32 i = 0; i < size; i++) {
        new_data[i * 4 + 0] = data[i * num_components + 0];
        new_data[i * 4 + 1] = data[i * num_components + 1];
        new_data[i * 4 + 2] = data[i * num_components + 2];
        new_data[i * 4 + 3] = 255_u8;
    }
    return new_data;
}

auto Image::load_from_path(const std::string& path) -> Image {
    stbi_set_flip_vertically_on_load(true);
    i32 width = 0;
    i32 height = 0;
    i32 num_components = 0;

    u8* data = stbi_load(path.c_str(), &width, &height, &num_components, 0);
    if (stbi_failure_reason() != nullptr) { std::cout << stbi_failure_reason(); }

    if (data == nullptr) {
        Logger::err("Failed to load image: {}", path);
        return {};
    }
    // NOTE: we force 3 components to 4 components
    if (num_components == 3) {
        u8* data_ = add_fourth_components(data, width, height, num_components);
        num_components = 4;
        stbi_image_free(data);
        data = data_;
    }

    Image img;
    img.data = data;
    img.width = width;
    img.height = height;
    img.num_components = num_components;
    return img;
}

/*---------------------------
    GPUBuffer
----------------------------*/

GPUBuffer::GPUBuffer(GPUBuffer&& other) noexcept { (void)other; }

auto GPUBuffer::operator=(GPUBuffer&& other) noexcept -> GPUBuffer& { return *this; }

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

GPUBuffer::GPUBuffer(RenderSystem* system, void* data, size_t size, TYPE usage)
    : m_system(system)
    , m_api(system->m_api)
    , m_size(size)
    , m_type(usage) {

    switch (system->m_api) {
        case GRAPHICS_API::OPENGL: {
            OpenGL_Context* device =
                &(dynamic_cast<OpenGL_Renderer*>(system->m_renderer))->m_context;

            m_handle = device->create_buffer(to_opengl(usage), data, size);
        } break;
        case GRAPHICS_API::VULKAN: {
            vulkan::LogicalDevice* device =
                (dynamic_cast<Vulkan_Renderer*>(system->m_renderer))->m_logical_device;

            m_handle = device->create_buffer(to_vulkan(usage), data, size);
        } break;
        default: assert(false);
    }
}

GPUMeshData::GPUMeshData(
    RenderSystem*     system,
    const VertexData& vertex_data,
    bool              interleaved
) {

    const std::vector<f32> flat_data = convert_into_data(vertex_data, interleaved);

    void*  data = (void*)flat_data.data();
    size_t size = sizeof(flat_data[0]) * flat_data.size();
    m_vertex_buffer = new GPUBuffer(system, data, size, GPUBuffer::TYPE::VERTEX);

    if (!vertex_data.m_indices.empty()) {
        const auto& indices = vertex_data.m_indices;
        void*       indices_data = (void*)indices.data();
        size_t      indices_size = sizeof(indices[0]) * indices.size();
        m_index_buffer =
            new GPUBuffer(system, indices_data, indices_size, GPUBuffer::TYPE::INDEX);
    }
}

/*---------------------------
    Texture Handle
---------------------------*/

TextureHandle::TextureHandle(const Image& img)
    : m_data(img.data)
    , m_size({(u32)img.width, (u32)img.height})
    , m_num_components(img.num_components) {}

TextureHandle::TextureHandle(TextureHandle&& other) noexcept
    : m_data(other.m_data)
    , m_size(other.m_size)
    , m_num_components(other.m_num_components)
    , m_api(other.m_api)
    , m_handle(other.m_handle) {
    other.m_data = nullptr;
    other.m_size = {0, 0};
    other.m_num_components = 0;
    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_handle = nullptr;
    // *this = std::move(other);
}

auto TextureHandle::operator=(TextureHandle&& other) noexcept -> TextureHandle& {
    m_data = other.m_data;
    m_size = other.m_size;
    m_num_components = other.m_num_components;
    m_api = other.m_api;
    m_handle = other.m_handle;

    other.m_data = nullptr;
    other.m_size = {0, 0};
    other.m_num_components = 0;
    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_handle = nullptr;
    return *this;
}

TextureHandle::~TextureHandle() {
    if (m_data != nullptr) { stbi_image_free(m_data); }
}

auto TextureHandle::init(void* context) -> void {
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto* d = static_cast<OpenGL_Context*>(context);
            m_handle = d->create_texture(m_data, m_size, m_num_components);
        } break;
        case GRAPHICS_API::VULKAN: {
            auto* d = static_cast<vulkan::LogicalDevice*>(m_handle);
            auto* texture =
                new vulkan::Vulkan_Texture(*d, m_data, m_size, VK_FORMAT_R8G8B8A8_SRGB);
            m_handle = texture;
        } break;
        default: assert(false);
    }
}

/*---------------------------
    ShaderHandle
---------------------------*/

ShaderHandle::ShaderHandle(const Desc& desc)
    : m_code(desc.shading_code) {}

ShaderHandle::ShaderHandle(ShaderHandle&& other) noexcept
    : m_code(std::move(other.m_code))
    , m_api(other.m_api)
    , m_handle(other.m_handle) {

    // other.m_code = nullptr;
    // other.m_vertex_format = nullptr;
    // other.m_api = GRAPHICS_API::UNDEFINED;
    // other.m_handle = nullptr;
}

auto ShaderHandle::operator=(ShaderHandle&& other) noexcept -> ShaderHandle& {
    m_code = other.m_code;
    m_api = other.m_api;
    m_handle = other.m_handle;

    // other.m_code = nullptr;
    // other.m_vertex_format = nullptr;
    // other.m_api = GRAPHICS_API::UNDEFINED;
    // other.m_handle = nullptr;
    return *this;
}

auto ShaderHandle::set_uniform(const std::string& name, const void* data, size_t size)
    -> void {
    (void)data;
    (void)size;
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto* shader = (opengl::Shader*)m_handle;
            // shader->set_uniform(name, data, size);
        } break;
        case GRAPHICS_API::VULKAN: {
            auto* shader = (Vulkan_Shader*)m_handle;
            auto [set, binding] = shader->get_location(name);
            // shader->bind_buffer(set, binding, data, size);
            // shader->m_sets[set].bind_uniform_buffer(binding, data, size);
            // shader->set_uniform(name, data, size);

        } break;
        default: assert(false);
    }
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
            m_renderer = new OpenGL_Renderer(*this, window);

        } break;
        case GRAPHICS_API::VULKAN: {
            m_renderer = new Vulkan_Renderer(*this, window);
        } break;
        default: assert(false);
    }
}

auto RenderSystem::init(GRAPHICS_API api, Window* window) -> void {
    m_api = api;
    switch (api) {
        case GRAPHICS_API::OPENGL: {
            m_renderer = new OpenGL_Renderer(*this, window);
        } break;
        case GRAPHICS_API::VULKAN: {
            m_renderer = new Vulkan_Renderer(*this, window);
        } break;
        default: assert(false);
    }
}

RenderSystem::RenderSystem(RenderSystem&& other) noexcept
    : m_api(other.m_api)
    , m_renderer(other.m_renderer) {

    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_renderer = nullptr;
}

auto RenderSystem::operator=(RenderSystem&& other) noexcept -> RenderSystem& {
    m_api = other.m_api;
    m_renderer = other.m_renderer;
    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_renderer = nullptr;
    return *this;
}

auto RenderSystem::register_texture(Image& image) -> TextureHandle* {

    m_registered_textures.emplace_back();
    auto& tex = m_registered_textures.back();
    tex.m_data = image.data;
    tex.m_size.x = image.width;
    tex.m_size.y = image.height;
    tex.m_num_components = image.num_components;
    tex.m_api = m_api;
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto*           renderer = dynamic_cast<OpenGL_Renderer*>(m_renderer);
            OpenGL_Context* device = &renderer->m_context;

            tex.m_handle =
                device->create_texture(tex.m_data, tex.m_size, tex.m_num_components);

        } break;
        case GRAPHICS_API::VULKAN: {
            auto*                  renderer = dynamic_cast<Vulkan_Renderer*>(m_renderer);
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
            tex.m_handle =
                new vulkan::Vulkan_Texture(*device, tex.m_data, tex.m_size, format);

        } break;
        default: assert(false);
    }
    return &tex;
}

auto RenderSystem::register_shader(const ShaderHandle::Desc& desc) -> ShaderHandle* {

    m_registered_shaders.emplace_back();
    auto& shader = m_registered_shaders.back();
    shader.m_code = desc.shading_code;
    shader.m_api = m_api;

    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto* ren = dynamic_cast<OpenGL_Renderer*>(m_renderer);
            auto* ctx = (OpenGL_Context*)&ren->m_context;

            opengl::Shader::Desc shader_desc;

            // TODO: Move this whole remapping thing into the OpenGL Shader class. This is
            // not a thing which should be known by a shared layer.

            std::string         v_source;
            ShadingCode::Module mod_0;
            auto&               vert_mod = shader.m_code.m_modules[0];
            mod_0.m_code = remap_for_opengl(vert_mod.m_code, vert_mod.m_stage, nullptr);
            mod_0.m_stage = vert_mod.m_stage;

            std::string         f_source;
            ShadingCode::Module mod_1;
            auto&               frag_mod = shader.m_code.m_modules[1];
            mod_1.m_code = remap_for_opengl(frag_mod.m_code, frag_mod.m_stage, nullptr);
            mod_1.m_stage = frag_mod.m_stage;

            shader_desc.code.m_modules.resize(2);
            shader_desc.code.m_modules[0] = std::move(mod_0);
            shader_desc.code.m_modules[1] = std::move(mod_1);

            shader.m_handle = new opengl::Shader(*ctx, shader_desc);

            // Logger::warn("Vertex source:\n {}", v_source);
            // Logger::warn("Fragment source:\n {}", f_source);
        } break;
        case GRAPHICS_API::VULKAN: {
            auto*                  ren = dynamic_cast<Vulkan_Renderer*>(m_renderer);
            vulkan::LogicalDevice* ctx = ren->m_logical_device;

            Vulkan_Shader::Desc shader_desc;
            shader_desc.code = shader.m_code;

            shader.m_handle = new Vulkan_Shader(*ctx, *ren, shader_desc);
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
auto RenderSystem::register_mesh(const VertexData& data) -> GPUMeshData* {
    m_registered_meshes.emplace_back(this, data);
    return &m_registered_meshes.back();
}

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

#include <dlfcn.h>

static auto load_module(const char* path) -> void* {
#if defined(JF_PLATFORM_LINUX)
    return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
#elif defined(JF_PLATFORM_WINDOWS)
#else
    JF_UNIMPLEMENTED("");
#endif
}

static auto get_program_path() -> std::string {
    char    buf[1024] = {0};
    ssize_t buf_size = readlink("/proc/self/exe", buf, sizeof(buf) /*- 1*/);

    if (buf_size == -1) {
        auto  error_code = errno;
        char* error_str = std::strerror(error_code);
        Logger::err(
            "readlink(/proc/self/exe) failed with error code: {}: {}",
            error_code,
            error_str
        );
    }
    std::string path = buf;
    std::size_t path_end = path.find_last_of('/');
    if (path_end != std::string::npos) { path.resize(path_end + 1); }
    return path;
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
    vulkan = load_module("vulkan-1.dll");
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

    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            //  NOTE: AFAIK, nothing needs to be done

        } break;
        case GRAPHICS_API::VULKAN: {
            if (texture != nullptr) {
                auto* sh = (Vulkan_Shader*)shader->m_handle;
                auto* tex = (vulkan::Vulkan_Texture*)texture->m_handle;

                auto& set = sh->m_sets[vulkan::FREQUENCY::PER_MATERIAL];
                if (set.m_descriptors.empty()) {
                    Logger::err("The shader does not support textures");
                    assert(false);
                    return nullptr;
                }
                set.bind_combined_image_sampler(0, *tex);
                set.update();
            }
        } break;
        default: assert(false);
    }

    return &material;
}

auto RenderSystem::submit(const Object& obj) -> void {
    const RenderCommand command = {
        .transform = obj.m_transform,
        .vertex_data = obj.m_vertex_data,
        .material = obj.m_material,
        .m_mesh = obj.m_mesh,
    };
    m_render_commands.push_back(command);
}

} // namespace JadeFrame