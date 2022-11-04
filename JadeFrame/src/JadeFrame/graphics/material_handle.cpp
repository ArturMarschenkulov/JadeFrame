#include "material_handle.h"
#include "opengl/opengl_shader.h"
#include "opengl/opengl_texture.h"
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
    m_data = stbi_load(path.c_str(), &m_size.width, &m_size.height, &m_num_components, 0);
    if (m_data == nullptr) { Logger::err("Failed to load texture: {} ", path); }
}

TextureHandle::~TextureHandle() { stbi_image_free(m_data); }

auto TextureHandle::init(void* context) -> void {


    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            GLenum format = {};
            switch (m_num_components) {
                case 3: format = GL_RGB; break;
                case 4: format = GL_RGBA; break;
                default:
                    Logger::err("TextureHandle::init() - Unsupported number of components: {}", m_num_components);
                    assert(false);
            }
            opengl::Texture* texture = new opengl::Texture(
                *(OpenGL_Context*)context, m_data, {static_cast<u32>(m_size.width), static_cast<u32>(m_size.height)}, format, format,
                GL_UNSIGNED_BYTE);
            m_handle = texture;


        } break;
        case GRAPHICS_API::VULKAN: {
            /*		Vulkan_Texture* texture = new Vulkan_Texture();
                            texture->init();*/
        } break;
        default: assert(false);
    }
}

ShaderHandle::ShaderHandle(const DESC& desc) {
    m_code = desc.shading_code;
    m_vertex_format = desc.vertex_format;
}

auto ShaderHandle::init(void* context) -> void {

    // for (auto& m : m_code.m_modules) {
    //	if ((m.m_stage == SHADER_STAGE::VERTEX) || (m.m_stage == SHADER_STAGE::FRAGMENT)) {
    //		if(std::holds_alternative<std::string>(m.m_code) ||
    // std::holds_alternative<std::vector<u32>>(m.m_code)) {
    //			if(std::get<std::string>(m.m_code).empty() ||
    // std::get<std::vector<u32>>(m.m_code).empty()) {
    //				__debugbreak();
    //			}
    //		}
    //	}
    // }
    switch (m_api) {
        case GRAPHICS_API::OPENGL: {
            opengl::Shader::DESC shader_desc;
            shader_desc.code = m_code;
            m_handle = new opengl::Shader(shader_desc);

        } break;
        case GRAPHICS_API::VULKAN: {
            Logger::warn("lnlknm,lm. ");
            Vulkan_Shader::DESC shader_desc;
            shader_desc.code = m_code;
            m_handle = new Vulkan_Shader(*(vulkan::LogicalDevice*)m_handle, shader_desc);
            // m_handle = new Vulkan_Shader({ vertex_shader_code, fragment_shader_code });
        } break;
        default: assert(false);
    }
}

auto MaterialHandle::init(void* context) const -> void {

    m_shader_handle->init(context);

    if (m_texture_handle != nullptr) { m_texture_handle->init(context); }
}

} // namespace JadeFrame