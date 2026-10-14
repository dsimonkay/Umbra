# umbra_io: framebuffer -> image file (uses raylib's image export, no window).
add_library(umbra_io STATIC
    image_io.cpp
)
# image_io.h includes core/framebuffer.h: PUBLIC. raylib is used only inside image_io.cpp: PRIVATE,
# so its headers don't reach the targets that link umbra_io. (A static library cannot link anything
# itself, so raylib still ends up on the executable's link line; PRIVATE only keeps the headers and
# compile settings to ourselves.)
target_link_libraries(umbra_io
    PUBLIC umbra_core
    PRIVATE raylib umbra_warnings)
