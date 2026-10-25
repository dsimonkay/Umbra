# Roadmap

A ray tracer built one evening at a time. Each milestone is a handful of small steps,
and **every step ends with something new on the screen** (or at least a passing test).

## Ground rules

- **No deadlines.** The evening counts are only there to show how big a step is. They are not a schedule.
- **Stop anywhere.** After any milestone you have a working program you can show to people.
  From M3 on it already looks good.
- **One topic per step.** Ray tracing first. C++23 features come in when they fit the code, not
  as homework.
- **Leave a breadcrumb.** End every session by writing the *next tiny step* (≤ 30 minutes, concrete)
  into [DEVLOG.md](DEVLOG.md). Next time you start with that step and skip the "where was I?" part.
- **The book is the map.** Milestones M1–M5 follow
  [_Ray Tracing in One Weekend_](https://raytracing.github.io/books/RayTracingInOneWeekend.html)
  (RTiOW, free online). The book writes to a PPM file; we draw into the viewport instead.
  Otherwise the math is the same.

## Status

| | Milestone | Evenings | You will see |
|---|---|---|---|
| ✅ | [M0 Skeleton](#m0--skeleton) | done | the window, the panels, a checkerboard |
| ▶ | [M1 First light](#m1--first-light-rtiow-24) | 2–3 | a sky gradient, computed from real rays |
| | [M2 Spheres](#m2--spheres-rtiow-56) | 2–3 | a sphere, shaded by its surface normals |
| | [M3 A clean image](#m3--a-clean-image-rtiow-78--progressive-rendering) | 2–3 | smooth edges; the image refines over time |
| | [M4 Materials](#m4--materials-rtiow-911) | 3–4 | matte, metal and glass spheres |
| | [M5 The cover shot](#m5--the-cover-shot-rtiow-1214) | 2 | the book's cover image, rendered by you |
| | [M6 Scene editor](#m6--scene-editor) | 3–4 | panels for objects, materials and the camera; click to select |
| | [M7 Interactive camera](#m7--interactive-camera) | 2–3 | orbit / pan / zoom with the mouse, like a 3D app |
| | [M8 All cores](#m8--all-cores) | 3–4 | the same image, many times faster |
| | [M9 CUDA](#m9--cuda-backend) | 4–6 | the same image again, on the GPU |
| | [Beyond](#beyond) | ∞ | whatever sparks joy |

Mark the current milestone with ▶. When you finish one, see [Finishing a milestone](#finishing-a-milestone).

---

## M0 — Skeleton

**Done** (set up by Claude, 2026-10-11).

What is there:

- CMake + presets; raylib 6.0, Dear ImGui (docking branch) and rlImGui are fetched automatically.
- `umbra`: a dark GUI with dockable panels (*Viewport*, *Render*), a menu and a status bar.
  It renders the image row by row on the UI thread within a 12 ms budget per frame, so you can watch it
  being drawn (*Slow motion* makes this very visible). *Save PNG* writes to `./renders/`.
- `umbra-cli`: renders to a PNG without opening a window.
- `umbra --screenshot file.png`: renders, saves a picture of the whole window, quits.
- `umbra::RenderPixel(x, y, width, height)` in `src/core/render.cpp` is the function the app calls for
  every pixel. In M0 it drew a checkerboard.
- Added on 2026-10-12: the UI scale follows the system or *View ▸ UI scale*; the settings and the panel
  layout are saved in `~/.config/umbra/`.

Your first session (≈ 30–60 minutes):

- [x] Build and run it in WSL (see README). Resize the panels, toggle *Slow motion*, press F5.
- [x] Read `src/core/render.cpp` and skim `src/app/main.cpp`. You don't need to understand all of it.
- [x] `git init`, first commit, push to a private GitHub repo.

---

## M1 — First light (RTiOW 2–4)

**Goal:** replace the checkerboard with the first real rays. The result is a sky gradient whose
colors come from the ray directions.

Steps:

1. [x] **Gradient.** In `RenderPixel`: red = x / (width − 1), green = y / (height − 1), blue = 0
   (this is the RTiOW chapter 2 image). It is a trivial change, and it gets you started.
2. [x] **Colors as floats.** Let the rendering math work with colors in [0, 1] and convert to `Pixel`
   in exactly one place (`ToPixel(color)`, with clamping). Every later milestone depends on this.
   (Done in `src/core/color.h`: `Color` is its own struct with `r`, `g`, `b`; `ToPixel` rounds to nearest.)
3. [x] **`Vector3`** (RTiOW 3). Written by hand as a plain value type: an aggregate (no constructors),
   `constexpr` arithmetic operators as hidden friends, `Dot`, `Cross`, `SquaredLength`, `Length`,
   `UnitVector`, and `static_assert`s as unit tests that run inside the compiler.
   (The book writes `vec3`, `unit_vector`; here types and functions are `CamelCase`, see `.clang-tidy`.)
   - `Length()` and `UnitVector()` are not `constexpr`: `std::sqrt` is `constexpr` only from C++26 (GCC
     folds it anyway as an extension, Clang does not). They get runtime tests in M2 (step 5).
   - *Design question, decided (devlog 2026-10-25):* one plain `Vector3` for directions and offsets, as in
     the book. No tags, mixins or CRTP. Positions are a separate `Point3` type, see step 4.
   - *Name clashes with raylib:* raylib declares `Color`, `Vector3`, `Ray`, `Camera`, `Material` and
     `Texture` in the global namespace. Keep your types in `namespace umbra`, and in app code write
     `umbra::Vector3` for yours and `::Vector3` for raylib's.
4. [x] **`Ray`** with `origin` (a `Point3`), `direction` (a `Vector3`), `At(t)`. `Point3` is its own type,
   not an alias as in the book, with three mixed operators (`P − P → V`, `P + V → P`, `P − V → P`);
   `static_assert`s on concepts pin what must *not* compile (`P + P`, `P * float`, `V − P`). The direction
   is not necessarily a unit vector. (Devlog 2026-10-25.)
5. [ ] **Camera basics + sky** (RTiOW 4): viewport, `pixel_delta_u/v`, one ray per pixel,
   lerp from white to light blue by the ray's y direction.
   (The lerp is `(1 − a) * white + a * blue`, so `Color` needs `+` and `float * Color` first.)
   (Recomputing the camera for every pixel is wasteful. That is fine for now, and M3 fixes it.)

**Side quest: the rule of five, in practice.** The app keeps a raw raylib `Texture2D` and has to call
`UnloadTexture()` by hand (`AppState::texture` in `src/app/app_state.h`; unloaded in
`src/app/render_job.cpp` and at the end of `src/app/main.cpp`).
- [ ] Write a small RAII `Texture` class: it owns the handle, its destructor unloads it, copying is
  deleted, and moving transfers ownership (the moved-from object gets an empty handle).
  (raylib already has a global `struct Texture`. A class of the same name in an unnamed namespace
  makes every use ambiguous. The app code lives in `namespace umbra::app`: put yours there, where
  `Texture` then means yours and raylib's is `::Texture`. Or call it e.g. `OwnedTexture`.)
- [ ] Replace the manual unload calls with it.
- [ ] Experiment: make it copyable with `= default` and resize the image a few times. Watch what happens
  (double unload). Then undo it.

**C++ angle:** value types and the *rule of zero* (`Vector3`, `Framebuffer`) versus resource owners and the
*rule of five* (`Texture`); `constexpr` + `static_assert` (also for what must *not* compile, via a concept).

**Interview story:** "Explain the rule of 0/3/5." Answer with your own code: "`Vector3` and the framebuffer
need no special members. The texture wrapper is move-only because two copies would release the same
GPU handle twice. I tried it, and this is what happened."

**Done when:** the sky gradient is in the viewport, the `Texture` wrapper is in, and
`docs/gallery/m1-*.png` exists.

---

## M2 — Spheres (RTiOW 5–6)

**Goal:** the first 3D object: a sphere shaded by its surface normals, then a big "ground" sphere
under it.

Steps:

1. [ ] **Ray–sphere intersection** (a quadratic equation; look at the discriminant). The first
   image is a flat red disc, and that is the moment it becomes 3D.
2. [ ] Return the hit distance `t`; color by the normal (`0.5 * (n + 1)`).
3. [ ] Simplify the quadratic (the `h = b / 2` trick).
4. [ ] `HitRecord`, something "hittable", a sphere, a list of objects, an `Interval`.
5. [ ] **First unit tests.** Add doctest (or Catch2) via FetchContent and test ray–sphere: hit, miss,
   tangent, ray starting inside. Compare floats with a tolerance. Test `Vector3::Length()` and
   `UnitVector()` here too (no `static_assert`: they are not `constexpr`), and decide whether
   `NearlyEqual` needs a relative tolerance.

**A design choice to make (and write down in the devlog):** RTiOW uses `virtual` + `shared_ptr`.
Alternatives:
- Return `std::optional<HitRecord>` instead of `bool` + an out parameter.
- Store objects by value (`std::vector<sphere>`) instead of a list of pointers.
- `std::variant<sphere, ...>` + `std::visit` instead of virtual functions.

Pick one, and know why. Plain values and arrays will make M8 (threads) and M9 (GPU) easier.

**C++ angle:** `std::optional`, `std::variant` vs. virtual dispatch, ownership (`unique_ptr` /
`shared_ptr` / values), unit testing floating-point code.

**Interview story:** "Why values/variant instead of virtual + shared_ptr?" You will have a concrete
answer.

**Done when:** a small sphere on a large ground sphere, normal-shaded; tests pass in `ctest`.

---

## M3 — A clean image (RTiOW 7–8 + progressive rendering)

**Goal:** a proper camera class, antialiasing, and an image that keeps getting cleaner while you
watch it.

Steps:

1. [ ] **Camera class** (RTiOW 7): precompute the viewport once and keep scene + camera out of
   globals. This replaces the bare `RenderPixel` function with your own design
   (e.g. `camera.Sample(x, y, rng)`).
2. [ ] **Random numbers** with `<random>`: one engine, passed explicitly (it becomes per-thread in M8).
   Why not `rand()`?
3. [ ] **Antialiasing** (RTiOW 8): jitter the ray inside the pixel and average the samples.
4. [ ] **Progressive accumulation in the app.** A float accumulation buffer. Each *pass* adds one
   sample to every pixel, and the displayed image is `sum / passes`. *Render* panel: sample counter,
   target samples. This replaces the row scheduler (plumbing, so Claude can do the UI part). Keep
   `umbra-cli` in sync (`--samples N`).
5. [ ] *(optional)* A `std::mdspan` view over the buffer (needs GCC 15 / recent MSVC), or
   `std::views::cartesian_product` for the pixel loops.

**C++ angle:** `<random>`, class invariants, C++23 ranges/views where they read better than loops.

**Done when:** edges are smooth and the viewport visibly refines from noisy to clean.

---

## M4 — Materials (RTiOW 9–11)

**Goal:** matte, metal and glass. This is where it starts to look like a real renderer.

Steps:

1. [ ] **Diffuse bounce** with a depth limit. Write it as a loop, not recursion, because the GPU (M9)
   will thank you. Fix "shadow acne" (start the hit interval at 0.001).
2. [ ] **Lambertian** reflection; **gamma correction** in `ToPixel`.
3. [ ] **Materials**: a material type + metal with fuzz.
4. [ ] **Glass**: refraction (Snell), total internal reflection, Schlick's approximation; the hollow
   glass sphere trick.

**C++ angle:** keep the material design consistent with your M2 choice; `enum class`; `std::visit`
with an "overloaded" lambda set; why recursion can be a problem.

**Interview story:** "What was shadow acne and how did you fix it?" It is a floating-point precision
story, a cousin of the FMA story in the devlog.

**Done when:** the classic three spheres (glass, matte, metal) on the ground.

---

## M5 — The cover shot (RTiOW 12–14)

**Goal:** a camera you can place, depth of field, and the book's final scene with hundreds of
random spheres.

Steps:

1. [ ] Field of view; `lookfrom` / `lookat` / `vup`.
2. [ ] Defocus blur (aperture, focus distance).
3. [ ] The final random scene. Render it big, save it to `docs/gallery/`.
4. [ ] **Measure:** note the resolution, samples and render time in the devlog. This is your
   baseline for M8 and M9. (It will be slow, which is good motivation for both.)

**Done when:** the cover image is in the gallery, with its render time written down.

---

## M6 — Scene editor

**Goal:** change the scene live, in the GUI.

Steps:

1. [ ] *Scene* panel: list of objects, add / remove / select.
2. [ ] *Properties* panel: position (`DragFloat3`), radius, material type, albedo (`ColorEdit3`),
   fuzz, refraction index.
3. [ ] *Camera* panel: field of view, look-from / look-at, aperture, focus distance.
4. [ ] Any change restarts the accumulation (a "dirty" flag).
5. [ ] **Picking:** click in the viewport, shoot a ray through that pixel, select what it hits.
   Your own ray tracer powers your UI.

**C++ angle:** separating scene data from UI state; editing a `variant` with `std::visit`.

**Done when:** you can build a small scene by hand, and every change re-renders.

---

## M7 — Interactive camera

**Goal:** navigate like in a 3D application.

Steps:

1. [ ] Orbit camera around a target (spherical coordinates): drag = orbit, wheel = zoom,
   middle-drag = pan.
2. [ ] While moving: low resolution, 1 sample per pixel. When still: full resolution, accumulate.
3. [ ] *(optional)* A second panel with a rasterized preview using raylib's 3D drawing
   (`Camera3D`, `DrawSphere`, `DrawGrid`) into a `RenderTexture`. That gives the classic
   "3D view + render view" setup.

**Done when:** moving around feels smooth and the image sharpens as soon as you stop.

---

## M8 — All cores

**Goal:** use every CPU core while the UI never stutters.

Steps:

1. [ ] Split the image into tiles (e.g. 32×32). A pool of `std::jthread` workers takes tiles from an
   `std::atomic` counter.
2. [ ] One random engine per thread.
3. [ ] Cancel a pass with `std::stop_token` when the scene or the camera changes.
4. [ ] Hand the results to the UI thread safely (think about who writes the accumulation buffer, and when).
5. [ ] Add a ThreadSanitizer preset (`-fsanitize=thread`) and keep it clean.
6. [ ] Measure the speedup against the M5 baseline and write it in the devlog.

**C++ angle:** `std::jthread`, `std::stop_token`, `std::atomic` and memory ordering basics, data
races, false sharing. These come up in interviews a lot.

**Done when:** the speedup is roughly the core count, recorded, and TSan reports nothing.

---

## M9 — CUDA backend

**Goal:** the same image on the GPU. You have done this once already (Mandelbrot), and the structure
is the same: one thread per pixel.

Steps:

1. [ ] CMake option `UMBRA_ENABLE_CUDA` (off by default, so the project still builds without `nvcc`).
2. [ ] Shared math headers marked `__host__ __device__` (a `UMBRA_HD` macro that is empty in
   normal builds).
3. [ ] A flat, GPU-friendly scene: plain arrays, no virtual functions, no `shared_ptr`.
4. [ ] One thread per pixel, a simple per-thread RNG (e.g. PCG), the bounce loop from M4. Use `float`:
   GeForce cards run `double` at 1/32 to 1/64 speed.
5. [ ] Copy the result back and upload it as a texture. No CUDA–OpenGL interop needed.
6. [ ] *Render* panel: backend switch (CPU / CUDA).
7. [ ] Compare CPU and GPU images **with a tolerance**, not byte by byte (see the FMA story in the
   devlog).

Notes: keep `.cu` files at the C++ standard `nvcc` supports (C++20 is safe); C++23 stays in host code.

**Done when:** the M5 scene renders on the GPU, the speed is recorded, and the difference from the
CPU image is within the tolerance.

---

## Beyond

Pick whatever looks fun:

- **BVH** (bounding volume hierarchy; _Ray Tracing: The Next Week_): a huge speedup for many
  objects. It is also a great interview topic (trees, cache, SAH).
- Textures: checker, image, Perlin noise.
- Quads and area lights, then the Cornell box.
- Triangle meshes + OBJ loading (e.g. tinyobjloader), then the Stanford bunny or the Utah teapot.
- Scene files (JSON), with `std::expected` for parse errors.
- HDR environment maps.
- A denoiser (Intel Open Image Denoise).
- _Ray Tracing: The Rest of Your Life_: importance sampling, much less noise.

---

## Finishing a milestone

1. Tick the boxes, move ▶ to the next milestone in the status table.
2. Update `umbra::kMilestone` in `src/core/render.h` (it shows up in the GUI).
3. Save a picture to `docs/gallery/mN-short-name.png` (*Save PNG*, or `umbra --screenshot`),
   and add it to the gallery in the README.
4. Write a devlog entry (with an interview story if there is one).
5. Commit, push. Enjoy the picture for a minute.

## C++23 compiler cheat sheet

The skeleton builds with GCC 13. These features need newer compilers:

| Feature | GCC (libstdc++) | Notes |
|---|---|---|
| `std::expected` | 12 | used in the skeleton; Clang needs 19+ with libstdc++ |
| `views::zip`, `enumerate`, `chunk`, `cartesian_product` | 13 | |
| `std::print`, `std::println` | 14 | |
| `std::generator` | 14 | |
| `std::ranges::to` | 14 | |
| deducing `this` | 14 | |
| `std::mdspan` | 15 | |

Not a C++23 feature, but it bites: `std::sqrt` is `constexpr` only from C++26. GCC folds it in a
`static_assert` anyway; Clang refuses it. Keep such calls out of `static_assert`s.

On Ubuntu 24.04: `sudo apt install g++-14`, then configure with `CXX=g++-14 cmake --preset release`
(delete `build/` first). MSVC (Visual Studio 2022, latest update) supports all of these.
Details: [cppreference: compiler support](https://en.cppreference.com/w/cpp/compiler_support/23).
