# 3D Engine

A C++17/OpenGL 3.3 scene renderer with entity components, hierarchical transforms, textured Blinn–Phong lighting, spline animation, and procedural geometry. The sample scene combines a spline-driven coaster, generated tube meshes, and subdivided terrain.

## Features

- Entity registry and component systems for transforms, meshes, lights, and animation.
- Parent-child transforms and switchable cameras.
- GLSL lighting with diffuse/specular textures and uniform buffers for camera and light data.
- Catmull–Rom paths, tangent-based orientation, and procedural tube meshes.
- Recursive triangle subdivision with deterministic displacement at shared terrain edges.
- OBJ loading through TinyObjLoader and image loading through stb_image.

This is a learning and experimentation project. It does not claim production-engine performance or a validated frame-rate target.

## Requirements

The current Makefile targets Linux:

- C++17 compiler (`g++` by default), C compiler, and GNU Make.
- GLFW development headers/library and GLM headers.
- OpenGL development support (`libGL`) and a display/driver supporting OpenGL 3.3 Core.

GLAD, stb_image, and TinyObjLoader sources/headers are included in the repository. GLFW and GLM must be supplied by the system. The GLM version must provide the `glm/ext` and `glm/gtx` headers used by the source.

## Build and run

```sh
git clone https://github.com/Quentin2006/3dEngine.git
cd 3dEngine
make -j2
make run
```

Run from the repository root so relative shader paths resolve. The current executable name is `bin/opengl_template`. `make clean` removes local build output.

### Sample-scene asset limitation

The sample scene in `src/app.cpp` currently points to an external `3dEngine-assets` directory using the author's absolute local paths. Those OBJ files and textures are not bundled here. Before running the sample, supply assets you have permission to use and update the mesh/texture paths in `App::run()`. A fresh clone is therefore a source portfolio and build starting point, not a self-contained demo. This repository does not grant rights to the external assets.

A graphical session is required to run the renderer. A successful compile alone does not verify rendering output. The repository currently has no automated rendering or performance test suite.

## Controls

| Keys | Action |
| --- | --- |
| W / A / S / D | Move the active camera |
| Q / E | Move down / up |
| Arrow keys | Rotate the camera |
| C | Switch camera |
| + / - | Increase / decrease terrain subdivision |

## Source map

- `src/app.cpp`: sample scene, resources, and render loop.
- `src/ecs/`: entity registry, components, and update/render systems.
- `src/math/spline.h`: Catmull–Rom interpolation and path orientation.
- `src/mesh.cpp`: mesh loading and procedural geometry.
- `src/fractal_terrain.cpp`: recursive terrain subdivision and normals.
- `src/shaders/`: vertex and fragment shaders.
- `src/uniformBuffer.*`: uniform-buffer management.
- `src/window.*`, `src/camera.*`, `src/callbacks.*`: window, camera, and input handling.
- `include/`: bundled third-party loader headers.

## License

See [LICENSE](LICENSE). Bundled third-party code retains its own notices.
