#include "pch.h"
#include "graphics_shared.h"
#include "JadeFrame/platform/platform_shared.h"
#include "vulkan/vulkan_renderer.h"
#include "vulkan/vulkan_shader.h"
#include "opengl/opengl_renderer.h"

#include "JadeFrame/utils/assert.h"
#include "reflect.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"



namespace JadeFrame {

Image::~Image() {
    if (data != nullptr) { stbi_image_free(data); }
}
Image::Image(Image&& other) noexcept {
    data = other.data;
    width = other.width;
    height = other.height;
    num_components = other.num_components;
    other.data = nullptr;
}
Image& Image::operator=(Image&& other) noexcept {
    data = other.data;
    width = other.width;
    height = other.height;
    num_components = other.num_components;
    other.data = nullptr;
    return *this;
}

auto Image::load(const std::string& path) -> Image {
    stbi_set_flip_vertically_on_load(true);
    i32 width, height, num_components;
    u8* data = stbi_load(path.c_str(), &width, &height, &num_components, 0);

    Image img;
    img.data = data;
    img.width = width;
    img.height = height;
    img.num_components = num_components;
    return img;
}

TextureHandle::TextureHandle(const Image& img) {
    m_data = img.data;
    m_size.height = img.height;
    m_size.width = img.width;
    m_num_components = img.num_components;
}
TextureHandle::TextureHandle(TextureHandle&& other) {
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
    // *this = std::move(other);
}

auto TextureHandle::operator=(TextureHandle&& other) -> TextureHandle& {
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
            auto& ctx = *(OpenGL_Context*)context;
            m_handle = ctx.create_texture(m_data, m_size, m_num_components);
        } break;
        case GRAPHICS_API::VULKAN: {
            vulkan::Vulkan_Texture* texture = new vulkan::Vulkan_Texture();
            auto                    ld = (vulkan::LogicalDevice*)m_handle;
            texture->init(*ld, m_data, m_size, VK_FORMAT_R8G8B8A8_SRGB);
            m_handle = texture;
        } break;
        default: assert(false);
    }
}

ShaderHandle::ShaderHandle(const Desc& desc) {
    m_code = desc.shading_code;
    m_vertex_format = desc.vertex_format;
}
ShaderHandle::ShaderHandle(ShaderHandle&& other) {
    m_code = other.m_code;
    m_vertex_format = other.m_vertex_format;
    m_api = other.m_api;
    m_handle = other.m_handle;

    // other.m_code = nullptr;
    // other.m_vertex_format = nullptr;
    // other.m_api = GRAPHICS_API::UNDEFINED;
    // other.m_handle = nullptr;
}
auto ShaderHandle::operator=(ShaderHandle&& other) -> ShaderHandle& {
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

VertexAttribute::VertexAttribute(const std::string& name, SHADER_TYPE type, bool normalized)
    : name(name)
    , type(type)
    , size(SHADER_TYPE_get_size(type))
    , offset(0)
    , normalized(normalized) {}

VertexFormat::VertexFormat(const std::initializer_list<VertexAttribute>& attributes)
    : m_attributes(attributes) {
    this->calculate_offset_and_stride(m_attributes);
}
auto VertexFormat::calculate_offset_and_stride(std::vector<VertexAttribute>& attributes) -> void {
    size_t offset = 0;
    m_stride = 0;
    for (VertexAttribute& attribute : attributes) {
        attribute.offset = offset;
        offset += attribute.size;
        m_stride += attribute.size;
    }
}




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

        } break;
        case GRAPHICS_API::VULKAN: {
            m_renderer = new Vulkan_Renderer(*this, window);
        } break;
        default: assert(false);
    }
}

RenderSystem::~RenderSystem() {}

RenderSystem::RenderSystem(RenderSystem&& other) {
    m_api = other.m_api;
    m_renderer = other.m_renderer;
    m_registered_textures = std::move(other.m_registered_textures);
    other.m_api = GRAPHICS_API::UNDEFINED;
    other.m_renderer = nullptr;
    other.m_registered_textures.clear();
}
auto RenderSystem::operator=(RenderSystem&& other) -> RenderSystem& {
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
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            OpenGL_Renderer* renderer = static_cast<OpenGL_Renderer*>(m_renderer);
            m_registered_textures[id].m_api = m_api;
            // m_registered_textures[id].init(&renderer->m_context);
            auto& t = m_registered_textures[id];
            auto& ctx = renderer->m_context;
            t.m_handle = ctx.create_texture(t.m_data, t.m_size, t.m_num_components);

        } break;
        case GRAPHICS_API::VULKAN: {
            Vulkan_Renderer* renderer = static_cast<Vulkan_Renderer*>(m_renderer);
            auto             ld = renderer->m_logical_device;
            m_registered_textures[id].m_api = m_api;
            // m_registered_textures[id].init(ld);
            auto& t = m_registered_textures[id];

            vulkan::Vulkan_Texture* texture = new vulkan::Vulkan_Texture();
            texture->init(*ld, t.m_data, t.m_size, VK_FORMAT_R8G8B8A8_SRGB);
            t.m_handle = texture;
        } break;
        default: assert(false);
    }
    u32 old_id = id;
    id++;
    return old_id;
}

auto RenderSystem::register_shader(const ShaderHandle::Desc& shader_desc) -> u32 {
    static u32 id = 1;

    m_registered_shaders[id].m_code = shader_desc.shading_code;
    m_registered_shaders[id].m_vertex_format = shader_desc.vertex_format;
    m_registered_shaders[id].m_api = m_api;


    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            OpenGL_Renderer*     r = static_cast<OpenGL_Renderer*>(m_renderer);
            opengl::Shader::Desc shader_desc;
            shader_desc.code = ogl(m_registered_shaders[id].m_code);
            shader_desc.vertex_format = m_registered_shaders[id].m_vertex_format;


            m_registered_shaders[id].m_handle = new opengl::Shader(*(OpenGL_Context*)&r->m_context, shader_desc);
        } break;
        case GRAPHICS_API::VULKAN: {
            Vulkan_Renderer* r = static_cast<Vulkan_Renderer*>(m_renderer);

            auto ld = r->m_logical_device;

            Vulkan_Shader::Desc shader_desc;
            shader_desc.code = m_registered_shaders[id].m_code;
            shader_desc.vertex_format = m_registered_shaders[id].m_vertex_format;
            m_registered_shaders[id].m_handle = new Vulkan_Shader(*(vulkan::LogicalDevice*)ld, shader_desc);


            auto* sh = (Vulkan_Shader*)m_registered_shaders[id].m_handle;
            for (int i = 0; i < sh->m_pipeline.m_set_layouts.size(); i++) {
                r->m_sets[i] = r->m_set_pool.allocate_set(sh->m_pipeline.m_set_layouts[i]);
            }

            // TODO: Remove this hard coded code later on
            r->m_sets[0].bind_uniform_buffer(0, r->m_ub_cam, 0, sizeof(Matrix4x4));
            r->m_sets[3].bind_uniform_buffer(0, r->m_ub_tran, 0, sizeof(Matrix4x4));
            for (int i = 0; i < r->m_sets.size(); i++) { r->m_sets[i].update(); }

        } break;
        default: assert(false);
    }
    u32 old_id = id;
    id++;
    return old_id;
}
auto RenderSystem::register_mesh(const VertexFormat& format, const VertexData& data) -> u32 {
    static u32   id = 1;
    VertexFormat vertex_format;
    // In case there is no buffer layout provided use a default one
    if (format.m_attributes.size() == 0) {
        Logger::warn("No vertex format provided, using default one. (v_position float3, v_color float4, "
                     "v_texture_coord float2, v_normal float3");
        const VertexFormat vf = {
            {     "v_position", SHADER_TYPE::V_3_F32},
            {        "v_color", SHADER_TYPE::V_4_F32},
            {"v_texture_coord", SHADER_TYPE::V_2_F32},
            {       "v_normal", SHADER_TYPE::V_3_F32},
        };
        vertex_format = vf;
    } else {
        vertex_format = format;
    }
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {

            OpenGL_Renderer* renderer = static_cast<OpenGL_Renderer*>(m_renderer);
            renderer->m_registered_meshes[id] = opengl::GPUMeshData(renderer->m_context, data, vertex_format);
        } break;
        case GRAPHICS_API::VULKAN: {
            Vulkan_Renderer* renderer = static_cast<Vulkan_Renderer*>(m_renderer);
            renderer->m_registered_meshes[id] =
                vulkan::Vulkan_GPUMeshData{*renderer->m_logical_device, data, vertex_format};
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