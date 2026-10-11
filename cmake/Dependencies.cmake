# Third-party dependencies, fetched with FetchContent on the first configure.
# Versions are pinned on purpose: a hobby project should not break overnight
# because something upstream moved.

include(FetchContent)

# --- raylib: window, input, textures, (later) 3D preview -------------------
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE)

# --- Dear ImGui, docking branch: the panels --------------------------------
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.9b-docking
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE)

# --- rlImGui: glue between raylib and ImGui (main branch, 2026-08-24) ------
FetchContent_Declare(rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        1550009359ad975927f7f0e4a3f47e3f27123ea9)

FetchContent_MakeAvailable(raylib imgui rlimgui)

# ImGui and rlImGui ship without a CMakeLists.txt, so we build them here.
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp)
target_include_directories(imgui SYSTEM PUBLIC ${imgui_SOURCE_DIR})

add_library(rlimgui STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp)
target_include_directories(rlimgui SYSTEM PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui PUBLIC imgui raylib)
# Size of the Font Awesome icons merged into the UI font (see theme.cpp).
target_compile_definitions(rlimgui PUBLIC FONT_AWESOME_ICON_SIZE=14)
