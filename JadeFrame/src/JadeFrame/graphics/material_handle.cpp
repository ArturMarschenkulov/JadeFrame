#include "material_handle.h"
#include "opengl/opengl_shader.h"
#include "opengl/opengl_texture.h"
#include "opengl/opengl_context.h"
#include "pch.h"
#include "stb/stb_image.h"
#include "vulkan/vulkan_buffer.h"
#include "vulkan/vulkan_shader.h"
#include "vulkan/vulkan_logical_device.h"
#include <cassert>

namespace JadeFrame {

TextureHandle::TextureHandle(const std::string& path) {
    // flip textures on their y coordinate while loading
    stbi_set_flip_vertically_on_load(true);
    // i32 width, height, num_components;
    v2i32 size;
    i32   num_components;

    m_data = stbi_load(path.c_str(), &size.width, &size.height, &num_components, 0);
    if (size.width >= 0 && size.height >= 0) {
        m_size.height = size.height;
        m_size.width = size.width;
        m_num_components = num_components;
        Logger::warn("Textire loaded at {}", fmt::ptr(m_data));
    } else {
        m_size = {0, 0};
        m_num_components = 0;
        Logger::warn("TextureHandle::TextureHandle: Failed to get size: {}", path.c_str());
    }
    if (m_data == nullptr) { Logger::err("Failed to load texture: {} ", path); }
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
            auto texture = new opengl::Texture(ctx.create_texture(m_data, m_size, m_num_components));
            // auto texture = std::make_unique<opengl::Texture>(ctx.create_texture(m_data, m_size, m_num_components));

            m_handle = texture;
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

ShaderHandle::ShaderHandle(const DESC& desc) {
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

auto ShaderHandle::init(void* context) -> void {

    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            opengl::Shader::DESC shader_desc;
            shader_desc.code = m_code;
            shader_desc.vertex_format = m_vertex_format;
            m_handle = new opengl::Shader(*(OpenGL_Context*)context, shader_desc);

        } break;
        case GRAPHICS_API::VULKAN: {
            Vulkan_Shader::DESC shader_desc;
            shader_desc.code = m_code;
            shader_desc.vertex_format = m_vertex_format;
            m_handle = new Vulkan_Shader(*(vulkan::LogicalDevice*)context, shader_desc);
        } break;
        default: assert(false);
    }
}

} // namespace JadeFrame