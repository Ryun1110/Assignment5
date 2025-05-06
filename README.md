# Software Rasterizer – Sphere Scene (CPU‑only)

> **Coursework**: Implementing a full software rasterizer without any
> GPU / hardware acceleration.

This program renders a tessellated sphere entirely on the **CPU** and
stores the resulting image as `output.ppm` (all platforms) and
`output.bmp` (Windows only).

No OpenGL, Direct3D, Vulkan, Metal, or GDI drawing calls are used –
every step of the 3‑D pipeline (model, view, projection, viewport,
triangle rasterization, Z‑buffering) is executed with plain C++17 code.

---

## Features

* **Right‑Handed camera space**  
  * Camera at the origin, looking down the −z axis  
  * Frustum planes: _near_ = −0.1, _far_ = −1000
* **Transformation pipeline**  
  * Model = `scale(2) → translate(0,0,−7)`  
  * View = identity  
  * Perspective frustum: `l,r,b,t = ±0.1`
  * Viewport: 512 × 512 px (y‑axis flipped)
* **Triangle rasterizer**  
  * Edge‑function test inside per‑tile bounding box  
  * 32‑bit float Z‑buffer  
  * Mono‑color shading (`RGB 255,255,255`)
* **Portable output**  
  * `output.ppm` (P6) – viewable everywhere  
  * `output.bmp` – plus auto‑open on Windows

---

## Build & Run

### Windows (MSVC)

```bat
cl /O2 /std:c++17 sphere_scene.cpp /Fe:rasterizer.exe
rasterizer.exe
Windows (MinGW‑w64) or Linux / macOS (GCC / Clang)
bash

g++ -O2 -std=c++17 sphere_scene.cpp -o rasterizer
./rasterizer
No external dependencies – only the C++17 standard library.

File Overview
File	Purpose
sphere_scene.cpp	Complete source code (≈ 200 lines)
output.ppm	Generated P6 image (created at runtime)
output.bmp	Same image in BMP format (Windows only)

Code Structure
Math – Vec3, Vec4, Mat4 with operator overloads

Transforms – helper functions: translate, scale,
perspective, viewport

Scene generation – lat/long subdivision of a unit sphere

Rasterization – edge‑function, bounding‑box scan, Z‑buffer

Image write‑out – portable PPM, Win32 BMP helper

Extending the Project
Idea	Hint
Accurate Z‑interpolation	Use barycentric weights instead of triangle average
Lighting	Per‑vertex normals → Gouraud or per‑pixel Phong shading
Texture mapping	Add (u,v) coords, sample 2‑D image
Back‑face culling	Skip triangles whose screen‑space normal faces away
Clipping	Implement Sutherland–Hodgman or homogeneous clipping


