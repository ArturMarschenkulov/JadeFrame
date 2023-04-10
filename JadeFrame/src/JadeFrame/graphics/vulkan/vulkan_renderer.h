#pragma once
#include "JadeFrame/prelude.h"
#include "../mesh.h"
#include "vulkan_context.h"
#include "vulkan_sync_object.h"

#include "../graphics_shared.h"
namespace JadeFrame {

class Windows_Window;
class Object;
class Matrix4x4;
class RGBAColor;

struct Vulkan_RenderCommand {
    const Matrix4x4*         transform = nullptr;
    const VertexData*        vertex_data = nullptr;
    MaterialHandle           material_handle = {0, 0};
    const GPUDataMeshHandle* m_GPU_mesh_data = nullptr;
};

class Vulkan_Renderer : public IRenderer {
public:
    Vulkan_Renderer(RenderSystem& system, const IWindow* window);

    virtual auto set_clear_color(const RGBAColor& color) -> void override;

    virtual auto submit(const Object& obj) -> void override;
    virtual auto render(const Matrix4x4& view_projection) -> void override;
    virtual auto present() -> void override;
    virtual auto clear_background() -> void override;
    virtual auto set_viewport(u32 x, u32 y, u32 width, u32 height) const -> void override;
    virtual auto take_screenshot(const char* filename) -> void override;

    // virtual auto main_loop() -> void override;

public:
    Vulkan_Context                           m_context;
    vulkan::LogicalDevice*                   m_logical_device;
    RenderSystem*                            m_system = nullptr;
    mutable std::deque<Vulkan_RenderCommand> m_render_commands;
    std::map<u32, vulkan::GPUMeshData>       m_registered_meshes;

private: // NOTE: probably temporary
    RGBAColor m_clear_color;

    // TODO: Move the descriptor stuff to the shader code
public: // Descriptor set
    vulkan::DescriptorPool m_set_pool;


    vulkan::Buffer m_ub_cam;
    vulkan::Buffer m_ub_tran;

public:
    struct Frame {
        vulkan::LogicalDevice* device;
        u32                    index;

        vulkan::CommandBuffer cmd;

        vulkan::Fence     fence;
        vulkan::Semaphore sem_available;
        vulkan::Semaphore sem_finished;

        auto init(vulkan::LogicalDevice* device) -> void {
            device = device;
            index = 0;
            fence = device->create_fence(true);
            sem_available = device->create_semaphore();
            sem_finished = device->create_semaphore();
            cmd = device->m_command_pool.allocate_buffer();
        }
        auto acquire_image(vulkan::Swapchain& swapchain) -> void {
            device->wait_for_fence(fence, VK_TRUE, UINT64_MAX);
            index = swapchain.acquire_image_index(&sem_available, nullptr);
        }

        auto submit(vulkan::Queue& queue) -> void {
            fence.reset();
            queue.submit(cmd, &sem_available, &sem_finished, &fence);
        }
    };
    std::vector<Frame> m_frames;
    size_t             m_frame_index = 0;
};
} // namespace JadeFrame