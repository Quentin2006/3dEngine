Bundled third-party loader headers:

- glad/glad.h and KHR/khrplatform.h: GLAD OpenGL loader headers.
  The matching generated implementation is src/glad.c.
- stb/stb_image.h: image loader.
- tol/tiny_obj_loader.h: OBJ model loader.

The generated GLAD files are already checked in; no generation step is needed
for a normal build. GLFW development files and GLM headers are system
dependencies, documented in the repository README.

Retain the license notices in each bundled source/header.
