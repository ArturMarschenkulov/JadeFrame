#include "pch.h"
#include "graphics_shared.h"
#include "platform/platform_shared.h"
#include "vulkan/vulkan_renderer.h"
#include "vulkan/vulkan_shader.h"
#include "opengl/opengl_renderer.h"
JF_PRAGMA_PUSH
#pragma warning(disable : 4006)
#include "shaderc/shaderc.hpp"
JF_PRAGMA_POP

#include "SPIRV-Cross/spirv_glsl.hpp"
#include "SPIRV-Cross/spirv_hlsl.hpp"
#include "SPIRV-Cross/spirv_msl.hpp"

#include "JadeFrame/utils/assert.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"



namespace JadeFrame {

Image::~Image() {
    if (data != nullptr) { stbi_image_free(data); }
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



static auto ogl(const ShadingCode& code) -> ShadingCode;

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


// --------------------------------------------
// Here is reflection code found. Mainly spirv_cross and shaderc are used

static auto ogl(const ShadingCode& code) -> ShadingCode {

    spirv_cross::CompilerGLSL::Options options;
    options.version = 450;
    options.es = false;
    options.vulkan_semantics = true;
    spirv_cross::CompilerGLSL compiler = spirv_cross::CompilerGLSL(code.m_modules[0].m_code);

    compiler.set_common_options(options);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    std::array<std::vector<spirv_cross::Resource*>, 4> dd;
    for (u32 j = 0; j < resources.uniform_buffers.size(); j++) {
        spirv_cross::Resource& resource = resources.uniform_buffers[j];

        u32 set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        JF_ASSERT(set <= 3, "As of right now, only 4 descriptor sets are supported. (0, 1, 2, 3)");

        dd[set].push_back(&resource);
    }

    u32 binding = 0;
    for (u32 i = 0; i < dd.size(); i++) {
        for (u32 j = 0; j < dd[i].size(); j++) {
            // compiler.unset_decoration(dd[i][j]->id, spv::DecorationDescriptorSet);
            compiler.set_decoration(dd[i][j]->id, spv::DecorationDescriptorSet, 0);
            compiler.set_decoration(dd[i][j]->id, spv::DecorationBinding, binding);
            binding++;
        }
    }
    auto source = compiler.compile();

    auto new_code = code;
    new_code.m_modules[0].m_code = string_to_SPIRV(source, SHADER_STAGE::VERTEX, GRAPHICS_API::OPENGL);
    return new_code;
}


auto string_to_SPIRV(const std::string& code, SHADER_STAGE stage, GRAPHICS_API api) -> std::vector<u32> {
    shaderc_shader_kind kind = {};
    switch (stage) {
        case SHADER_STAGE::VERTEX: {
            kind = shaderc_vertex_shader;
        } break;
        case SHADER_STAGE::FRAGMENT: {
            kind = shaderc_fragment_shader;
        } break;
        default: assert(false);
    }

    shaderc::CompileOptions options;
    switch (api) {
        case GRAPHICS_API::VULKAN: {
            options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        } break;
        case GRAPHICS_API::OPENGL: {
            options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
        } break;
    }
    options.SetWarningsAsErrors();
    options.SetGenerateDebugInfo();
    const bool optimize = false;
    if constexpr (optimize == true) { options.SetOptimizationLevel(shaderc_optimization_level_size); }
    shaderc::Compiler             compiler;
    shaderc::SpvCompilationResult comp_result = compiler.CompileGlslToSpv(code, kind, "", options);
    shaderc_compilation_status    comp_status = comp_result.GetCompilationStatus();
    if (comp_status != shaderc_compilation_status_success) {
        assert(false);
        return std::vector<u32>();
    }

    std::vector<u32> result = {comp_result.cbegin(), comp_result.cend()};
    return result;
}

static auto debug_print_resources(const spirv_cross::ShaderResources& resources) -> void {
    Logger::info("printing shader resources");
    for (const spirv_cross::Resource& resource : resources.uniform_buffers) {
        const std::string& name = resource.name;
        Logger::info("\tuniform_buffers {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.storage_buffers) {
        const std::string& name = resource.name;
        Logger::info("\tstorage_buffers {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.stage_inputs) {
        const std::string& name = resource.name;
        Logger::info("\tstage_inputs {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.stage_outputs) {
        const std::string& name = resource.name;
        Logger::info("\tstage_outputs {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.subpass_inputs) {
        const std::string& name = resource.name;
        Logger::info("\tsubpass_inputs {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.storage_images) {
        const std::string& name = resource.name;
        Logger::info("\tstorage_images {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.sampled_images) {
        const std::string& name = resource.name;
        Logger::info("\tsampled_images {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.atomic_counters) {
        const std::string& name = resource.name;
        Logger::info("\tatomic_counters {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.acceleration_structures) {
        const std::string& name = resource.name;
        Logger::info("\tacceleration_structures {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.push_constant_buffers) {
        const std::string& name = resource.name;
        Logger::info("\tpush_constant_buffers {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.separate_images) {
        const std::string& name = resource.name;
        Logger::info("\tseparate_images {}", name);
    }
    for (const spirv_cross::Resource& resource : resources.separate_samplers) {
        const std::string& name = resource.name;
        Logger::info("\tseparate_samplers {}", name);
    }

    for (const spirv_cross::BuiltInResource& resource : resources.builtin_inputs) {
        const std::string& name = resource.resource.name;
        Logger::info("\tbuiltin_inputs {}", name);
    }
    for (const spirv_cross::BuiltInResource& resource : resources.builtin_outputs) {
        const std::string& name = resource.resource.name;
        Logger::info("\tbuiltin_outputs {}", name);
    }
}

static auto to_SHADER_TYPE(const spirv_cross::SPIRType& type, u32 rows, u32 columns) -> SHADER_TYPE {
    SHADER_TYPE result = SHADER_TYPE::NONE;
    if (rows > 4) { JF_PANIC("rows must be less than 4"); }
    if (columns > 4) { JF_PANIC("columns must be less than 4"); }


    if (columns == 1) {
        switch (type.basetype) {
            case spirv_cross::SPIRType::Float: {
                SHADER_TYPE arr[] = {SHADER_TYPE::F32, SHADER_TYPE::V_2_F32, SHADER_TYPE::V_3_F32, SHADER_TYPE::V_4_F32};
                result = arr[rows - 1];
            } break;
            default: JF_ASSERT(false, "this should not be reached!");
        }
    } else {
        switch (type.basetype) {
            if (columns == rows) {
                JF_UNIMPLEMENTED("matrix types with different row and column count are not supported yet!");
            }
            case spirv_cross::SPIRType::Float: {
                SHADER_TYPE arr[] = {SHADER_TYPE::M_2_2_F32, SHADER_TYPE::M_3_3_F32, SHADER_TYPE::M_4_4_F32};

                result = arr[rows - 2];
            } break;
            default: JF_ASSERT(false, "this should not be reached!");
        }
    }
    return result;
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

auto temp_cmp_0(const ReflectedCode::Input& i0, const ReflectedCode::Input& i1) -> bool {
    return i0.location < i1.location;
}
auto temp_cmp(ReflectedCode::Output i0, ReflectedCode::Output i1) -> bool { return i0.location < i1.location; }
auto reflect(const ShadingCode& code) -> ReflectedCode {
    ReflectedCode result = {};

    result.m_modules.resize(code.m_modules.size());
    for (u32 i = 0; i < code.m_modules.size(); i++) {
        auto& current_module = code.m_modules[i];
        auto& current_module_code = current_module.m_code;
        auto& current_result_module = result.m_modules[i];
        result.m_modules[i].m_stage = current_module.m_stage;

        spirv_cross::Compiler        compiler(current_module_code);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        debug_print_resources(resources);

        result.m_modules[i].m_inputs.resize(resources.stage_inputs.size());
        for (u32 j = 0; j < resources.stage_inputs.size(); j++) {
            const spirv_cross::Resource& resource = resources.stage_inputs[j];


            const std::string& name = resource.name;

            const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
            const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
            i32                          member_count = static_cast<u32>(buffer_type.member_types.size());
            u32                          location = compiler.get_decoration(resource.id, spv::DecorationLocation);
            u32                          size = (buffer_type.width / 8) * buffer_type.vecsize * buffer_type.columns;

            std::vector<ReflectedCode::Input>& inputs = current_result_module.m_inputs;
            inputs[j].name = name;
            inputs[j].location = location;
            inputs[j].size = size;
            inputs[j].type = to_SHADER_TYPE(buffer_type, buffer_type.vecsize, buffer_type.columns);
        }



        result.m_modules[i].m_outputs.resize(resources.stage_inputs.size());
        for (u32 j = 0; j < resources.stage_outputs.size(); j++) {
            const spirv_cross::Resource& resource = resources.stage_outputs[j];

            const std::string& name = resource.name;



            const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
            const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
            i32                          member_count = static_cast<u32>(buffer_type.member_types.size());
            u32                          location = compiler.get_decoration(resource.id, spv::DecorationLocation);
            u32                          size = (buffer_type.width / 8) * buffer_type.vecsize * buffer_type.columns;

            std::vector<ReflectedCode::Output>& outputs = current_result_module.m_outputs;
            outputs[j].name = name;
            outputs[j].location = location;
            outputs[j].size = size;
            outputs[j].type = to_SHADER_TYPE(buffer_type, buffer_type.vecsize, buffer_type.columns);
        }

        result.m_modules[i].m_uniform_buffers.resize(resources.uniform_buffers.size());
        for (u32 j = 0; j < resources.uniform_buffers.size(); j++) {
            const spirv_cross::Resource& resource = resources.uniform_buffers[j];

            const std::string&           name = resource.name;
            const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
            const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);

            Logger::info(
                "the uniform buffer {} has {} members. base id {}, id {}", name, buffer_type.member_types.size(),
                resource.base_type_id, resource.type_id);

            u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            u32 set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            u32 size = static_cast<u32>(compiler.get_declared_struct_size(buffer_type));

            std::vector<ReflectedCode::UniformBuffer>& uniform_buffers = current_result_module.m_uniform_buffers;
            uniform_buffers[j].binding = binding;
            uniform_buffers[j].set = set;
            uniform_buffers[j].name = name;
            uniform_buffers[j].size = size;

            JF_ASSERT(base_type.member_types.size() > 0, "this uniform buffer doesn't have any members");
            for (u32 jj = 0; jj < base_type.member_types.size(); jj++) {
                const spirv_cross::SPIRType& member_type = compiler.get_type(base_type.member_types[jj]);
                const std::string&           member_name = compiler.get_member_name(resource.base_type_id, jj);
                u32 member_size = static_cast<u32>(compiler.get_declared_struct_member_size(buffer_type, jj));
                u32 member_offset = static_cast<u32>(compiler.type_struct_member_offset(buffer_type, jj));


                ReflectedCode::UniformBuffer::Member member = {};
                member.name = member_name;
                member.size = member_size;
                member.offset = member_offset;
                member.type = to_SHADER_TYPE(member_type, member_type.vecsize, member_type.columns);
                Logger::info(
                    "\tthe member {}.{} has, type {}, size {} and offset {}", name, member_name, to_string(member.type),
                    member_size, member_offset);

                uniform_buffers[j].members.push_back(member);
            }
            compiler.get_member_name(resource.base_type_id, 0);
            compiler.get_declared_struct_size(compiler.get_type(resource.base_type_id));
        }


        result.m_modules[i].m_sampled_images.resize(resources.sampled_images.size());
        for (u32 j = 0; j < resources.sampled_images.size(); j++) {
            const spirv_cross::Resource& resource = resources.sampled_images[j];

            const std::string&           name = resource.name;
            const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
            const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
            i32                          member_count = static_cast<u32>(buffer_type.member_types.size());
            u32                          binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
            u32                          set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

            u32 dimension = base_type.image.dim;
            u32 array_size = buffer_type.array[0];

            if (array_size == 0) {
                array_size = 1;
            } else {
                assert(false);
            }
            std::vector<ReflectedCode::SampledImage>& sampled_images = current_result_module.m_sampled_images;
            sampled_images[j].binding = binding;
            sampled_images[j].set = set;
            sampled_images[j].name = name;
        }
        std::sort(current_result_module.m_inputs.begin(), current_result_module.m_inputs.end(), temp_cmp_0);
        // std::sort(current_result_module.m_outputs.begin(), current_result_module.m_outputs.end(), temp_cmp);



        // result.m_modules[i].m_push_constant_ranges.resize(resources.push_constant_buffers.size());
        // for (u32 j = 0; j < resources.push_constant_buffers.size(); j++) {
        //     const spirv_cross::Resource& resource = resources.push_constant_buffers[j];

        //     const std::string&           buffer_name = resource.name;
        //     const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.base_type_id);
        //     u32                          buffer_size = (u32)compiler.get_declared_struct_size(buffer_type);
        //     u32                          member_count = uint32_t(buffer_type.member_types.size());
        //     u32                          buffer_offset = 0;

        //     std::vector<VkPushConstantRange>& push_constant_ranges = current_result_module.m_push_constant_ranges;
        //     push_constant_ranges[j].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        //     push_constant_ranges[j].size = buffer_size - buffer_offset;
        //     push_constant_ranges[j].offset = buffer_offset;
        // }
    }
    return result;
}
} // namespace JadeFrame