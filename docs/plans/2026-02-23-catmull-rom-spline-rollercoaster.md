# Catmull-Rom Spline Integration — Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add Catmull-Rom spline interpolation so the sweep mesh generates smooth curved tubes and the parametric animator moves objects smoothly along the same curve, enabling roller coaster-like paths.

**Architecture:** A header-only spline utility provides point evaluation and path subdivision. The sweep mesh generator subdivides control points into a dense smooth path before generating circles. The parametric animator subdivides and lerps between subdivided points for smooth continuous motion. The Sweep struct is updated to separate path resolution from circle resolution.

**Tech Stack:** C++17, GLM, OpenGL

---

## Task 1: Create `spline.h` — Catmull-Rom utility

**Files:**
- Create: `src/math/spline.h`

**Status:** COMPLETE (commit a821cf0)

---

## Task 2: Update `Sweep` struct to separate path and circle resolution

**Files:**
- Modify: `src/ecs/components.h:44-48`

**Status:** COMPLETE (commit a821cf0)

---

## Task 3: Update `Mesh::loadSweep` to accept separate params and use spline subdivision

**Files:**
- Modify: `src/mesh.h:28-29`
- Modify: `src/mesh.cpp:182-247`

**Status:** COMPLETE (commits a821cf0, 9037b61)

---

## Task 4: Update `ResourceManager` to pass new params

**Files:**
- Modify: `src/resource_manager.h:22-23`
- Modify: `src/resource_manager.cpp:33-44`

**Status:** COMPLETE (commit a821cf0)

---

## Task 5: Update `App::loadObjectFromConfig` call site

**Files:**
- Modify: `src/app.cpp:65-67`

**Status:** COMPLETE (commit a821cf0)

---

## Task 6: Update `ObjectConfig` sweep definition in `app.cpp`

**Files:**
- Modify: `src/app.cpp:100-112`

**Status:** COMPLETE (commit a821cf0)

---

## Task 7: Update `ParametricAnimator` to use spline interpolation

**Files:**
- Modify: `src/ecs/systems.h:43-54`

**Status:** COMPLETE (commit 6670222)

---

## Task 8: Build and test

**Status:** COMPLETE — Clean build, zero warnings, zero errors

---

## Commits

1. `a821cf0` feat: add Catmull-Rom spline subdivision to sweep mesh
2. `9037b61` fix: address code review issues in spline/sweep implementation
3. `6670222` feat: smooth parametric animation using Catmull-Rom spline

---

## Future Improvements (Not Implemented)

- **Orientation tracking**: Make the cube face its direction of travel (spline tangent gives this for free)
- **Smooth normals**: Switch sweep mesh from flat per-face to per-vertex normals from Frenet frame
- **Arc-length parameterization**: Give uniform speed through all segments regardless of control point spacing
