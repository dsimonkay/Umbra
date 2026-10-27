# umbra_core: the renderer itself. Plain C++, NO raylib / ImGui dependency.
# This is the part that will later be shared with the CUDA backend (M9).
add_library(umbra_core STATIC
    hit_record.cpp
    render.cpp
    sphere.cpp
)
# Includes are written from src/ (#include "core/render.h"); whoever links umbra_core gets that root too.
target_include_directories(umbra_core PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(umbra_core PRIVATE umbra_warnings)
