#ifndef UMBRA_CORE_RENDER_H
#define UMBRA_CORE_RENDER_H

#include "core/pixel.h"

#include <string_view>

namespace umbra
{

/// @brief Shown in the GUI. Bump it when you finish a milestone (docs/ROADMAP.md).
inline constexpr std::string_view kMilestone{"M0 - skeleton"};

/// @brief Compute the color of a single pixel. This is the heart of the renderer.
///
/// Both the GUI and the CLI call this for every pixel, row by row, top to bottom.
/// (0, 0) is the top-left corner; width and height are the full image size.
///
/// It is a plain function on purpose: no hidden state, so later (M8) it can be
/// called from many threads at once, and (M9) its logic can move to the GPU.
///
/// @pre The coordinates (x, y) must be within the bounds of the image dimensions (0 <= x < width, 0 <= y < height).
/// @pre The image dimensions (width, height) must be positive integers (width > 1, height > 1).
///
/// @param x The x-coordinate of the pixel.
/// @param y The y-coordinate of the pixel.
/// @param image_width The width of the image.
/// @param image_height The height of the image.
/// @return The rendered Pixel at the specified coordinates.
[[nodiscard]] Pixel RenderPixel(int x, int y, int image_width, int image_height);

} // namespace umbra

#endif // UMBRA_CORE_RENDER_H
