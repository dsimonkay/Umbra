# Devlog

Newest entry on top. Keep entries short. The most important line is **Next tiny step**: something
concrete you can finish in 30 minutes, so the next session starts with doing instead of remembering.

Template:

```
## YYYY-MM-DD — Mx: short title
- Done: ...
- Learned: ...
- Interview story: ... (only if something interesting happened: a bug, a design decision)
- Next tiny step: ...
```

---

## 2026-10-25 — M1: `Vector3`

- Done (10-24 and 10-25):
  - `static_assert`s for `ToPixel` in `core/color.h` (the step that was open since 10-13).
  - `src/core/vector3.h`: `umbra::Vector3`, three `float`s, a plain aggregate. `constexpr` hidden-friend
    operators `+`, `-`, unary `-`, `float * V`, `V * float`, `V / float`, and a defaulted, exact `==`.
    Members: `SquaredLength()`, `Dot()`, `Cross()`, `Length()` and `UnitVector()` (the last two are not
    `constexpr`, see the interview story). `NearlyEqual(a, b, epsilon)` is a separate function with
    `kVector3Epsilon = 1e-6f`. `render.cpp` includes the header, so the compiler really checks its
    `static_assert`s (a header nobody includes is never compiled).
  - `static_assert(std::is_aggregate_v<T>)` and `std::is_trivially_copyable_v<T>` for `Vector3`, `Color` and
    `Pixel`. `Pixel`'s `==` is defaulted inside the struct now, so it cannot forget a member.
  - Naming: constants are `kCamelCase` (Google style) at namespace scope and `snake_case` inside
    functions. `.clang-tidy` and `CLAUDE.md` say so.
  - Decided: my own `Vector3`, not raylib's (the core cannot see `raylib.h`, raylib's is not `constexpr`,
    and M9 needs code that also runs on CUDA). `float`, not a template: `Color` is `float`, and so is M9.
  - Decided: no tags, mixins or CRTP (the 10-13 design notes). They would automate the easy part
    (`T op T → T`); the interesting operations mix types. One `Vector3` for everything, like the book.
    Whether positions get their own `Point3` is decided at `Ray` (M1 step 4).
- Learned:
  - An aggregate has no user-declared constructor (since C++20 even `= default` counts), no private
    members and no virtual functions. `Vector3{1, 2, 3}` copies the values into the members in order, no
    constructor runs; missing values come from the `{}` member initializers. My first version had
    constructors that left the members uninitialized: `Vector3 v;` held garbage, and only `Vector3{}` was
    zero (value-initialization). Without constructors `Vector3 v;` is zero too, and the type follows the
    rule of zero: trivially copyable, `sizeof` is three floats.
  - `==` with a tolerance is not an equality. It is not transitive (0, 6e-7 and 1.2e-6: `a == b`,
    `b == c`, but `a != c`), and a fixed `1e-6` stops meaning "close" above 16, where neighbouring floats
    are further apart than that. So `==` is exact, and the tolerance is an explicit `NearlyEqual` at the
    call site. Whether it needs a relative tolerance waits for the M2 tests.
  - A `static_assert` is only worth something if it can fail. Mutation testing by hand: break one term in
    the code and see whether any assert notices. The axis-vector tests missed all of `Dot`, the `y` of
    unary minus and two of the six terms of `Cross`. `{1, 2, 3} × {4, 5, 6} = {-3, 6, -3}` uses all six
    products (12, 15, 12, 6, 5, 8), and `Cross(a, b) · a == 0` checks the geometry.
  - `Dot(a, b) = |a||b| cos θ`: how much two vectors point the same way (0 = perpendicular), and
    `Length(v) = sqrt(Dot(v, v))`. `Cross(a, b)` is a vector perpendicular to both, as long as the area of
    their parallelogram. Needed for: ray–sphere (M2, dots), shading and reflection (M4, dots), the camera
    basis (M5, crosses).
  - `UnitVector()` of the zero vector is NaN (0/0). It is a documented precondition (`@pre`), as in the book.
- Interview story: "Why does your `static_assert` pass on GCC and fail on Clang?" `Length()` was
  `constexpr` and had `static_assert(Vector3{3, 4, 0}.Length() == 5.0f)`. GCC accepted it; Clang said
  "static assertion expression is not an integral constant expression" and pointed at `__builtin_sqrtf`
  inside `<cmath>`. `std::sqrt` is `constexpr` only from C++26. GCC folds it as a builtin anyway (an
  extension); the standard does not promise it. (MSVC: not tried yet.) Fix: `Length()` and `UnitVector()`
  lost `constexpr`, their asserts became `SquaredLength()` asserts, and the real checks wait for the unit
  tests (M2 step 5). I only saw it because the header was compiled with a second compiler (clang-tidy
  uses Clang as well).
- Next tiny step: M1 step 4, `src/core/ray.h`. First decide `Point3`: `using Point3 = Vector3;` like the
  book, or its own type with three mixed operators (`P − P → V`, `P + V → P`, `P − V → P`; about 15
  lines). Write the decision here. Then `struct Ray { Point3 origin; Vector3 direction; }` with a
  `constexpr` `At(float t)` (`origin + t * direction`), one `static_assert`, and an include from
  `render.cpp`.

## 2026-10-15 — M1 side quest: RAII `Texture` wrapper

- Done (10-14 and 10-15):
  - `src/app/texture.h/.cpp`: `Texture` owns the raylib `Texture2D`. The destructor unloads it, copying is
    deleted, moving transfers ownership (`std::exchange` leaves an empty handle behind). The destructor
    and the move assignment share a private `Unload()`: if valid, unload, then reset the handle to `{}`,
    so a second call does nothing. Besides that it has only `Upload()` and one accessor,
    `const ::Texture2D &Get() const`. No rlImGui include: the viewport calls
    `rlImGuiImageSizeV(&app.texture.Get(), ...)` itself.
  - `Upload(std::span<const Pixel>)` always uploads and `assert`s its precondition: the texture is RGBA8
    and the span holds width × height pixels (`UpdateTexture()` reads that much, whatever the buffer
    holds). `Framebuffer::Pixels()` returns the span; `Data()` stays for C APIs (the PNG export).
  - `AppState::texture` is a `Texture` now; the manual `UnloadTexture()` calls are gone. The dirty flag
    stays in `AppState` (`texture_is_dirty`): `UploadTexture()` in `render_job.cpp` decides *when* to
    upload, the texture only knows *how*.
  - `Run()` in `main.cpp`: `AppState` lives in an inner block that ends before `rlImGuiShutdown()` and
    `CloseWindow()`. A workaround, see *Next tiny step*.
- Learned:
  - `DrawViewport(const AppState &)` makes `app.texture` const, and a const object can only call `const`
    members. Inside a `const` member, `&texture_` is a `const ::Texture2D *`, so a `GetAddress() const`
    returning `::Texture2D *` does not compile.
  - Don't give the resource owner one method per library that uses it (`Texture::rlImGuiImageSizeV()`).
    Lend the raw handle instead, like `unique_ptr::get()` or `vector::data()`. One `const &` accessor is
    enough: every rlImGui function takes `const Texture *`, raylib takes `Texture2D` by value (except
    `GenTextureMipmaps`). A non-const `::Texture2D &Get()` would be a hole: `app.texture.Get() = {}` leaks.
    The const is shallow, as with `const unique_ptr<T>`: it protects the handle, not the GPU object.
  - `std::move` only casts. On a plain C struct like `Texture2D` the "move" is a copy, so the source still
    holds the handle. A move must leave the source in a state where its destructor does nothing
    (`std::exchange(other.texture_, {})`; id 0 makes `IsTextureValid()` false).
  - `app.texture = Texture{...}` needs no `std::move`: a temporary is already an rvalue, so overload
    resolution picks the move assignment (by rule, not by optimization). `std::move` is for named objects.
  - Naming: in the standard library `unique_ptr::release()` gives up ownership *without* freeing, and
    `reset()` frees. So my "free it" function is `Unload()`, not `Release()`.
  - `std::span` (C++20) carries the size with the pointer, so the callee can check it. A bare
    `const Pixel *` can't. The asserts run only in the `debug` preset (`-DNDEBUG` in release).
  - RAII ties cleanup to scope: locals die in reverse order of construction, at the closing brace. The GL
    context is a resource too, but `InitWindow()`/`CloseWindow()` are manual calls, so `app` was destroyed
    after the window. gdb showed `main` → `~AppState()` → `UnloadTexture` → `rlUnloadTexture` → SIGSEGV
    (exit code 139).
  - `layout_path` must outlive `rlImGuiShutdown()` (ImGui saves `layout.ini` there), so it stays outside
    the inner block.
- Interview story: "Explain the rule of 0/3/5." My first move constructor did
  `texture_ = std::move(other.texture_)`. It compiled, but the temporary kept the handle and its
  destructor deleted the GPU texture the app was still using: garbage in the viewport. After the fix,
  the app segfaulted on exit, because RAII had moved the unload from "before `CloseWindow()`" to "end of
  `Run()`". Both bugs have the same cause: with RAII the *when* of cleanup is decided by scopes, not by
  the order of calls, so every resource has to be an object, the window as well.
- Possible cleanup (optional): viewport_panel.cpp still has the old call commented out.
- Next tiny step: back to the core. Add `static_assert`s for `ToPixel` in `core/color.h` (black -> 0,
  white -> 255, 0.5 -> 128, `{2, -1, 0.5}` -> `{255, 0, 128}`), the step from 10-13 that is still open.
  Optional side step: make the window a resource (a small class that calls `InitWindow` + `rlImGuiSetup`
  and their shutdown calls, declared before `app`), then the inner block can go.

## 2026-10-13 — Plumbing: main.cpp split, one CMakeLists.txt per target

- Done (by Claude; nothing changes on screen: the before/after screenshots differ only in the FPS and
  speed numbers):
  - `src/app/main.cpp`: 737 → 160 lines, start-up and the main loop only. The rest is in `app_state.h`,
    `render_job.cpp`, `ui_scale.cpp`, `command_line.cpp` and `panels/` (one file per window).
  - New target `umbra_ui` in `src/ui/`: `theme.cpp` (moved from `src/app/`) and `widgets.cpp` (the
    property grid). It knows nothing about the app.
  - One `CMakeLists.txt` per target directory; the root one has a table of what links what. The
    executables are still in `build/<preset>/`.
  - `umbra_io` links raylib `PRIVATE`; `umbra-cli` lists raylib itself.
  - `CMAKE_CXX_SCAN_FOR_MODULES OFF`: no modules here, and the scan flags GCC 14 gets (`-fmodules-ts`,
    ...) ended up in `compile_commands.json` and broke clang-tidy and clangd.
- Learned:
  - CMake vs. Bazel: `PUBLIC` ≈ `deps`, `PRIVATE` ≈ `implementation_deps`. A PRIVATE dependency of a static
    library still reaches the executable's link line; what PRIVATE stops is the headers and compile
    flags. Nothing checks that a target includes only what it lists (no layering_check): `src/` is the
    include root for everyone.
  - raylib's color macros are a trap inside `namespace umbra`: `BLACK` expands to `Color{ 0, 0, 0, 255 }`,
    which means `umbra::Color` there as soon as `core/color.h` is included ("too many initializers").
- Interview story: "How do you keep a build's dependency graph honest?" `umbra-cli` called raylib
  (`SetTraceLogLevel`) without listing it, and compiled only because `umbra_io` passed raylib on
  (`PUBLIC`). With `PRIVATE` it failed right away (`raylib.h: No such file or directory`), at the line
  that had the hidden dependency.
- Design notes from the chat (not decided yet): a `Vector3Base<T, Tag, Features...>` with mixins like
  `Additive` (T + T → T) would allow `Position + Position`, exactly what the tags should forbid. The
  interesting operations mix types: Point − Point → Vector, Point + Vector → Point. Direction and
  Translation are both vectors (unit length is an invariant, not a separate algebra). So: hand-written
  `Point3` and `Vector3` (later `Normal3`, which transforms differently); first the table of allowed
  operations as `static_assert`s, plus a list of what must not compile; a CRTP base (like pbrt-v4's
  `Tuple3`) only when the duplication hurts. Open question: the midpoint of two points, if
  `Point + Point` is not allowed?
- Next tiny step: add `static_assert`s for `ToPixel` in `core/color.h` (black -> 0, white -> 255,
  0.5 -> 128, `{2, -1, 0.5}` -> `{255, 0, 128}`). Then read RTiOW 3.1 and write the bare vector type(s)
  with `operator+` and `operator-`, one `static_assert` each.

## 2026-10-13 — M1: gradient, colors as floats

- Done (two sessions, 10-12 and 10-13):
  - M1 step 1: `RenderPixel` draws the RTiOW chapter 2 gradient (black, red, green and yellow corners).
  - M1 step 2: `core/color.h` with `Color` (three floats, `r`/`g`/`b`) and `constexpr ToPixel()`, the one
    place where a color becomes bytes. It clamps to [0, 1] and rounds to the nearest byte.
  - Naming convention: `CamelCase` types and functions, `snake_case` variables; `.clang-tidy` checks it
    (`.clang-format` cannot). Headers renamed from `.hpp` to `.h`. `umbra-cli` rejects sizes below 2.
  - GUI: the UI scale follows the system or *View ▸ UI scale*; settings are saved in `~/.config/umbra/`.
- Learned:
  - `x / (width - 1)` with two `int`s is integer division: 0 everywhere except the last column.
  - Float to byte, three ways. `c * 255` truncates, so only exactly 1.0 gives 255. `c * 255.999` (the
    book) gives every byte an equal slice of [0, 1]. `c * 255 + 0.5` rounds to the nearest byte: byte `k`
    means `k / 255`, so the error is at most half a step and pixel -> color -> pixel gives the same byte.
    I chose rounding. With `+ 0.5f` instead of `std::round` it stays `constexpr` everywhere.
  - `clang-format` only does layout; naming rules are `clang-tidy`'s `readability-identifier-naming`.
- Interview story: "Why did my `constexpr` function fail to link?" `ToPixel` was declared in `color.h` and
  defined in `color.cpp`. GCC warned `inline function ... used but never defined`, then the linker said
  `undefined reference`. The reason: `constexpr` functions are implicitly `inline`, and an inline function
  must be defined in every translation unit that uses it. In `color.cpp` nothing called it, so the
  compiler emitted no symbol at all. Fix: the body goes into the header. (Same rule for templates, so all
  of `Vector3` will live in a header.)
- Open design question: a tagged `Vector3<Tag>` so that `Position` and `Direction` are different types.
  Distinct types pay off when each has its own set of operations, so the real work is deciding the
  operators between tags (`Position − Position` is a `Direction`; `Position + Direction` is a
  `Position`). `Color` stays a separate struct: it shares almost no operations with the geometric types
  and wants `r`/`g`/`b`. pbrt does the same (`Point3` / `Vector3` / `Normal3`, RGB separate).
- Next tiny step: add `static_assert`s for `ToPixel` in `core/color.h` (black -> 0, white -> 255,
  0.5 -> 128, `{2, -1, 0.5}` -> `{255, 0, 128}`). Then read RTiOW 3.1 and write the bare `Vector3`
  struct with `operator+` and `operator-`, with one `static_assert` each.

## 2026-10-11 — M0: skeleton

- Done: project skeleton set up by Claude: CMake + presets, raylib 6.0 / ImGui docking / rlImGui,
  GUI with *Viewport* and *Render* panels, progressive row rendering, *Save PNG*, `umbra-cli`,
  `--screenshot`. Builds with GCC 13 and 14 (Ubuntu 24.04). Not yet tried with MSVC.
- Learned: raylib is not a ray tracer. It is a small C library for windows, input and drawing,
  named after its author's nickname, Ray.
- Interview story (from the Mandelbrot project, before this one): the CPU and CUDA renderers were
  byte-compared by `make test`. Building the CPU version with and without `-ffp-contract=off` changes
  0.016 % of the pixels in the default view and 0.12 % in a zoomed view. The reason: with
  `-march=native`, GCC fuses `a*b + c` into FMA instructions by default in C++, and `nvcc` does the
  same (`--fmad=true`). FMA rounds once instead of twice, and in a chaotic iteration a one-ulp
  difference can change the escape count. So whether the byte-for-byte test passes depends on
  compiler choices, not on the source. Lesson: compare floating-point images with a tolerance.
- Next tiny step: build and run `umbra` in WSL, toggle *Slow motion*, then `git init` and make the
  first commit.
