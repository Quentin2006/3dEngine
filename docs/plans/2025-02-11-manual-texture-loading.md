# Manual Texture Loading Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Change texture loading pipeline so textures must be manually specified when loading OBJ files, with empty texture path defaulting to white texture.

**Architecture:** Add optional `texturePath` field to `MeshPath` struct, modify `loadMesh()` to accept texture parameter, remove automatic MTL texture loading from `loadObj()`, and explicitly call `setTexture()` when a texture path is provided.

**Tech Stack:** C++, OpenGL, tinyobjloader

---

## Task 1: Modify MeshPath struct to include texture path

**Files:**
- Modify: `src/ecs/components.h:13-16`

**Changes:**
Add optional `texturePath` field to `MeshPath` struct.

```cpp
struct MeshPath {
  std::string path;
  std::string name;
  std::string texturePath; // Empty = use default white texture
};
```

**Step 1: Edit MeshPath struct**

Add the texturePath field with default empty value.

**Step 2: Verify syntax**

Run: `make` or compile to verify no syntax errors.

**Step 3: Commit**

```bash
git add src/ecs/components.h
git commit -m "feat: add texturePath field to MeshPath struct"
```

---

## Task 2: Modify ResourceManager::loadMesh to accept texture parameter

**Files:**
- Modify: `src/resource_manager.h:15-16`
- Modify: `src/resource_manager.cpp:5-29`

**Changes:**
Add optional `texturePath` parameter to `loadMesh()` method.

**Step 1: Update header declaration**

In `src/resource_manager.h:15-16`:
```cpp
std::shared_ptr<Mesh> loadMesh(const std::string &path,
                               const std::string &filename,
                               const std::string &texturePath = "");
```

**Step 2: Update implementation**

In `src/resource_manager.cpp`:
- Change function signature to include texturePath parameter
- After `loadObj()` call, check if texturePath is non-empty
- If non-empty: call `mesh->setTexture(path + texturePath)`
- If empty: mesh already has default white texture from constructor

```cpp
std::shared_ptr<Mesh> ResourceManager::loadMesh(const std::string &path,
                                                const std::string &filename,
                                                const std::string &texturePath) {
  std::string key = path + filename;

  // Check if already cached
  auto it = meshCache.find(key);
  if (it != meshCache.end()) {
    if (auto shared = it->second.lock()) {
      return shared;
    }
    // Expired, remove from cache
    meshCache.erase(it);
  }

  // Load new mesh
  auto mesh = std::make_shared<Mesh>();
  int verts = mesh->loadObj(path, filename);
  if (verts == 0) {
    std::cerr << "Failed to load mesh: " << key << std::endl;
    return nullptr;
  }

  // Load texture if specified, otherwise keep default white texture
  if (!texturePath.empty()) {
    std::string fullTexturePath = path + texturePath;
    mesh->setTexture(fullTexturePath);
  }

  meshCache[key] = mesh;
  return mesh;
}
```

**Step 3: Compile and verify**

Run: `make` to verify no compilation errors.

**Step 4: Commit**

```bash
git add src/resource_manager.h src/resource_manager.cpp
git commit -m "feat: add texturePath parameter to loadMesh()"
```

---

## Task 3: Remove automatic texture loading from Mesh::loadObj

**Files:**
- Modify: `src/mesh.cpp:92-111`

**Changes:**
Remove the code that automatically loads textures from MTL file.

**Step 1: Remove MTL texture loading**

Delete lines 92-111 in `src/mesh.cpp`:
```cpp
  // Track if we loaded a texture for this mesh
  bool textureLoaded = false;

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

      int material_id = shapes[s].mesh.material_ids[f];

      // Only load texture if material has diffuse texture and we haven't loaded
      // it yet
      if (material_id >= 0 && !textureLoaded) {
        const std::string &texName = materials[material_id].diffuse_texname;
        if (!texName.empty()) {
          std::string texturePath = filePath + texName;
          std::cerr << "Loading texture: " << texturePath << std::endl;
          setTexture(texturePath);
          textureLoaded = true;
        }
      }
```

And update the loop to remove the material_id check.

The loop should become:
```cpp
  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
      for (size_t v = 0; v < fv; v++) {
```

**Step 2: Compile and verify**

Run: `make` to verify no compilation errors.

**Step 3: Commit**

```bash
git add src/mesh.cpp
git commit -m "feat: remove automatic MTL texture loading from loadObj()"
```

---

## Task 4: Update App::loadObjectFromConfig to use MeshPath texture

**Files:**
- Modify: `src/app.cpp:57-74`

**Changes:**
Update `loadObjectFromConfig()` to pass the texture path from config.

**Step 1: Update loadObjectFromConfig**

In `src/app.cpp:60-61`:
```cpp
  registry.getMesh(obj).mesh =
      resourceManager.loadMesh(cfg.mesh.path, cfg.mesh.name, cfg.mesh.texturePath);
```

**Step 2: Compile and verify**

Run: `make` to verify no compilation errors.

**Step 3: Commit**

```bash
git add src/app.cpp
git commit -m "feat: pass texturePath from config to loadMesh()"
```

---

## Task 5: Update object configurations to specify textures

**Files:**
- Modify: `src/app.cpp:99-149`

**Changes:**
Update the object configurations to include texture paths for objects that should have textures.

**Step 1: Update cottage configuration**

Change the cottage config from:
```cpp
      {
          .mesh = {"assets/cottage/", "cottage_obj.obj"},
          .transform = {{0, 3, -15}, {0, 0, 0}, {2, 2, 2}},
      },
```

To:
```cpp
      {
          .mesh = {"assets/cottage/", "cottage_obj.obj", "textures/cottage_diffuse.png"},
          .transform = {{0, 3, -15}, {0, 0, 0}, {2, 2, 2}},
      },
```

**Step 2: Update cube light configurations**

For the light cubes in `createLightObj()`, leave texturePath empty (default white):
```cpp
return {
    .mesh = {"assets/3d-cubes/", "cube.obj", ""},
    .transform = {pos, {0, 0, 0}, {0.25, 0.25, 0.25}},
    .light = {color, 0.0f},
    .sineAnim = sine,
};
```

**Step 3: Compile and verify**

Run: `make` to verify no compilation errors.

**Step 4: Test runtime behavior**

Run the application and verify:
- Cottage loads with the diffuse texture
- Light cubes render with white default texture
- No compilation errors

**Step 5: Commit**

```bash
git add src/app.cpp
git commit -m "feat: update object configs with texture paths"
```

---

## Task 6: Update Mesh::setTexture visibility if needed

**Files:**
- Verify: `src/mesh.h:30-31`

**Changes:**
Ensure `setTexture()` can be called from `ResourceManager` (currently it's private).

**Step 1: Check current visibility**

`setTexture()` is currently private. ResourceManager needs to call it.

**Step 2: Make setTexture public**

Move `setTexture` declaration from `private:` to `public:` in `src/mesh.h`:

```cpp
public:
  Mesh();

  // Prevent copying (vertexBuffer is immobile)
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

  // Draw the mesh
  void draw();

  // Load mesh from OBJ file
  int loadObj(const std::string &assetsPath, const std::string &objName);

  // Set texture manually
  void setTexture(const std::string &path);

  // Getters
  size_t getVertexCount() const { return vertexCount; }
  unsigned int getVAO() { return buffer.getVAO(); }

private:
  vertexBuffer buffer;
  size_t vertexCount;
  unsigned int texture;

  std::vector<Vertex> vertices;
```

**Step 3: Compile and verify**

Run: `make` to verify no compilation errors.

**Step 4: Commit**

```bash
git add src/mesh.h
git commit -m "feat: make setTexture() public for ResourceManager access"
```

---

## Final Verification

**Run complete build:**
```bash
make clean && make
```

**Test application:**
```bash
./bin/3dEngine
```

**Verify:**
- [ ] Cottage renders with cottage_diffuse.png texture
- [ ] Light cubes render with default white texture
- [ ] No texture loading errors in console
- [ ] Clean compile with no warnings
