# Sponza rendering scaffold

This standalone OpenGL example renders colored cubes and a floor using the existing
position/color shader, a perspective camera, and depth testing. No Sponza asset is
required or imported.

With `BUILD_EXAMPLES=ON` and `JADEFRAME_ENABLE_OPENGL=ON`:

```sh
cmake --build build --target Example_sponza
./build/examples/sponza/Example_sponza
```

Use W/A/S/D to move, Q/E to move down/up in the camera basis, and arrow keys to look.
Close the window to exit. `--smoke-test` renders three frames, verifies that a frame
can be captured, and exits with a status code. It requires a display; on Linux it
can also run under `xvfb-run -a` with a software OpenGL implementation.

`create_test_scene()` owns example setup. It uploads local CPU meshes and creates
objects that borrow registered GPU meshes and a material. Several objects share
the same mesh. Replacing that setup with future imported geometry does not change
the submission loop. Uploaded buffers, layout, and draw counts belong to each
`GPUMeshData`; `RenderSystem` owns registered resources until shutdown. Source CPU
meshes can be discarded after registration. Objects and queued commands must not
outlive the registry, and registered resources must not be moved or replaced.

The existing triangle-list pipeline and shader vertex format requirements remain.
The upload format is packed in CPU attribute order at consecutive shader locations;
shaders must match it. This example keeps only position and color attributes.
There is no material/texture import, lighting, or new scene-management framework.
