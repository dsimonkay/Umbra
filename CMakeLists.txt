cmake_minimum_required(VERSION 3.28)

project(umbra
    VERSION 0.1.0
    DESCRIPTION "A hobby ray tracer with a GUI"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # error instead of silent fallback
set(CMAKE_CXX_EXTENSIONS OFF)          # -std=c++23, not -std=gnu++23
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)    # no C++20 modules here; with GCC 14 the scan flags (-fmodules-ts, ...)
                                       # end up in compile_commands.json and break clangd and clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # for clangd / VS Code IntelliSense

# Third-party code: raylib, Dear ImGui (docking branch), rlImGui.
# Downloaded automatically on the first configure (needs git + internet).
include(cmake/Dependencies.cmake)

# ---------------------------------------------------------------------------
# Warnings: only for our own targets, not for the third-party libraries.
# ---------------------------------------------------------------------------
add_library(umbra_warnings INTERFACE)
if(MSVC)
    target_compile_options(umbra_warnings INTERFACE /W4 /permissive- /utf-8)
else()
    target_compile_options(umbra_warnings INTERFACE -Wall -Wextra -Wpedantic -Wshadow)
endif()

# ---------------------------------------------------------------------------
# Our targets: one directory under src/ each, with its own CMakeLists.txt.
#
#   target       directory   links                                      what it is
#   umbra_core   src/core    -                                          the renderer, plain C++
#   umbra_io     src/io      PUBLIC umbra_core, PRIVATE raylib          framebuffer -> PNG
#   umbra_ui     src/ui      PUBLIC imgui raylib, PRIVATE rlimgui       the look, small widgets
#   umbra        src/app     umbra_core umbra_io umbra_ui imgui         the GUI application
#                            raylib rlimgui
#   umbra-cli    src/cli     umbra_core umbra_io raylib                 the headless renderer
#
# The rule: link what you #include. PUBLIC if your own headers include it (Bazel: deps),
# PRIVATE if only your .cpp files do (Bazel: implementation_deps).
#
# Third party (cmake/Dependencies.cmake): rlimgui links imgui and raylib. GLFW, the window
# library, is not a target of ours: raylib compiles its own copy (raylib/src/external/glfw)
# into libraylib.a.
# ---------------------------------------------------------------------------

# Executables go to the top of the build directory (build/<preset>/umbra), not to build/<preset>/src/app/.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

enable_testing()

add_subdirectory(src/core)
add_subdirectory(src/io)
add_subdirectory(src/ui)
add_subdirectory(src/app)
add_subdirectory(src/cli)
