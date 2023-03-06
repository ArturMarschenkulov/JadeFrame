#pragma once
#include "JadeFrame/prelude.h"
#include "../mesh.h"
#include "vulkan_context.h"

#include "../material_handle.h"
#include "../graphics_shared.h"
namespace JadeFrame {

class Windows_Window;
class Object;
class Matrix4x4;
class RGBAColor;

struct Vulkan_RenderCommand {
    const Matrix4x4*         transform = nullptr;
    const VertexData*        vertex_data = nullptr;
    MaterialHandle*          material_handle = nullptr;
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
    Vulkan_Context                            m_context;
    vulkan::LogicalDevice*                    m_logical_device;
    RenderSystem*                             m_system = nullptr;
    mutable std::deque<Vulkan_RenderCommand>  m_render_commands;
    std::map<u32, vulkan::Vulkan_GPUMeshData> m_registered_meshes;

private: // NOTE: probably temporary
    RGBAColor m_clear_color;

    // TODO: Move the descriptor stuff to the shader code
public: // Descriptor set
    vulkan::DescriptorPool m_descriptor_pool;
    std::array<vulkan::DescriptorSet, static_cast<u8>(vulkan::DESCRIPTOR_SET_FREQUENCY::MAX)>       m_descriptor_sets;


    vulkan::Buffer m_ub_cam = {vulkan::Buffer::TYPE::UNIFORM};
    vulkan::Buffer m_ub_tran = {vulkan::Buffer::TYPE::UNIFORM};
};
} // namespace JadeFrame