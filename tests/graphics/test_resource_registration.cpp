#include <gtest/gtest.h>
#include <limits>
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/graphics/mesh.h"

using namespace JadeFrame;

namespace {
struct State {
    bool                     alive = false;
    int                      allocations = 0;
    int                      releases = 0;
    int                      fail_at = 0;
    int                      calls = 0;
    std::vector<std::string> events;
};

struct Allocation {
    State*      state;
    std::string kind;
};

auto release(void* ptr) noexcept -> void {
    auto* allocation = static_cast<Allocation*>(ptr);
    EXPECT_TRUE(allocation->state->alive);
    ++allocation->state->releases;
    allocation->state->events.push_back(allocation->kind);
    delete allocation;
}

class Renderer final : public IRenderer {
public:
    explicit Renderer(State& state)
        : s(state) {}

    ~Renderer() override {
        for (auto* buffer : buffers) { release(buffer); }
        s.alive = false;
        s.events.push_back("renderer");
    }

    auto initialize() -> Result<void, InitError> override {
        s.alive = true;
        return ok<void, InitError>();
    }

    auto allocate(const char* kind) -> NativeHandle {
        if (++s.calls == s.fail_at) { return {nullptr, noop_native_handle_deleter}; }
        ++s.allocations;
        return {
            new Allocation{&s, kind},
             release
        };
    }

    auto create_buffer(GPUBuffer::TYPE type, const void*, size_t) -> void* override {
        auto* buffer = allocate(type == GPUBuffer::INDEX ? "index" : "vertex").release();
        if (buffer) { buffers.push_back(buffer); }
        return buffer;
    }

    auto destroy_buffer(void* p) -> void override {
        std::erase(buffers, p);
        release(p);
    }

    auto create_texture(const Image&) -> NativeHandle override {
        return allocate("texture");
    }

    auto create_shader(const ShadingCode&) -> NativeHandle override {
        return allocate("shader");
    }

    auto create_material(const ShaderHandle&, const TextureHandle*)
        -> NativeHandle override {
        // Two native allocations. The temporary must release the first if the second
        // fails.
        auto first = allocate("partial material");
        if (!first) { return first; }
        auto second = allocate("material");
        return second;
    }

    auto set_shader_uniform(void*, const std::string&, const void*, size_t)
        -> Result<void, RendererError> override {
        return ok<void, RendererError>();
    }

    auto present() -> void override {}

    auto wait_until_idle() -> void override { s.events.push_back("idle"); }

    auto clear_background() -> void override {}

    auto render(const Camera&) -> void override {}

    auto set_clear_color(const RGBAColor&) -> void override {}

    auto set_viewport(u32, u32, u32, u32) -> void override {}

    auto take_screenshot(const char*) -> Result<Image, RendererError> override {
        return err<Image, RendererError>(
            RendererError{RendererErrorCode::UNSUPPORTED, "test"}
        );
    }

    std::vector<void*> buffers;
    State&             s;
};

class Registration : public testing::Test {
protected:
    State        state;
    RenderSystem system;

    auto SetUp() -> void override {
        ASSERT_TRUE(system
                        .init_with_renderer(
                            GRAPHICS_API::SOFTWARE, std::make_unique<Renderer>(state)
                        )
                        .is_ok());
    }

    auto TearDown() -> void override {
        system.shutdown();
        EXPECT_EQ(state.allocations, state.releases);
    }

    auto image() -> Image {
        Image i;
        i.width = 2;
        i.height = 2;
        i.num_components = 4;
        i.data.resize(16);
        return i;
    }

    auto mesh() -> Mesh {
        Mesh m;
        m.m_attributes[Mesh::POSITION.m_id] = {
            Mesh::POSITION, {0, 0, 0, 1, 0, 0, 0, 1, 0}
        };
        m.m_indices = {0, 1, 2};
        return m;
    }
};
} // namespace

TEST_F(Registration, InvalidTexturesDoNotAllocateOrChangeRegistry) {
    auto previous = system.try_register_texture(image());
    ASSERT_TRUE(previous.is_ok());
    auto* native = previous.value()->m_handle.get();
    for (int dimension : {0, -1}) {
        auto i = image();
        i.width = dimension;
        EXPECT_TRUE(system.try_register_texture(i).is_err());
        i = image();
        i.height = dimension;
        EXPECT_TRUE(system.try_register_texture(i).is_err());
    }
    for (int components : {-1, 0, 1, 2, 5, 100}) {
        auto i = image();
        i.num_components = components;
        EXPECT_TRUE(system.try_register_texture(i).is_err());
    }
    auto i = image();
    i.data.resize(15);
    EXPECT_TRUE(system.try_register_texture(i).is_err());
    i.width = std::numeric_limits<i32>::max();
    i.height = i.width;
    EXPECT_TRUE(system.try_register_texture(i).is_err());
    EXPECT_EQ(state.calls, 1);
    EXPECT_EQ(system.m_registered_textures.size(), 1U);
    EXPECT_EQ(previous.value()->m_handle.get(), native);
}

TEST_F(Registration, NativeTextureAndShaderFailuresLeaveRegistriesUnchanged) {
    auto texture = system.try_register_texture(image());
    auto shader = system.try_register_shader({});
    ASSERT_TRUE(texture.is_ok());
    ASSERT_TRUE(shader.is_ok());
    state.fail_at = state.calls + 1;
    EXPECT_TRUE(system.try_register_texture(image()).is_err());
    state.fail_at = state.calls + 1;
    EXPECT_TRUE(system.try_register_shader({}).is_err());
    EXPECT_EQ(system.m_registered_textures.size(), 1U);
    EXPECT_EQ(system.m_registered_shaders.size(), 1U);
    EXPECT_NE(texture.value()->m_handle, nullptr);
    EXPECT_NE(shader.value()->m_handle, nullptr);
}

TEST_F(Registration, VertexAndIndexFailuresRollBackWithoutChangingExistingMesh) {
    auto previous = system.try_register_mesh(mesh());
    ASSERT_TRUE(previous.is_ok());
    auto* native = previous.value()->m_vertex_buffer->m_handle;
    for (int failure : {1, 2}) {
        const auto released = state.releases;
        state.fail_at = state.calls + failure;
        auto result = system.try_register_mesh(mesh());
        ASSERT_TRUE(result.is_err());
        EXPECT_NE(
            result.error().m_message.find(failure == 1 ? "vertex" : "index"),
            std::string::npos
        );
        EXPECT_EQ(state.releases - released, failure - 1);
        EXPECT_EQ(system.m_registered_meshes.size(), 1U);
        EXPECT_EQ(previous.value()->m_vertex_buffer->m_handle, native);
    }
    state.fail_at = 0;
    EXPECT_TRUE(system.try_register_mesh(mesh()).is_ok());
}

TEST_F(Registration, InvalidMeshAndBufferInputsNeverAllocate) {
    EXPECT_TRUE(system.try_register_mesh(Mesh{}).is_err());
    int       data = 1;
    GPUBuffer a(&system, nullptr, 4, GPUBuffer::VERTEX);
    GPUBuffer b(&system, &data, 0, GPUBuffer::INDEX);
    GPUBuffer c(&system, &data, 3, GPUBuffer::INDEX);
    GPUBuffer d(&system, &data, 4, static_cast<GPUBuffer::TYPE>(100));
    EXPECT_EQ(a.m_handle, nullptr);
    EXPECT_EQ(b.m_handle, nullptr);
    EXPECT_EQ(c.m_handle, nullptr);
    EXPECT_EQ(d.m_handle, nullptr);
    EXPECT_EQ(state.calls, 0);
}

TEST_F(Registration, MaterialDependenciesAndPartialFailureAreTransactional) {
    auto shader = system.try_register_shader({});
    auto texture = system.try_register_texture(image());
    ASSERT_TRUE(shader.is_ok());
    ASSERT_TRUE(texture.is_ok());
    ShaderHandle  foreign;
    TextureHandle invalid;
    EXPECT_TRUE(system.try_register_material(nullptr, nullptr).is_err());
    EXPECT_TRUE(system.try_register_material(&foreign, nullptr).is_err());
    EXPECT_TRUE(system.try_register_material(shader.value(), &invalid).is_err());
    const auto calls = state.calls;
    state.fail_at = calls + 2;
    auto result = system.try_register_material(shader.value(), texture.value());
    EXPECT_TRUE(result.is_err());
    EXPECT_EQ(state.releases, 1);
    EXPECT_TRUE(system.m_registered_materials.empty());
    state.fail_at = 0;
    EXPECT_TRUE(system.try_register_material(shader.value(), texture.value()).is_ok());
}

TEST_F(
    Registration,
    ShutdownOrdersResourcesAndRepeatedInitializationDoesNotDoubleDelete
) {
    for (int iteration = 0; iteration < 3; ++iteration) {
        auto shader = system.try_register_shader({});
        auto texture = system.try_register_texture(image());
        ASSERT_TRUE(shader.is_ok());
        ASSERT_TRUE(texture.is_ok());
        ASSERT_TRUE(
            system.try_register_material(shader.value(), texture.value()).is_ok()
        );
        ASSERT_TRUE(system.try_register_mesh(mesh()).is_ok());
        state.events.clear();
        system.shutdown();
        system.shutdown();
        EXPECT_EQ(state.allocations, state.releases);
        EXPECT_EQ(
            state.events,
            (std::vector<std::string>{
                "idle", "index", "vertex", "material", "shader", "texture", "renderer"
            })
        );
        EXPECT_TRUE(system.try_register_shader({}).is_err());
        EXPECT_TRUE(system.try_register_texture(image()).is_err());
        EXPECT_TRUE(system.try_register_mesh(mesh()).is_err());
        ASSERT_TRUE(system
                        .init_with_renderer(
                            GRAPHICS_API::SOFTWARE, std::make_unique<Renderer>(state)
                        )
                        .is_ok());
    }
}

TEST_F(Registration, MovingBuffersReleasesEachAllocationExactlyOnce) {
    int       data = 0;
    GPUBuffer a(&system, &data, sizeof(data), GPUBuffer::VERTEX);
    GPUBuffer b(&system, &data, sizeof(data), GPUBuffer::VERTEX);
    b = std::move(a);
    EXPECT_EQ(a.m_handle, nullptr);
    EXPECT_EQ(state.releases, 1);
    GPUBuffer c(std::move(b));
    EXPECT_EQ(b.m_handle, nullptr);
    EXPECT_NE(c.m_handle, nullptr);
}

TEST_F(Registration, StandaloneBufferCanBeDestroyedAfterContextShutdown) {
    int  data = 0;
    auto buffer =
        std::make_unique<GPUBuffer>(&system, &data, sizeof(data), GPUBuffer::VERTEX);
    ASSERT_NE(buffer->m_handle, nullptr);
    system.shutdown();
    EXPECT_EQ(state.allocations, state.releases);
    buffer.reset();
    EXPECT_EQ(state.allocations, state.releases);
}

TEST_F(Registration, LegacyMeshConstructorRollsBackFailedIndexAllocation) {
    state.fail_at = state.calls + 2;
    GPUMeshData data(&system, mesh());
    EXPECT_EQ(data.m_vertex_buffer, nullptr);
    EXPECT_EQ(data.m_index_buffer, nullptr);
    EXPECT_EQ(state.allocations, state.releases);
}

TEST_F(Registration, ForeignLiveDependenciesAreRejectedBeforeNativeCreation) {
    State        other_state;
    RenderSystem other;
    ASSERT_TRUE(other
                    .init_with_renderer(
                        GRAPHICS_API::SOFTWARE, std::make_unique<Renderer>(other_state)
                    )
                    .is_ok());
    auto foreign_shader = other.try_register_shader({});
    auto foreign_texture = other.try_register_texture(image());
    auto shader = system.try_register_shader({});
    ASSERT_TRUE(foreign_shader.is_ok());
    ASSERT_TRUE(foreign_texture.is_ok());
    ASSERT_TRUE(shader.is_ok());
    const auto calls = state.calls;
    EXPECT_TRUE(system.try_register_material(foreign_shader.value(), nullptr).is_err());
    EXPECT_TRUE(
        system.try_register_material(shader.value(), foreign_texture.value()).is_err()
    );
    EXPECT_EQ(state.calls, calls);
    other.shutdown();
    EXPECT_EQ(other_state.allocations, other_state.releases);
}
