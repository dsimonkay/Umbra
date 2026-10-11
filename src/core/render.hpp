#ifndef UMBRA_CORE_RENDER_HPP
#define UMBRA_CORE_RENDER_HPP

#include "core/pixel.hpp"

#include <string_view>

namespace umbra
{

/// @brief Shown in the GUI. Bump it when you finish a milestone (docs/ROADMAP.md).
inline constexpr std::string_view milestone{"M0 - skeleton"};

/// @brief Compute the color of a single pixel. This is the heart of the renderer.
///
/// Both the GUI and the CLI call this for every pixel, row by row, top to bottom.
/// (0, 0) is the top-left corner; width and height are the full image size.
///
/// It is a plain function on purpose: no hidden state, so later (M8) it can be
/// called from many threads at once, and (M9) its logic can move to the GPU.
[[nodiscard]] Pixel render_pixel(int x, int y, int width, int height);

} // namespace umbra

#endif // UMBRA_CORE_RENDER_HPP
