# Asset import fixtures

- `multi_mesh.obj` and `multi_mesh.mtl`: two triangles with distinct materials,
  diffuse colors, opacity, and a relative diffuse texture path.
- `instances.gltf`: two meshes and three instances under a translated, rotated,
  nonuniformly scaled parent. The first mesh is referenced twice. The geometry
  buffer is embedded so the fixture needs no binary sidecar.

Texture paths intentionally refer to absent files: these tests import metadata
without loading images or creating GPU resources. Tests also derive malformed
indices and an embedded-image variant from the glTF fixture in temporary files.
