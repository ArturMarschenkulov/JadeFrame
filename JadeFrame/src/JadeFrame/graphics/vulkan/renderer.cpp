#include "renderer.h"
#if defined(_WIN32)
    #include "JadeFrame/platform/windows/windows_window.h"
#elif defined(__linux__)
    #include "JadeFrame/platform/linux/linux_window.h"
#endif

#include "JadeFrame/utils/logger.h"
#include "../graphics_shared.h"
#include "../graphics_language.h"
#include "buffer.h"
#include "shader.h"

#include <algorithm>
#include <bit>
#include <limits>
#include <memory>

namespace JadeFrame {

namespace vulkan {

auto to_viewport_state(
    const ViewportRect& rect,
    VkExtent2D          framebuffer,
    VkExtent2D          max_dimensions
) -> ViewportState {
    // Scissor: the rectangle clipped to the framebuffer, converted to Vulkan's
    // top-left origin.
    const u64 x0 = std::min<u64>(rect.x, framebuffer.width);
    const u64 y0 = std::min<u64>(rect.y, framebuffer.height);
    const u64 x1 = std::min<u64>(u64{rect.x} + rect.width, framebuffer.width);
    const u64 y1 = std::min<u64>(u64{rect.y} + rect.height, framebuffer.height);

    const VkRect2D scissor = {
        .offset = {static_cast<i32>(x0), static_cast<i32>(framebuffer.height - y1)},
        .extent = {static_cast<u32>(x1 - x0), static_cast<u32>(y1 - y0)},
    };

    // Viewport: not clipped, since clipping it would change the mapping of normalized
    // device coordinates. Only brought into the range Vulkan accepts. Keeping the
    // origin below the maximum dimension keeps every edge inside viewportBoundsRange,
    // which is at least [-2 * max, 2 * max - 1].
    const u32 max_width = std::max(max_dimensions.width, 1U);
    const u32 max_height = std::max(max_dimensions.height, 1U);
    const u32 width = std::clamp(rect.width, 1U, max_width);
    const u32 height = std::clamp(rect.height, 1U, max_height);
    const u32 x = std::min(rect.x, max_width - 1);
    const u32 y = std::min(rect.y, max_height - 1);

    const VkViewport viewport = {
        .x = static_cast<f32>(x),
        // The lower edge of the rectangle, seen from the top of the framebuffer. From
        // there the negative height extends the viewport upwards.
        .y = static_cast<f32>(framebuffer.height) - static_cast<f32>(y),
        .width = static_cast<f32>(width),
        .height = -static_cast<f32>(height),
        .minDepth = 0.0F,
        .maxDepth = 1.0F,
    };
    return ViewportState{.viewport = viewport, .scissor = scissor};
}

auto readback_to_rgb(
    std::span<const u8> texels,
    u32                 width,
    u32                 height,
    VkFormat            format,
    std::vector<u8>&    rgb
) -> bool {
    // Byte offsets of red, green and blue inside a texel.
    size_t red = 0;
    size_t blue = 0;
    switch (format) {
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
            red = 2;
            blue = 0;
            break;
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_UNORM:
            red = 0;
            blue = 2;
            break;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
            // Packed formats are defined in terms of a host-endian u32.
            if (std::endian::native != std::endian::little) { return false; }
            red = 0;
            blue = 2;
            break;
        default: return false;
    }

    const size_t texel_size = 4;
    const size_t row_texels = width;
    const size_t rows = height;
    if (texels.size() / texel_size < row_texels * rows) { return false; }

    rgb.resize(row_texels * rows * 3);
    for (size_t row = 0; row < rows; ++row) {
        const u8* src = texels.data() + row * row_texels * texel_size;
        u8*       dst = rgb.data() + (rows - 1 - row) * row_texels * 3;
        for (size_t i = 0; i < row_texels; ++i) {
            dst[i * 3 + 0] = src[i * texel_size + red];
            dst[i * 3 + 1] = src[i * texel_size + 1];
            dst[i * 3 + 2] = src[i * texel_size + blue];
        }
    }
    return true;
}

} // namespace vulkan

static auto to_vulkan(GPUBuffer::TYPE type) -> vulkan::Buffer::TYPE {
    switch (type) {
        case GPUBuffer::TYPE::VERTEX: return vulkan::Buffer::TYPE::VERTEX;
        case GPUBuffer::TYPE::INDEX: return vulkan::Buffer::TYPE::INDEX;
        case GPUBuffer::TYPE::UNIFORM: return vulkan::Buffer::TYPE::UNIFORM;
        case GPUBuffer::TYPE::STAGING: return vulkan::Buffer::TYPE::STAGING;
        default: assert(false); return vulkan::Buffer::TYPE::VERTEX;
    }
}

static auto delete_vulkan_texture(void* handle) noexcept -> void {
    delete static_cast<vulkan::Vulkan_Texture*>(handle);
}

static auto delete_vulkan_shader(void* handle) noexcept -> void {
    delete static_cast<Vulkan_Shader*>(handle);
}

static auto delete_vulkan_material(void* handle) noexcept -> void {
    delete static_cast<Vulkan_Material*>(handle);
}
static const i32 MAX_FRAMES_IN_FLIGHT = 4;

static auto resource_error(std::string message) -> Result<void, InitError> {
    return err<void, InitError>(InitError{
        InitErrorCode::RESOURCE_CREATION_FAILED,
        GRAPHICS_API::VULKAN,
        std::move(message),
    });
}

Vulkan_Renderer::Vulkan_Renderer(RenderSystem& system, Window* window)
    : m_logical_device(&m_context.m_instance.m_logical_device)
    , m_system(&system)
    , m_window(window) {}

auto Vulkan_Renderer::initialize() -> Result<void, InitError> {
    // Every step stores what it creates in a member. If a later step fails, the
    // destructor releases them in reverse declaration order, the context last.
    auto context = m_context.init(m_window);
    if (context.is_err()) { return context; }

    auto swapchain = m_swapchain.init(*m_logical_device, m_window);
    if (swapchain.is_err()) { return swapchain; }

    m_frames.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (!m_frames[i].init(m_logical_device)) {
            return resource_error(
                "failed to create the synchronization objects and command buffer of a "
                "frame"
            );
        }
    }

    auto dependents = this->create_swapchain_dependents();
    if (dependents.is_err()) { return dependents; }

    m_viewport = vulkan::ViewportRect{
        .x = 0,
        .y = 0,
        .width = m_swapchain.m_extent.width,
        .height = m_swapchain.m_extent.height,
    };
    return ok<void, InitError>();
}

auto Vulkan_Renderer::create_swapchain_dependents() -> Result<void, InitError> {
    m_framebuffers.clear();

    m_render_pass = m_logical_device->create_render_pass(m_swapchain.m_image_format);
    if (m_render_pass.m_handle == VK_NULL_HANDLE) {
        return resource_error("failed to create the render pass");
    }

    this->create_present_semaphores();
    for (const vulkan::Semaphore& semaphore : m_present_semaphores) {
        if (semaphore.m_handle == VK_NULL_HANDLE) {
            return resource_error("failed to create a present semaphore");
        }
    }

    const u32 swapchain_image_amount = static_cast<u32>(m_swapchain.m_images.size());
    m_framebuffers.resize(swapchain_image_amount);
    for (size_t i = 0; i < swapchain_image_amount; i++) {
        m_framebuffers[i] = m_logical_device->create_framebuffer(
            m_swapchain.m_image_views[i],
            m_swapchain.m_depth_image_view,
            m_render_pass,
            m_swapchain.m_extent
        );
        if (m_framebuffers[i].m_handle == VK_NULL_HANDLE) {
            return resource_error(
                "failed to create the framebuffer of swapchain image " +
                std::to_string(i)
            );
        }
    }
    return ok<void, InitError>();
}

auto Vulkan_Renderer::create_buffer(
    GPUBuffer::TYPE type,
    const void*     data,
    size_t          size
) -> void* {
    if (!m_logical_device || !size || type > GPUBuffer::STAGING ||
        ((type == GPUBuffer::VERTEX || type == GPUBuffer::INDEX) && !data) ||
        (type == GPUBuffer::INDEX && size % sizeof(u32) != 0)) {
        return nullptr;
    }
    return m_logical_device->create_buffer(
        to_vulkan(type), const_cast<void*>(data), size
    );
}

auto Vulkan_Renderer::destroy_buffer(void* handle) -> void {
    if (handle == nullptr || m_logical_device == nullptr) { return; }
    m_logical_device->destroy_buffer(static_cast<vulkan::Buffer*>(handle));
}

auto Vulkan_Renderer::create_texture(const Image& image) -> NativeHandle {
    if (validate_texture_image(image).is_err()) {
        return {nullptr, noop_native_handle_deleter};
    }
    if (m_logical_device == nullptr) {
        return NativeHandle(nullptr, noop_native_handle_deleter);
    }

    VkFormat format = VK_FORMAT_UNDEFINED;
    switch (image.num_components) {
        case 3: format = VK_FORMAT_R8G8B8_SRGB; break;
        case 4: format = VK_FORMAT_R8G8B8A8_SRGB; break;
        default:
            Logger::err("Unsupported number of texture components: {}", image.num_components);
            return NativeHandle(nullptr, noop_native_handle_deleter);
    }

    const auto size = v2u32::create(
        static_cast<u32>(image.width), static_cast<u32>(image.height)
    );
    auto texture = std::make_unique<vulkan::Vulkan_Texture>(
        *m_logical_device, const_cast<u8*>(image.data.data()), size, format
    );
    if (!texture->m_is_initialized) { return {nullptr, noop_native_handle_deleter}; }
    return NativeHandle(texture.release(), delete_vulkan_texture);
}

auto Vulkan_Renderer::create_shader(const ShadingCode& code) -> NativeHandle {
    if (validate_graphics_shader(code).is_err()) {
        return {nullptr, noop_native_handle_deleter};
    }
    if (m_logical_device == nullptr) {
        return NativeHandle(nullptr, noop_native_handle_deleter);
    }

    Vulkan_Shader::Desc shader_desc;
    shader_desc.code = code;
    auto shader = std::make_unique<Vulkan_Shader>(*m_logical_device, *this, shader_desc);
    if (!shader->m_pipeline.m_is_compiled) {
        // The pipeline constructor already logged the reason.
        Logger::err("Cannot create a Vulkan shader, its graphics pipeline is unusable");
        return NativeHandle(nullptr, noop_native_handle_deleter);
    }
    return NativeHandle(shader.release(), delete_vulkan_shader);
}

auto Vulkan_Renderer::create_material(
    const ShaderHandle&  shader,
    const TextureHandle* texture
) -> NativeHandle {
    if (m_logical_device == nullptr) {
        return NativeHandle(nullptr, noop_native_handle_deleter);
    }

    if (shader.m_api != GRAPHICS_API::VULKAN ||
        (texture && texture->m_api != GRAPHICS_API::VULKAN)) {
        return {nullptr, noop_native_handle_deleter};
    }
    auto* native_shader = static_cast<Vulkan_Shader*>(shader.m_handle.get());
    auto* native_texture = texture == nullptr
                               ? nullptr
                               : static_cast<vulkan::Vulkan_Texture*>(
                                     texture->m_handle.get()
                                 );
    if (native_shader == nullptr || (texture != nullptr && native_texture == nullptr)) {
        Logger::err("Cannot create a Vulkan material from invalid resources");
        return NativeHandle(nullptr, noop_native_handle_deleter);
    }

    auto material = std::make_unique<Vulkan_Material>(
        *m_logical_device, *native_shader, native_texture
    );
    if (!material->m_is_initialized) { return {nullptr, noop_native_handle_deleter}; }
    return NativeHandle(material.release(), delete_vulkan_material);
}

auto Vulkan_Renderer::set_shader_uniform(
    void*              /*shader*/,
    const std::string& /*name*/,
    const void*        /*data*/,
    size_t             /*size*/
) -> Result<void, RendererError> {
    // Vulkan has no per-shader uniform storage. Uniform data lives in uniform buffers
    // that are bound through the descriptor sets of a `Vulkan_Material` and written by
    // `render()`. A pipeline alone has nothing a named value could be written to.
    return err<void, RendererError>(RendererError{
        RendererErrorCode::UNSUPPORTED,
        "Vulkan: named shader uniforms are not supported, uniform data is provided "
        "through the uniform buffers of a material",
    });
}

Vulkan_Renderer::~Vulkan_Renderer() { this->wait_until_idle(); }

auto Vulkan_Renderer::wait_until_idle() -> void {
    if (m_logical_device == nullptr) { return; }
    m_logical_device->wait_until_idle();
}

auto Vulkan_Renderer::create_present_semaphores() -> void {
    // Replacing them also discards semaphores that were signaled for a present which
    // then failed. The device has to be idle when this is called on recreation.
    m_present_semaphores.clear();
    m_present_semaphores.reserve(m_swapchain.m_images.size());
    for (size_t i = 0; i < m_swapchain.m_images.size(); i++) {
        m_present_semaphores.push_back(m_logical_device->create_semaphore());
    }
}

auto Vulkan_Renderer::recreate_swapchain() -> void {
    m_logical_device->wait_until_idle();

    m_framebuffers.clear();
    // After a failed attempt the swapchain is deinitialized and has to be initialized
    // from scratch.
    auto result = m_swapchain.m_device == nullptr
                      ? m_swapchain.init(*m_logical_device, m_window)
                      : m_swapchain.recreate();
    if (result.is_ok() && m_swapchain.m_handle == VK_NULL_HANDLE) {
        // No drawable area (minimized window). `render()` retries every frame.
        m_present_semaphores.clear();
        m_swapchain_recreation_failed = false;
        return;
    }
    if (result.is_ok()) { result = this->create_swapchain_dependents(); }

    if (result.is_err()) {
        // Without a swapchain `render()` drops the frame and tries again with the next
        // one. Report the first failure of a streak only.
        if (!m_swapchain_recreation_failed) {
            Logger::err(
                "Failed to recreate the Vulkan swapchain, frames are dropped until it "
                "succeeds: {}",
                to_string(result.error())
            );
        }
        m_swapchain_recreation_failed = true;
        m_framebuffers.clear();
        m_present_semaphores.clear();
        m_swapchain.deinit();
        return;
    }
    m_swapchain_recreation_failed = false;
}

auto Vulkan_Renderer::set_clear_color(const RGBAColor& color) -> void {
    m_clear_color = color;
}

auto Vulkan_Renderer::clear_background() -> void {}

auto Vulkan_Renderer::render(const Camera& camera) -> void {
    vulkan::LogicalDevice&        d = *m_logical_device;

    if (m_swapchain.m_handle == VK_NULL_HANDLE) {
        // The window had no drawable area when the swapchain was last (re)created.
        this->recreate_swapchain();
        if (m_swapchain.m_handle == VK_NULL_HANDLE) {
            // Still nothing to draw to, drop this frame.
            m_system->m_render_commands.clear();
            m_skip_present = true;
            return;
        }
    }

    auto& curr_frame = m_frames[m_frame_index];
    curr_frame.acquire_image(m_swapchain);

    if (m_swapchain.m_is_recreated) {
        m_swapchain.m_is_recreated = false;
        this->recreate_swapchain();
        m_skip_present = true;
        return;
    }

    std::deque<RenderCommand>& render_commands = m_system->m_render_commands;
    // prepare shaders and its dynamic uniform buffers
    // TODO: Find a better way to do this, but for now it works
    for (u64 i = 0; i < render_commands.size(); i++) {
        const auto&           cmd = render_commands[i];
        const MaterialHandle& mh = *cmd.material;
        auto*                 material = static_cast<Vulkan_Material*>(mh.m_handle.get());
        material->set_dynamic_ub_num(render_commands.size());
    }

    vulkan::CommandBuffer& cb = curr_frame.m_cmd;
    // cb.record([&] {
    cb.record_begin();
    vulkan::Framebuffer& framebuffer = m_framebuffers[curr_frame.m_index];
    const RGBAColor      c = m_clear_color;
    const VkClearValue   clear_value = VkClearValue{{{c.r, c.g, c.b, c.a}}};

    // cb.render_pass(framebuffer, m_render_pass, m_swapchain.m_extent, clear_value, [&] {
    cb.render_pass_begin(framebuffer, m_render_pass, m_swapchain.m_extent, clear_value);

    // Every pipeline leaves viewport and scissor dynamic, so setting them once is valid
    // for all draws of this command buffer. Deriving them from the current extent keeps
    // them correct across swapchain recreation.
    const auto& limits = d.m_physical_device->limits();
    const vulkan::ViewportState viewport_state = vulkan::to_viewport_state(
        m_viewport,
        m_swapchain.m_extent,
        VkExtent2D{limits.maxViewportDimensions[0], limits.maxViewportDimensions[1]}
    );
    cb.set_viewport(viewport_state.viewport);
    cb.set_scissor(viewport_state.scissor);
    for (u64 i = 0; i < render_commands.size(); i++) {
        using namespace vulkan;
        const RenderCommand& cmd = render_commands[i];
        MaterialHandle&      mh = *cmd.material;
        auto*                material = static_cast<Vulkan_Material*>(mh.m_handle.get());

        const VkPipelineBindPoint bp = VK_PIPELINE_BIND_POINT_GRAPHICS;
        vulkan::Pipeline&         pl = material->m_shader->m_pipeline;
        auto&                     sets = material->m_sets;
        if (!pl.m_is_compiled) {
            Logger::err("Skipping a render command with an uncompiled Vulkan pipeline");
            continue;
        }
        cb.bind_pipeline(bp, pl);

        const auto* bg_cam = mh.m_info.get_bind_group_by_name("Camera");
        const auto* bg_tran = mh.m_info.get_bind_group_by_name("Transform");
        if (bg_cam == nullptr || bg_tran == nullptr) {
            Logger::err("Material is missing required bind groups.");
            continue;
        }
        if (bg_cam->m_set != static_cast<u32>(vulkan::FREQUENCY::PER_FRAME) ||
            bg_tran->m_set != static_cast<u32>(vulkan::FREQUENCY::PER_OBJECT)) {
            Logger::err("Camera and Transform bind groups use incompatible descriptor sets");
            continue;
        }

        mat4x4 cam = camera.get_view_projection(GRAPHICS_API::VULKAN);
        auto   cam_set = static_cast<FREQUENCY>(bg_cam->m_set);
        auto   cam_binding = bg_cam->m_binding;
        material->write_ub(cam_set, cam_binding, &cam, sizeof(cam), 0);

        const auto& tran = cmd.transform;
        auto        tran_set = static_cast<FREQUENCY>(bg_tran->m_set);
        auto        tran_binding = bg_tran->m_binding;
        const VkDeviceSize transform_stride =
            material->get_dynamic_uniform_stride(bg_tran->m_set, bg_tran->m_binding);
        const u64 transform_offset = transform_stride * i;
        if (transform_stride == 0 || transform_offset > std::numeric_limits<u32>::max()) {
            Logger::err("Transform binding is not a valid dynamic uniform buffer");
            continue;
        }
        const u32 dyn_offset = static_cast<u32>(transform_offset);
        material->write_ub(tran_set, tran_binding, &tran, sizeof(tran), dyn_offset);

        for (u32 set_index = 0; set_index < pl.m_set_layouts.size(); ++set_index) {
            if (set_index >= sets.size()) {
                Logger::err("Material is missing descriptor set {}", set_index);
                continue;
            }

            std::vector<u32> dynamic_offsets;
            const auto&      set_layout = pl.m_set_layouts[set_index];
            dynamic_offsets.reserve(set_layout.m_dynamic_count);
            bool valid_dynamic_bindings = true;
            for (const auto& binding : set_layout.m_bindings) {
                if (binding.descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC &&
                    binding.descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
                    continue;
                }

                const VkDeviceSize stride =
                    material->get_dynamic_uniform_stride(set_index, binding.binding);
                const u64 offset = stride * i;
                if (stride == 0 || offset > std::numeric_limits<u32>::max()) {
                    Logger::err(
                        "Missing or invalid dynamic uniform binding {} in descriptor set {}",
                        binding.binding,
                        set_index
                    );
                    valid_dynamic_bindings = false;
                    break;
                }
                dynamic_offsets.push_back(static_cast<u32>(offset));
            }
            if (!valid_dynamic_bindings) { continue; }
            cb.bind_descriptor_set(bp, pl, set_index, sets[set_index], dynamic_offsets);
        }

        this->render_mesh(cmd.m_mesh);
    }
    //});
    cb.render_pass_end();
    //});
    cb.record_end();

    curr_frame.submit(d.m_graphics_queue, m_present_semaphores[curr_frame.m_index]);

    render_commands.clear();
    m_frame_rendered = true;
}

auto Vulkan_Renderer::render_mesh(const GPUMeshData* gpu_data)
    -> void {
    if (gpu_data == nullptr) {
        Logger::err("Cannot render a mesh with incomplete render data");
        return;
    }
    if (gpu_data->m_vertex_buffer == nullptr ||
        gpu_data->m_vertex_buffer->m_handle == nullptr) {
        Logger::err("Cannot render a mesh without a valid vertex buffer");
        return;
    }

    const vulkan::Buffer* vertex_buffer =
        static_cast<vulkan::Buffer*>(gpu_data->m_vertex_buffer->m_handle);

    vulkan::CommandBuffer& cb = m_frames[m_frame_index].m_cmd;
    cb.bind_vertex_buffer(0, *vertex_buffer, 0);

    if (gpu_data->index_count() != 0) {
        if (gpu_data->m_index_buffer == nullptr ||
            gpu_data->m_index_buffer->m_handle == nullptr) {
            Logger::err("Cannot render indexed mesh without an index buffer");
            return;
        }
        const vulkan::Buffer* index_buffer =
            static_cast<vulkan::Buffer*>(gpu_data->m_index_buffer->m_handle);
        cb.bind_index_buffer(*index_buffer, 0);
        cb.draw_indexed(gpu_data->index_count(), 1, 0, 0, 0);
    } else {
        cb.draw(gpu_data->vertex_count(), 1, 0, 0);
    }
}

auto Vulkan_Renderer::present() -> void {
    // Presenting hands the image back to the swapchain, it cannot be read afterwards.
    m_frame_rendered = false;

    if (m_skip_present) {
        m_skip_present = false;
        return;
    }

    vulkan::LogicalDevice& d = *m_logical_device;
    (void)d;

    Frame&   frame = m_frames[m_frame_index];
    VkResult result = frame.present(
        m_swapchain.m_present_queue, m_swapchain, m_present_semaphores[frame.m_index]
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_framebuffer_resized) {
        m_framebuffer_resized = false;
        Logger::debug("recreate because of vkQueuePresentKHR");
        this->recreate_swapchain();
    } else if (result != VK_SUCCESS) {
        Logger::err("failed to present swap chain image!");
        assert(false);
    }

    // auto max_frames_in_flight = d.m_swapchain.m_images.size() - 1;
    m_frame_index = (m_frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

auto Vulkan_Renderer::set_viewport(u32 x, u32 y, u32 width, u32 height) -> void {
    // Takes effect with the next `render()`, which translates it for the then current
    // swapchain extent.
    m_viewport = vulkan::ViewportRect{.x = x, .y = y, .width = width, .height = height};
}

auto Vulkan_Renderer::take_screenshot(const char* filename)
    -> Result<Image, RendererError> {
    const auto fail = [](RendererErrorCode code, std::string message) {
        return err<Image, RendererError>(RendererError{code, std::move(message)});
    };

    if (!m_frame_rendered) {
        // Outside of this window the application does not own a swapchain image with
        // defined content.
        return fail(
            RendererErrorCode::FRAME_NOT_AVAILABLE,
            "Vulkan: a screenshot can only be taken between render() and present()"
        );
    }
    if (!m_swapchain.m_supports_readback) {
        return fail(
            RendererErrorCode::UNSUPPORTED,
            "Vulkan: the surface does not allow its swapchain images to be copied from"
        );
    }

    vulkan::LogicalDevice& d = *m_logical_device;
    Frame&                 frame = m_frames[m_frame_index];
    const vulkan::Image&   image = m_swapchain.m_images[frame.m_index];

    // The capture region is the area `render()` was allowed to draw to.
    const auto&    limits = d.m_physical_device->limits();
    const VkRect2D region = vulkan::to_viewport_state(
                                m_viewport,
                                m_swapchain.m_extent,
                                VkExtent2D{
                                    limits.maxViewportDimensions[0],
                                    limits.maxViewportDimensions[1]
                                }
    ).scissor;
    if (region.extent.width == 0 || region.extent.height == 0) {
        return fail(
            RendererErrorCode::INVALID_STATE,
            "Vulkan: the viewport does not cover any part of the framebuffer"
        );
    }

    // Validate the format before any GPU work is issued.
    std::vector<u8> rgb;
    if (!vulkan::readback_to_rgb({}, 0, 0, m_swapchain.m_image_format, rgb)) {
        return fail(
            RendererErrorCode::UNSUPPORTED,
            std::string("Vulkan: screenshots of swapchain format ") +
                std::to_string(static_cast<i32>(m_swapchain.m_image_format)) +
                " are not supported"
        );
    }

    // All supported formats use 4 bytes per texel and the copy is tightly packed.
    const VkDeviceSize byte_count =
        VkDeviceSize{region.extent.width} * region.extent.height * 4;
    const vulkan::Buffer readback(d, vulkan::Buffer::TYPE::READBACK, nullptr, byte_count);

    // `render()` left the image in PRESENT_SRC layout, and it has to be in that layout
    // again for `present()`.
    vulkan::CommandBuffer cmd = d.m_command_pool.allocate_buffer();
    cmd.record([&] {
        cmd.transition_layout(
            image,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT
        );
        cmd.copy_image_to_buffer(image, readback, region);
        cmd.transition_layout(
            image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_ACCESS_TRANSFER_READ_BIT,
            0,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
        );
    });

    // The copy is submitted to the queue the frame was submitted to, so it executes
    // after the frame's draw commands. Waiting for its own fence is all the
    // synchronization the readback needs; other frames in flight are not stalled on.
    vulkan::Fence copy_done = d.create_fence(false);
    d.m_graphics_queue.submit(cmd, nullptr, nullptr, &copy_done);
    d.wait_for_fence(copy_done, true, UINT64_MAX);
    d.m_command_pool.free_buffer(cmd);

    std::vector<u8> texels(static_cast<size_t>(byte_count));
    readback.read(texels.data(), byte_count, 0);
    if (!vulkan::readback_to_rgb(
            texels,
            region.extent.width,
            region.extent.height,
            m_swapchain.m_image_format,
            rgb
        )) {
        return fail(
            RendererErrorCode::BACKEND_FAILURE,
            "Vulkan: failed to convert the frame that was read back"
        );
    }

    Image result;
    result.data = std::move(rgb);
    result.width = static_cast<i32>(region.extent.width);
    result.height = static_cast<i32>(region.extent.height);
    result.num_components = 3;

    if (filename != nullptr && filename[0] != '\0' && !write_png(result, filename)) {
        return fail(
            RendererErrorCode::IO_FAILURE,
            std::string("Vulkan: failed to write the screenshot to ") + filename
        );
    }
    return ok<Image, RendererError>(std::move(result));
}
} // namespace JadeFrame
