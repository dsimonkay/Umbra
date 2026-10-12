# CLAUDE.md

Guidance for Claude Code in this repository.

## The project and its owner

**Umbra** is a hobby ray tracer with a GUI that looks like a professional tool.

- The plan: [docs/ROADMAP.md](docs/ROADMAP.md). The current milestone is marked ▶.
- The diary: [docs/DEVLOG.md](docs/DEVLOG.md). The newest entry ends with the *next tiny step*.

## Language

- **Everything in the repository is English:** code, comments, docs, commit messages.

## Build, run, look

```bash
cmake --preset release            # first time: also downloads raylib, ImGui, rlImGui
cmake --build --preset release
ctest --preset release            # tests
./build/release/umbra             # the GUI
./build/release/umbra-cli --out renders/cli.png [--width N --height N]
./build/release/umbra --screenshot renders/ui.png   # whole window -> PNG, then quits
```

There is also a `debug` preset (asserts on). Build directories are `build/<preset>/`.

**You cannot see the window, but you can read PNG files.** Look at the result after every change:
- After changing rendering code, run `umbra-cli` and open the PNG.
- After changing UI code, run `umbra --screenshot` and open the PNG. It needs a display: WSLg
  provides one, and on a headless Linux box use `xvfb-run -a ./build/release/umbra --screenshot ...`.

## Architecture

- `src/core/`: the renderer. Plain C++, **no raylib or ImGui includes**. Keep it that way: in M9 this
  logic moves to CUDA. `RenderPixel()` in `render.cpp` is the current entry point.
- `src/io/`: framebuffer to PNG (raylib's image export; no window needed).
- `src/app/`: the GUI. raylib window + Dear ImGui docking via rlImGui. `main.cpp` has the app state, the
  progressive row scheduler (12 ms budget per frame, UI thread) and the panels; `theme.cpp` has the
  colors and fonts.
- `src/cli/`: the headless renderer.
- Dependencies are pinned in `cmake/Dependencies.cmake`. **Ask before adding a new dependency.**

## How to help (important)

He wants to *build* this, not watch it being built. There are two kinds of work:

1. **Plumbing:** CMake, build errors, raylib/ImGui glue, panels and layout, file I/O, test scaffolding,
   tooling. Just do it. Keep changes small and explain them briefly.
2. **The core:** the ray-tracing math (vectors, rays, intersections, cameras, sampling, materials)
   and the C++ design topic of the current milestone (e.g. the RAII texture wrapper in M1, the
   threading design in M8). **Mentor mode by default:** explain the idea, point to the relevant
   RTiOW section, sketch the shape (signatures, pseudo-code), ask a guiding question, review his
   code. Write the full code only when he asks for it ("write it", "show me the code",
   "írd meg", "csináld meg"). Then keep it short and explain it.

If you are not sure which kind a task is, ask in one sentence.

## Working style

- Work on the ▶ milestone. Don't build abstractions for later milestones.
- Small steps. Each one ends with something to look at (a picture, a passing test). Build and run
  after every step.
- Use C++23 where it fits naturally, and say in one sentence why it fits. Check compiler support:
  the baseline is GCC 13; `<print>`, `std::generator` and deducing `this` need GCC 14, `<mdspan>` needs
  GCC 15 (see the cheat sheet at the end of the roadmap).
- Floating point: compare values and images with a tolerance, never byte for byte.
- Follow the existing style (`.clang-format`: Allman braces, 4 spaces, 120 columns; brace
  initialization). Naming (`.clang-tidy`): `snake_case` variables, parameters and members (private
  members end with `_`); `CamelCase` types and functions/methods; `lower_case` namespaces.
- Commit only when asked. Suggest a short imperative commit message in English.

## Session rituals

- **At the start:** read the newest devlog entry and the ▶ milestone. Say in one or two sentences
  where things stand, then suggest the next tiny step.
- **At the end** (or when he says he is done): draft a devlog entry with what got done, what was
  learned, an *Interview story* line if something interesting happened (a bug, a design decision),
  and the **next tiny step** (≤ 30 minutes, concrete).
- **When a milestone is finished:** follow "Finishing a milestone" in the roadmap (tick the boxes,
  move ▶, bump `umbra::milestone`, add a gallery picture, write a devlog entry).
