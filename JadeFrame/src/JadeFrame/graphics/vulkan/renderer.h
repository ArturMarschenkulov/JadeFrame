#pragma once
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

class Vulkan_Renderer : public IRenderer {
public:
    Vulkan_Renderer(RenderSystem& system, const Window* window);

    auto set_clear_color(const RGBAColor& color) -> void override;

    auto render(const Camera& camera) -> void override;
    auto present() -> void override;
    auto clear_background() -> void override;
    auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void override;
    auto take_screenshot(const char* filename) -> Image override;

    // virtual auto main_loop() -> void override;

public:
    Vulkan_Context         m_context;
    vulkan::LogicalDevice* m_logical_device;
    RenderSystem*          m_system = nullptr;

private: // NOTE: probably temporary
    RGBAColor m_clear_color;

public:
    struct Frame {
        vulkan::LogicalDevice* m_device;
        u32                    m_index;

        vulkan::CommandBuffer m_cmd;

        vulkan::Fence     m_fence;
        vulkan::Semaphore m_sem_available;
        vulkan::Semaphore m_sem_finished;

        auto init(vulkan::LogicalDevice* device) -> void {
            m_device = device;
            m_index = 0;
            m_fence = device->create_fence(true);
            m_sem_available = device->create_semaphore();
            m_sem_finished = device->create_semaphore();
            m_cmd = device->m_command_pool.allocate_buffer();
        }

        auto acquire_image(vulkan::Swapchain& swapchain) -> void {
            // Before acquiring the image we wait for the fence to be signaled.
            // If the fence is signaled, it means that the gpu has finished rendering the
            // frame.
            m_device->wait_for_fence(m_fence, VK_TRUE, UINT64_MAX);
            m_index = swapchain.acquire_image_index(&m_sem_available, nullptr);
        }

        auto submit(vulkan::Queue& queue) -> void {
            m_fence.reset();
            queue.submit(m_cmd, &m_sem_available, &m_sem_finished, &m_fence);
        }

        auto present(vulkan::Queue& queue, vulkan::Swapchain& swapchain) -> VkResult {
            return queue.present(m_index, swapchain, &m_sem_finished);
        }
    };

    std::vector<Frame> m_frames;
    size_t             m_frame_index = 0;

    vulkan::Swapchain                m_swapchain;
    vulkan::RenderPass               m_render_pass;
    std::vector<vulkan::Framebuffer> m_framebuffers;
    bool                             m_framebuffer_resized = false;

private:
    auto render_mesh(const Mesh* vertex_data, const GPUMeshData* gpu_data) -> void;
};
} // namespace JadeFrame