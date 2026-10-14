# umbra_ui: the look (colors, fonts, UI scale) and small reusable ImGui widgets.
# It knows nothing about the application: no app/ includes, no AppState.
add_library(umbra_ui STATIC
    theme.cpp
    widgets.cpp
)
target_include_directories(umbra_ui PUBLIC "${PROJECT_SOURCE_DIR}/src")
# theme.h includes imgui.h and raylib.h: PUBLIC. rlImGui is used only in theme.cpp: PRIVATE.
target_link_libraries(umbra_ui
    PUBLIC imgui raylib
    PRIVATE rlimgui umbra_warnings)
# theme.cpp asks GLFW (the window library inside raylib) for the monitor's DPI scale.
# raylib already links GLFW; we only need its header, and only here.
target_include_directories(umbra_ui SYSTEM PRIVATE "${raylib_SOURCE_DIR}/src/external/glfw/include")
