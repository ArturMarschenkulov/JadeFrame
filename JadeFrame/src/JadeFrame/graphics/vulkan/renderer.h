#pragma once
#include <span>
#include <vector>

#include "JadeFrame/types.h"
#include "../mesh.h"
#include "../graphics_shared.h"
#include "context.h"
#include "sync_object.h"

namespace JadeFrame {

class Windows_Window;
class Object;
class mat4x4;
class RGBAColor;

namespace vulkan {

/// A viewport as the renderer interface describes it: pixels, origin in the lower-left
/// corner of the framebuffer, y pointing up.
struct ViewportRect {
    u32 x = 0;
    u32 y = 0;
    u32 width = 0;
    u32 height = 0;
};

struct ViewportState {
    VkViewport viewport;
    VkRect2D   scissor;
};

/// Translates `rect` into the dynamic viewport and scissor state for a framebuffer of
/// size `framebuffer`.
///
/// The viewport is flipped (negative height) so that clip space y points up like in
/// OpenGL. The scissor is the part of `rect` that lies inside the framebuffer; it keeps
/// rendering inside both the viewport and the render area, and it is empty if `rect`
/// is empty or completely outside. Vulkan requires a viewport of at least 1x1 and at
/// most `max_dimensions` (VkPhysicalDeviceLimits::maxViewportDimensions), so the
/// viewport itself is clamped to that range.
[[nodiscard]] auto to_viewport_state(
    const ViewportRect& rect,
    VkExtent2D          framebuffer,
    VkExtent2D          max_dimensions
) -> ViewportState;

/// Converts tightly packed 4 byte texels of `format`, stored top row first, into tightly
/// packed RGB with the bottom row first (the layout of `JadeFrame::Image`).
///
/// Returns false and leaves `rgb` untouched if `format` is not one of the supported
/// 8 bit BGRA/RGBA formats or if `texels` holds less than `width * height` texels.
[[nodiscard]] auto readback_to_rgb(
    std::span<const u8> texels,
    u32                 width,
    u32                 height,
    VkFormat            format,
    std::vector<u8>&    rgb
) -> bool;

} // namespace vulkan

class Vulkan_Renderer : public IRenderer {
public:
    Vulkan_Renderer(RenderSystem& system, Window* window);
    ~Vulkan_Renderer() override;

    [[nodiscard]] auto initialize() -> Result<void, InitError> override;
    auto create_buffer(GPUBuffer::TYPE type, const void* data, size_t size)
        -> void* override;
    auto destroy_buffer(void* handle) -> void override;
    auto create_texture(const Image& image) -> NativeHandle override;
    auto create_shader(const ShadingCode& code) -> NativeHandle override;
    auto create_material(const ShaderHandle& shader, const TextureHandle* texture)
        -> NativeHandle override;
    auto set_shader_uniform(
        void*             shader,
        const std::string& name,
        const void*        data,
        size_t             size
    ) -> Result<void, RendererError> override;

    auto set_clear_color(const RGBAColor& color) -> void override;

    auto render(const Camera& camera) -> void override;
    auto present() -> void override;
    auto wait_until_idle() -> void override;
    auto clear_background() -> void override;
    auto set_viewport(u32 x, u32 y, u32 width, u32 height) -> void override;
    auto take_screenshot(const char* filename) -> Result<Image, RendererError> override;

    // virtual auto main_loop() -> void override;

public:
    // Declared first, so that the instance and the device outlive everything below.
    Vulkan_Context         m_context;
    vulkan::LogicalDevice* m_logical_device;
    RenderSystem*          m_system = nullptr;
    Window*                m_window = nullptr;

private: // NOTE: probably temporary
    RGBAColor m_clear_color;

    // Applied as dynamic state at the start of every frame, see `to_viewport_state`.
    vulkan::ViewportRect m_viewport;
    // True from the end of a successful `render()` until `present()`. While it is set
    // the swapchain image `m_frames[m_frame_index].m_index` holds the rendered frame.
    bool                 m_frame_rendered = false;

public:
    struct Sync {
        // Is signaled when the gpu has finished executing commands.
        vulkan::Fence     m_in_flight;
        // Is signaled when ready to be rendered, thus was acquired by the gpu.
        vulkan::Semaphore m_sem_available;
    };

    struct Frame {
        vulkan::LogicalDevice* m_device;
        u32                    m_index;

        vulkan::CommandBuffer m_cmd;

        Sync m_sync;

        /// Returns false if one of the frame's Vulkan objects could not be created.
        [[nodiscard]] auto init(vulkan::LogicalDevice* device) -> bool {
            m_device = device;
            m_index = 0;
            m_sync.m_in_flight = device->create_fence(true);
            m_sync.m_sem_available = device->create_semaphore();
            m_cmd = device->m_command_pool.allocate_buffer();
            return m_sync.m_in_flight.m_handle != VK_NULL_HANDLE &&
                   m_sync.m_sem_available.m_handle != VK_NULL_HANDLE &&
                   m_cmd.m_handle != VK_NULL_HANDLE;
        }

        auto acquire_image(vulkan::Swapchain& swapchain) -> void {
            // Before acquiring the image we wait for the fence to be signaled.
            // If the fence is signaled, it means that the gpu has finished rendering the
            // frame.
            m_device->wait_for_fence(m_sync.m_in_flight, VK_TRUE, UINT64_MAX);
            m_index = swapchain.acquire_image_index(&m_sync.m_sem_available, nullptr);
        }

        // `rendered` is signaled once the frame can be presented.
        auto submit(vulkan::Queue& queue, const vulkan::Semaphore& rendered) -> void {
            m_sync.m_in_flight.reset();
            queue.submit(m_cmd, &m_sync.m_sem_available, &rendered, &m_sync.m_in_flight);
        }

        auto present(
            vulkan::Queue&           queue,
            vulkan::Swapchain&       swapchain,
            const vulkan::Semaphore& rendered
        ) -> VkResult {
            return queue.present(m_index, swapchain, &rendered);
        }
    };

    std::vector<Frame> m_frames;
    size_t             m_frame_index = 0;
    // Signaled when the frame rendered into a swapchain image can be presented. There is
    // one per swapchain image, indexed by image index: a semaphore handed to a present
    // stays in use until that image is acquired again, which is unrelated to the order
    // in which the entries of `m_frames` are reused. Declared before the swapchain so
    // that they outlive it.
    std::vector<vulkan::Semaphore> m_present_semaphores;

    vulkan::Swapchain                m_swapchain;
    vulkan::RenderPass               m_render_pass;
    std::vector<vulkan::Framebuffer> m_framebuffers;
    bool                             m_framebuffer_resized = false;
    bool                             m_skip_present = false;
    // Set while the swapchain could not be recreated, see `recreate_swapchain`.
    bool                             m_swapchain_recreation_failed = false;

private:
    /// Rebuilds the swapchain and what depends on it. If that fails there is no
    /// swapchain afterwards and `render()` drops frames while retrying.
    auto recreate_swapchain() -> void;
    /// Creates what is derived from the swapchain: render pass, framebuffers and
    /// present semaphores. Without a swapchain (no drawable area) only the render pass
    /// is created.
    [[nodiscard]] auto create_swapchain_dependents() -> Result<void, InitError>;
    auto create_present_semaphores() -> void;
    auto render_mesh(const GPUMeshData* gpu_data) -> void;
};

static_assert(is_renderer<Vulkan_Renderer>);
} // namespace JadeFrame
