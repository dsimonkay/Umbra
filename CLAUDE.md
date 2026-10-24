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
./build/release/umbra --ui-scale 1.5                # UI scale for this run: 1.5, 150% or auto (works with --screenshot)
```

There is also a `debug` preset (asserts on). Build directories are `build/<preset>/`.

**You cannot see the window, but you can read PNG files.** Look at the result after every change:
- After changing rendering code, run `umbra-cli` and open the PNG.
- After changing UI code, run `umbra --screenshot` and open the PNG. It needs a display: WSLg
  provides one, and on a headless Linux box use `xvfb-run -a ./build/release/umbra --screenshot ...`.

## Architecture

One CMake target per directory under `src/`, each with its own `CMakeLists.txt`; the root
`CMakeLists.txt` has a table of what links what. Includes are written from `src/` (`#include "core/render.h"`).

- `src/core/` (`umbra_core`): the renderer. Plain C++, **no raylib or ImGui includes**. Keep it that way:
  in M9 this logic moves to CUDA. `RenderPixel()` in `render.cpp` is the current entry point.
- `src/io/` (`umbra_io`): framebuffer to PNG (raylib's image export, linked privately; no window needed).
- `src/ui/` (`umbra_ui`): the look, and nothing about the app. `theme.cpp` has the colors, fonts and UI
  scaling (hard-coded pixel sizes go through `ui::Px()` so they follow the scale); `widgets.cpp` has the
  "label | widget" property grid.
- `src/app/` (`umbra`): the GUI. raylib window + Dear ImGui docking via rlImGui.
  - `main.cpp`: start-up, the main loop, shut-down. `app_state.h`: `AppState`, everything kept between frames.
  - `render_job.cpp`: the progressive row scheduler (12 ms budget per frame, UI thread), the texture
    upload, *Save PNG*.
  - `panels/`: one file per window (main menu + shortcuts, status bar, dock space, viewport, render panel,
    about); `panels.h` declares them.
  - `ui_scale.cpp`: the UI scale in effect, either the system's (`ui::SystemScale()`: Xft.dpi on
    Linux/WSLg, display settings on Windows) or a fixed value from *View ▸ UI scale*.
  - `settings.cpp` reads/writes `~/.config/umbra/settings.ini` (the panel layout is `layout.ini` next to
    it; screenshot mode uses neither). `command_line.cpp`: `--screenshot`, `--ui-scale`.
- `src/cli/` (`umbra-cli`): the headless renderer.
- Dependencies are pinned in `cmake/Dependencies.cmake`. **Ask before adding a new dependency.**

Rules that keep it tidy:

- **Link what you include.** A target lists every library it includes directly. `PUBLIC` only if its own
  headers include it, `PRIVATE` otherwise.
- **A constant lives next to its only user.** Colors and sizes are the exception: they belong to the theme.
- **raylib inside `namespace umbra`:** raylib is C, so its names (`Color`, `Image`, `Texture`, `Ray`,
  `Camera`, ...) are global, and an umbra type with the same name hides them. Write raylib's types with a
  leading `::` (`::Color`, `::Image`, `::Texture2D`), and don't use raylib's color macros (`BLACK`,
  `WHITE`, ...): they expand to a plain `Color{...}`, which means `umbra::Color` there. No
  `using namespace umbra;` in a file that includes `raylib.h`.

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
- Floating point: compare values and images with a tolerance, never byte for byte. (In a `static_assert`
  on exactly representable numbers, `==` is fine.)
- Core value types (`Vector3`, `Color`, `Pixel`) are plain aggregates: no constructors, members with `{}`
  initializers, pinned by `static_assert(std::is_aggregate_v<T>)`. `operator==` is exact (`= default`); a
  tolerance is an explicit `NearlyEqual(...)` at the call site.
- `static_assert`s are the unit tests of the core, so make sure they can fail: break one term of the code
  and see that some assert notices. Core headers must compile with GCC *and* Clang (clang-tidy uses
  Clang): no `static_assert` on `std::sqrt` and friends, they are `constexpr` only from C++26.
- Follow the existing style (`.clang-format`: Allman braces, 4 spaces, 120 columns; brace
  initialization). Naming (`.clang-tidy`): `snake_case` variables, parameters and members (private
  members end with `_`); `CamelCase` types and functions/methods; `lower_case` namespaces.
  Constants are `kCamelCase` (Google style): `const` / `constexpr` variables at namespace scope, static
  `const` / `constexpr` members, and enumerators. Inside a function every constant is `snake_case`, like
  any local.
- Commit only when asked. Suggest a short imperative commit message in English.

## Session rituals

- **At the start:** read the newest devlog entry and the ▶ milestone. Say in one or two sentences
  where things stand, then suggest the next tiny step.
- **At the end** (or when he says he is done): draft a devlog entry with what got done, what was
  learned, an *Interview story* line if something interesting happened (a bug, a design decision),
  and the **next tiny step** (≤ 30 minutes, concrete).
- **When a milestone is finished:** follow "Finishing a milestone" in the roadmap (tick the boxes,
  move ▶, bump `umbra::kMilestone`, add a gallery picture, write a devlog entry).
