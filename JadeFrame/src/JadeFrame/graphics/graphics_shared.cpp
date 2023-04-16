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

/*---------------------------
    Image
---------------------------*/

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
static auto add_fourth_components(u8* data, i32 width, i32 height, i32 num_components) -> u8* {
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
    i32 width, height, num_components;
    u8* data = stbi_load(path.c_str(), &width, &height, &num_components, 0);

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
    Texture Handle
---------------------------*/

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
            auto                    ld = (vulkan::LogicalDevice*)m_handle;
            vulkan::Vulkan_Texture* texture = new vulkan::Vulkan_Texture(*ld, m_data, m_size, VK_FORMAT_R8G8B8A8_SRGB);
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

auto ShaderHandle::set_uniform(const std::string& name, const void* data, size_t size) -> void {
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            auto shader = (opengl::Shader*)m_handle;
            // shader->set_uniform(name, data, size);
        } break;
        case GRAPHICS_API::VULKAN: {
            auto shader = (Vulkan_Shader*)m_handle;
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

VertexAttribute::VertexAttribute(const std::string& name, SHADER_TYPE type, bool normalized)
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

auto VertexFormat::calculate_offset_and_stride(std::vector<VertexAttribute>& attributes) -> void {
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
            auto&    t = m_registered_textures[id];
            VkFormat format;
            switch (t.m_num_components) {
                case 3: format = VK_FORMAT_R8G8B8_SRGB; break;
                case 4: format = VK_FORMAT_R8G8B8A8_SRGB; break;
                default: assert(false);
            }
            t.m_handle = new vulkan::Vulkan_Texture(*ld, t.m_data, t.m_size, format);
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
            OpenGL_Renderer* ren = static_cast<OpenGL_Renderer*>(m_renderer);
            auto             ctx = (OpenGL_Context*)&ren->m_context;

            opengl::Shader::Desc shader_desc;
            shader_desc.code = ogl(m_registered_shaders[id].m_code);
            shader_desc.vertex_format = m_registered_shaders[id].m_vertex_format;

            opengl::Shader* shader = new opengl::Shader(*ctx, shader_desc);
            m_registered_shaders[id].m_handle = shader;
        } break;
        case GRAPHICS_API::VULKAN: {
            Vulkan_Renderer*       ren = static_cast<Vulkan_Renderer*>(m_renderer);
            vulkan::LogicalDevice* ctx = ren->m_logical_device;


            Vulkan_Shader::Desc shader_desc;
            shader_desc.code = m_registered_shaders[id].m_code;
            shader_desc.vertex_format = m_registered_shaders[id].m_vertex_format;

            Vulkan_Shader* shader = new Vulkan_Shader(*ctx, *ren, shader_desc);
            m_registered_shaders[id].m_handle = shader;
            for (int i = 0; i < shader->m_pipeline.m_set_layouts.size(); i++) {
                shader->m_sets[i] = ctx->m_set_pool.allocate_set(shader->m_pipeline.m_set_layouts[i]);
            }

            // TODO: Remove this hard coded code later on.
            shader->bind_buffer(0, 0, ren->m_ub_cam, 0, sizeof(Matrix4x4));
            shader->bind_buffer(3, 0, ren->m_ub_tran, 0, sizeof(Matrix4x4));

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
        vertex_format = VertexFormat::default_format();
    } else {
        vertex_format = format;
    }
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            OpenGL_Renderer* r = static_cast<OpenGL_Renderer*>(m_renderer);
            r->m_registered_meshes[id] = opengl::GPUMeshData(r->m_context, data, vertex_format);
        } break;
        case GRAPHICS_API::VULKAN: {
            Vulkan_Renderer* r = static_cast<Vulkan_Renderer*>(m_renderer);
            r->m_registered_meshes[id] = vulkan::GPUMeshData{*r->m_logical_device, data, vertex_format};
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