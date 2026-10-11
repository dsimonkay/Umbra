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
