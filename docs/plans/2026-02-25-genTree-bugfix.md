# genTree Recursive Bug Fix Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Fix critical bugs in the recursive genTree function.

**Architecture:** The tree generation recursively creates branches. The main bugs are:
1. Wrong loop bound - uses `numLevels` instead of `numPerLevel`
2. Wrong base case - checks `numLevels - 1 == 0` instead of `numLevels == 0`
3. Branch end position calculated from tree base instead of branch start

**Tech Stack:** C++, GLM math library

---

### Task 1: Fix loop bound

**Files:**
- Modify: `src/app.cpp:88`

**Step 1: Change numLevels to numPerLevel in loop**

Replace line 88:
```cpp
  for (int i = 0; i < numLevels; i++) {
```

With:
```cpp
  for (int i = 0; i < numPerLevel; i++) {
```

**Step 2: Verify build**

```bash
make
```

---

### Task 2: Fix base case

**Files:**
- Modify: `src/app.cpp:83`

**Step 1: Fix the base case check**

Replace lines 83-85:
```cpp
  if (numLevels - 1 == 0) {
    return;
  }
```

With:
```cpp
  if (numLevels <= 0) {
    return;
  }
```

**Step 2: Verify build**

```bash
make
```

---

### Task 3: Fix branch end position

**Files:**
- Modify: `src/app.cpp:91-93`

**Step 1: Fix branch end to extend outward from branch start**

Replace lines 91-93:
```cpp
    // get random end x,y,z value off of log
    glm::vec3 branchEnd =
        glm::mix(startPos, startPos + glm::vec3{0, 1, 0}, rand() % 100 / 100.f);
```

With:
```cpp
    // get random end x,y,z value extending outward from branch start
    float branchLen = glm::distance(startPos, endPos) * 0.5f;
    glm::vec3 direction = glm::normalize(branchStart - startPos);
    glm::vec3 branchEnd = branchStart + direction * branchLen + glm::vec3{0, branchLen * 0.5f, 0};
```

**Step 2: Verify build**

```bash
make
```

---

### Task 4: Verify tree renders

**Step 1: Run the application**

```bash
./bin/opengl_template
```

Check that branches spawn correctly at each level and the tree structure is reasonable of Changes

|.

---

### Summary Line | Fix |
|------|-----|
| 88 | `numLevels` → `numPerLevel` |
| 83-85 | `numLevels - 1 == 0` → `numLevels <= 0` |
| 91-93 | Branch extends from `branchStart` outward, not from tree base |
