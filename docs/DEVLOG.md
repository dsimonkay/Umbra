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
