#include "JadeFrame/utils/logger.h"
#include "pch.h"
#include "graphics_shared.h"
#include "JadeFrame/platform/platform_shared.h"
#include "vulkan/vulkan_renderer.h"
#include "vulkan/vulkan_shader.h"
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

static auto add_fourth_components(u8* data, i32 width, i32 height, i32 num_components)
    -> u8* {
    u8* new_data = new u8[width * height * 4];
    for (i32 i = 0; i < width * height; i++) {
        new_data[i * 4 + 0] = data[i * num_components + 0];
        new_data[i * 4 + 1] = data[i * num_components + 1];
        new_data[i * 4 + 2] = data[i * num_components + 2];
        new_data[i * 4 + 3] = 255;
    }
    return new_data;
}

auto Image::load(const std::string& path) -> Image {
    stbi_set_flip_vertically_on_load(true);
    i32 width = 0;
    i32 height = 0;
    i32 num_components = 0;

    u8* data = stbi_load(path.c_str(), &width, &height, &num_components, 0);
    if (stbi_failure_reason()) { std::cout << stbi_failure_reason(); }

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
GPUBuffer::~GPUBuffer() = default;

GPUBuffer::GPUBuffer(GPUBuffer&& other) noexcept { (void)other; }

auto GPUBuffer::operator=(GPUBuffer&& other) noexcept -> GPUBuffer& { return *this; }

static auto to_opengl(GPUBuffer::TYPE type) -> opengl::Buffer::TYPE {
    opengl::Buffer::TYPE result;
    switch (type) {
        case GPUBuffer::TYPE::VERTEX: result = opengl::Buffer::TYPE::VERTEX; break;
        case GPUBuffer::TYPE::INDEX: result = opengl::Buffer::TYPE::INDEX; break;
        case GPUBuffer::TYPE::UNIFORM: result = opengl::Buffer::TYPE::UNIFORM; break;
        default: JF_UNREACHABLE();
    }
    return result;
}

static auto to_vulkan(GPUBuffer::TYPE type) -> vulkan::Buffer::TYPE {
    vulkan::Buffer::TYPE result;
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
    RenderSystem*      system,
    const VertexData&  vertex_data,
    const VertexFormat vertex_format,
    bool               interleaved
) {

    const std::vector<f32> flat_data = convert_into_data(vertex_data, interleaved);

    void*  data = (void*)flat_data.data();
    size_t size = sizeof(flat_data[0]) * flat_data.size();
    m_vertex_buffer = GPUBuffer(system, data, size, GPUBuffer::TYPE::VERTEX);
    // m_vertex_buffer = system->create_buffer(data, size, GPUBuffer::TYPE::VERTEX);

    if (vertex_data.m_indices.size() > 0) {
        auto&  i_data = vertex_data.m_indices;
        void*  data = (void*)i_data.data();
        size_t size = sizeof(i_data[0]) * i_data.size();
        m_index_buffer = GPUBuffer(system, data, size, GPUBuffer::TYPE::INDEX);
        // m_index_buffer = system->create_buffer(data, size, GPUBuffer::TYPE::INDEX);
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
            auto* d = (OpenGL_Context*)context;
            m_handle = d->create_texture(m_data, m_size, m_num_components);
        } break;
        case GRAPHICS_API::VULKAN: {
            auto* d = (vulkan::LogicalDevice*)m_handle;
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

ShaderHandle::ShaderHandle(const Desc& desc) {
    m_code = desc.shading_code;
    m_vertex_format = desc.vertex_format;
}

ShaderHandle::ShaderHandle(ShaderHandle&& other) noexcept {
    m_code = other.m_code;
    m_vertex_format = other.m_vertex_format;
    m_api = other.m_api;
    m_handle = other.m_handle;

    // other.m_code = nullptr;
    // other.m_vertex_format = nullptr;
    // other.m_api = GRAPHICS_API::UNDEFINED;
    // other.m_handle = nullptr;
}

auto ShaderHandle::operator=(ShaderHandle&& other) noexcept -> ShaderHandle& {
    m_code = other.m_code;
    m_vertex_format = other.m_vertex_format;
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

VertexAttribute::VertexAttribute(
    const std::string& name,
    SHADER_TYPE        type,
    bool               normalized
)
    : name(name)
    , type(type)
    , size(SHADER_TYPE_get_size(type))
    , offset(0)
    , normalized(normalized) {}

/*---------------------------
    VertexFormat
---------------------------*/
VertexFormat::VertexFormat(const std::initializer_list<VertexAttribute>& attributes)
    : m_attributes(attributes) {
    this->calculate_offset_and_stride(m_attributes);
}

auto VertexFormat::default_format() -> VertexFormat {
    const VertexFormat result = {
        {     "v_position", SHADER_TYPE::V_3_F32},
        {        "v_color", SHADER_TYPE::V_4_F32},
        {"v_texture_coord", SHADER_TYPE::V_2_F32},
        {       "v_normal", SHADER_TYPE::V_3_F32},
    };
    return result;
}

auto VertexFormat::calculate_offset_and_stride(std::vector<VertexAttribute>& attributes)
    -> void {
    size_t offset = 0;
    m_stride = 0;
    for (VertexAttribute& attribute : attributes) {
        attribute.offset = offset;
        offset += attribute.size;
        m_stride += attribute.size;
    }
}

/*---------------------------
    RenderSystem
---------------------------*/

RenderSystem::RenderSystem(GRAPHICS_API api, IWindow* window) {
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

auto RenderSystem::init(GRAPHICS_API api, IWindow* window) -> void {
    m_api = api;
    switch (api) {
        case GRAPHICS_API::OPENGL: {
            m_renderer = new OpenGL_Renderer(*this, window);
#if 0 // JF_OPENGL_FB
      //  NOTE: This is only a temporary fix. This work around stems from the fact
      //  taht `this->register_shader(..)` requires an already initialized
      //  `OpenGL_Renderer`.
            VertexFormat layout = {
                {           "v_position", SHADER_TYPE::V_3_F32},
                {"v_texture_coordinates", SHADER_TYPE::V_2_F32}
            };
            ShaderHandle::Desc shader_handle_desc;
            shader_handle_desc.shading_code =
                GLSLCodeLoader::get_by_name("framebuffer_test");
            shader_handle_desc.vertex_format = layout;

            auto* ogl_renderer = dynamic_cast<OpenGL_Renderer*>(m_renderer);
            ogl_renderer->fb.m_shader = this->register_shader(shader_handle_desc);

#endif

        } break;
        case GRAPHICS_API::VULKAN: {
            m_renderer = new Vulkan_Renderer(*this, window);
        } break;
        default: assert(false);
    }
}

RenderSystem::~RenderSystem() {}

RenderSystem::RenderSystem(RenderSystem&& other) noexcept {
    m_api = other.m_api;
    m_renderer = other.m_renderer;
    m_registered_textures = std::move(other.m_registered_textures);
    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_renderer = nullptr;
    other.m_registered_textures.clear();
}

auto RenderSystem::operator=(RenderSystem&& other) noexcept -> RenderSystem& {
    m_api = other.m_api;
    m_renderer = other.m_renderer;
    m_registered_textures = std::move(other.m_registered_textures);
    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_renderer = nullptr;
    other.m_registered_textures.clear();
    return *this;
}

auto RenderSystem::register_texture(TextureHandle&& texture) -> u32 {
    static u32 id = 1;
    m_registered_textures[id] = std::move(texture);
    m_registered_textures[id].m_api = m_api;

    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto*           renderer = dynamic_cast<OpenGL_Renderer*>(m_renderer);
            OpenGL_Context* device = &renderer->m_context;

            // m_registered_textures[id].init(&renderer->m_context);
            auto& t = m_registered_textures[id];
            t.m_handle = device->create_texture(t.m_data, t.m_size, t.m_num_components);

        } break;
        case GRAPHICS_API::VULKAN: {
            auto*                  renderer = dynamic_cast<Vulkan_Renderer*>(m_renderer);
            vulkan::LogicalDevice* device = renderer->m_logical_device;
            auto&                  t = m_registered_textures[id];

            VkFormat format = {};
            switch (t.m_num_components) {
                case 3: format = VK_FORMAT_R8G8B8_SRGB; break;
                case 4: format = VK_FORMAT_R8G8B8A8_SRGB; break;
                default:
                    Logger::err(
                        "Unsupported number of components: {}", t.m_num_components
                    );
                    assert(false);
            }
            t.m_handle = new vulkan::Vulkan_Texture(*device, t.m_data, t.m_size, format);
        } break;
        default: assert(false);
    }
    u32 old_id = id;
    id++;
    return old_id;
}

auto RenderSystem::register_shader(const ShaderHandle::Desc& desc) -> u32 {
    static u32 id = 1;

    m_registered_shaders[id].m_code = desc.shading_code;
    m_registered_shaders[id].m_vertex_format = desc.vertex_format;
    m_registered_shaders[id].m_api = m_api;
    // Right now all shaders are internally stored as SPIRV, usually coming from GLSL.
    // Even if in the end one ends up using GLSL code again, it always has this
    // intermediate step of being converted to SPIRV.
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto* ren = dynamic_cast<OpenGL_Renderer*>(m_renderer);
            auto* ctx = (OpenGL_Context*)&ren->m_context;

            opengl::Shader::Desc shader_desc;

            // TODO: Move this whole remapping thing into the OpenGL Shader class. This is
            // not a thing which should be known by a shared layer.

            std::string         v_source;
            ShadingCode::Module mod_0;
            const auto& v_code = m_registered_shaders[id].m_code.m_modules[0].m_code;
            mod_0.m_code = remap_for_opengl(v_code, SHADER_STAGE::VERTEX, &v_source);
            mod_0.m_stage = SHADER_STAGE::VERTEX;

            std::string         f_source;
            ShadingCode::Module mod_1;
            const auto& f_code = m_registered_shaders[id].m_code.m_modules[1].m_code;
            mod_1.m_code = remap_for_opengl(f_code, SHADER_STAGE::FRAGMENT, &f_source);
            mod_1.m_stage = SHADER_STAGE::FRAGMENT;

            shader_desc.code.m_modules.resize(2);
            shader_desc.code.m_modules[0] = std::move(mod_0);
            shader_desc.code.m_modules[1] = std::move(mod_1);
            shader_desc.vertex_format = m_registered_shaders[id].m_vertex_format;

            auto* shader = new opengl::Shader(*ctx, shader_desc);
            m_registered_shaders[id].m_handle = shader;

            Logger::warn("Vertex source:\n {}", v_source);
            Logger::warn("Fragment source:\n {}", f_source);
        } break;
        case GRAPHICS_API::VULKAN: {
            auto*                  ren = dynamic_cast<Vulkan_Renderer*>(m_renderer);
            vulkan::LogicalDevice* ctx = ren->m_logical_device;

            Vulkan_Shader::Desc shader_desc;
            shader_desc.code = m_registered_shaders[id].m_code;
            shader_desc.vertex_format = m_registered_shaders[id].m_vertex_format;

            auto* shader = new Vulkan_Shader(*ctx, *ren, shader_desc);
            m_registered_shaders[id].m_handle = shader;
        } break;
        default: assert(false);
    }

    u32 old_id = id;
    id++;
    return old_id;
}

/**
 * @brief Registers a mesh on the GPU and returns an id to it.
 *
 * @param format
 * @param data
 * @return u32
 */
auto RenderSystem::register_mesh(const VertexData& data) -> u32 {
    static u32 id = 1;

    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto* r = dynamic_cast<OpenGL_Renderer*>(m_renderer);
            r->m_registered_meshes[id] = opengl::GPUMeshData(r->m_context, data);
        } break;
        case GRAPHICS_API::VULKAN: {
            auto* r = dynamic_cast<Vulkan_Renderer*>(m_renderer);
            r->m_registered_meshes[id] = vulkan::GPUMeshData{*r->m_logical_device, data};
        } break;
        default: assert(false);
    }
    u32 old_id = id;
    id++;
    return old_id;
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
        default: JF_UNIMPLEMENTED(""); return "UNKNOWN";
    }
}

} // namespace JadeFrame