#include "core/render.hpp"

#include <cstdint>

namespace umbra
{

// ---------------------------------------------------------------------------
// M0 placeholder: a neutral checkerboard, the "empty canvas" of image editors.
// It only proves that the pipeline (render -> framebuffer -> texture -> panel)
// works. Milestone M1 replaces it with your first ray. See docs/ROADMAP.md.
// ---------------------------------------------------------------------------
Pixel render_pixel(const int x, const int y, [[maybe_unused]] const int width, [[maybe_unused]] const int height)
{
    constexpr int cell_size{24};
    const bool dark_cell = ((x / cell_size) + (y / cell_size)) % 2 == 0;
    const std::uint8_t value = dark_cell ? 52 : 64;
    return Pixel{value, value, value, 255};
}

} // namespace umbra
