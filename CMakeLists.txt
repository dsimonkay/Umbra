cmake_minimum_required(VERSION 3.28)

project(umbra
    VERSION 0.1.0
    DESCRIPTION "A hobby ray tracer with a GUI"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # error instead of silent fallback
set(CMAKE_CXX_EXTENSIONS OFF)          # -std=c++23, not -std=gnu++23
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
# umbra_core: the renderer itself. Plain C++, NO raylib / ImGui dependency.
# This is the part that will later be shared with the CUDA backend (M9).
# ---------------------------------------------------------------------------
add_library(umbra_core STATIC
    src/core/render.cpp
)
target_include_directories(umbra_core PUBLIC src)
target_link_libraries(umbra_core PRIVATE umbra_warnings)

# ---------------------------------------------------------------------------
# umbra_io: framebuffer -> image file (uses raylib's image export, no window).
# ---------------------------------------------------------------------------
add_library(umbra_io STATIC
    src/io/image_io.cpp
)
target_link_libraries(umbra_io
    PUBLIC umbra_core raylib
    PRIVATE umbra_warnings)

# ---------------------------------------------------------------------------
# umbra: the GUI application.
# ---------------------------------------------------------------------------
add_executable(umbra
    src/app/main.cpp
    src/app/theme.cpp
)
target_link_libraries(umbra PRIVATE umbra_core umbra_io rlimgui umbra_warnings)
target_compile_definitions(umbra PRIVATE UMBRA_VERSION="${PROJECT_VERSION}")

# The UI font lives next to the executable (assets/), see theme.cpp.
add_custom_command(TARGET umbra POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:umbra>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${imgui_SOURCE_DIR}/misc/fonts/Roboto-Medium.ttf"
            "$<TARGET_FILE_DIR:umbra>/assets/Roboto-Medium.ttf"
    VERBATIM)

# ---------------------------------------------------------------------------
# umbra-cli: renders straight to a PNG file, no window.
# Handy for quick checks, benchmarks and for Claude Code to look at results.
# ---------------------------------------------------------------------------
add_executable(umbra-cli
    src/cli/main.cpp
)
target_link_libraries(umbra-cli PRIVATE umbra_core umbra_io umbra_warnings)

# ---------------------------------------------------------------------------
# Tests (ctest). For now just a smoke test; real unit tests arrive in M2.
# ---------------------------------------------------------------------------
enable_testing()
add_test(NAME cli_smoke
    COMMAND umbra-cli --width 64 --height 36 --out "${CMAKE_BINARY_DIR}/test-output/smoke.png")
