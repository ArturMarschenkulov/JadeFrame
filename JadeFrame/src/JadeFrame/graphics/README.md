This folder contains graphics related code.

In this folder the graphics API backends are located, e.g. `OpenGL`, `Vulkan`, `D3D11`, `D3D12`, etc.
At the time of writing, only `OpenGL` and `Vulkan` are in a usable state.


# Renderer contract
All backends implement `IRenderer` (`graphics_shared.h`), which documents the behavior every backend has to provide. The rule is that an operation either does what the interface says on every backend or returns a `RendererError`; a backend never silently ignores a request.

| Operation | Contract | OpenGL | Vulkan |
|---|---|---|---|
| `set_viewport` | Pixels, origin lower-left, y up. Clipped to the framebuffer, empty draws nothing. Does not follow window resizes by itself. | `glViewport` for the offscreen scene pass. The offscreen target follows the window size. | Dynamic viewport/scissor state set per frame (flipped viewport), independent of the swapchain size. |
| `take_screenshot` | Only between `render()` and `present()` (`FRAME_NOT_AVAILABLE` otherwise). Returns the viewport rectangle as RGB8, rows bottom-to-top, and optionally writes a PNG. | `glReadPixels` from the back buffer. Includes the GUI drawn after `render()`. | Copies the acquired swapchain image into a readback buffer. `UNSUPPORTED` if the surface forbids `TRANSFER_SRC` or the swapchain format is not 8 bit RGBA/BGRA. |
| `set_shader_uniform` | Always `UNSUPPORTED`. | Shaders are SPIR-V without loose uniforms. | No per-shader uniform storage. |
| `clear_background` | The frame produced by `render()` starts cleared to the clear color. | `glClear` | Nothing to do, the render pass clears. |

Uniform data is provided through the uniform buffers of a material (`Camera` and `Transform` blocks, written by `render()`).

The contract is checked by `tests/graphics/test_renderer_contract.cpp` (no GPU needed) and `tests/graphics/test_renderer_backends.cpp` (same scenario against every compiled-in backend, needs a display).

# Goals


# Notes


# Resources
Here will be contained various resources about comparing graphics APIs. For more specific resources look into the `README`s of the relevant graphics API.

- https://www.khronos.org/assets/uploads/developers/library/2016-vulkan-devday-uk/4-Using-spir-v-with-spirv-cross.pdf

- [sokol](https://github.com/floooh/sokol): Is header only (STB-style) library with `sokol_gfx.h`, where it abstracts over OpenGL, Metal and D3D11. It has also other interesting headers, like `sokol_app.h` for window and input management.

- [DiligentEngine](https://github.com/DiligentGraphics/DiligentEngine): low-level graphics library, abstracts over OpenGL, Vulkan, Metal, D3D11 and D3D12

- [LLGL](https://github.com/LukasBanana/LLGL): low-level graphics library, abstracts over OpenGL, Vulkan, Metal, D3D11 and D3D12
- []():