#ifndef UMBRA_CORE_COLOR_H
#define UMBRA_CORE_COLOR_H

#include "core/pixel.h"
#include <algorithm>
#include <cstdint>

namespace umbra
{

/// @brief Represents a color with red, green and blue components.
struct Color
{
    float r{};
    float g{};
    float b{};
};

/// @brief Converts a Color to a Pixel.
/// @param color The color to convert.
/// @param alpha The alpha value for the resulting Pixel. Defaults to 255U.
/// @return The corresponding Pixel with the specified alpha value.
/// @details Clamps the color components to the range [0,1] and scales them to the range [0,255] to create a Pixel with
/// the specified alpha value.
[[nodiscard]] constexpr Pixel ToPixel(const Color color, const std::uint8_t alpha = 255U)
{
    return Pixel{static_cast<std::uint8_t>((std::clamp(color.r, 0.0f, 1.0f) * 255.0f) + 0.5f),
                 static_cast<std::uint8_t>((std::clamp(color.g, 0.0f, 1.0f) * 255.0f) + 0.5f),
                 static_cast<std::uint8_t>((std::clamp(color.b, 0.0f, 1.0f) * 255.0f) + 0.5f), alpha};
}

/// @brief Static assertions to verify the correctness of the ToPixel function.
static_assert(ToPixel(Color{0.0f, 0.0f, 0.0f}) == Pixel{0, 0, 0, 255});
static_assert(ToPixel(Color{1.0f, 1.0f, 1.0f}) == Pixel{255, 255, 255, 255});
static_assert(ToPixel(Color{0.5f, 0.5f, 0.5f}) == Pixel{128, 128, 128, 255});
static_assert(ToPixel(Color{2.0f, -1.0f, 0.5f}) == Pixel{255, 0, 128, 255});

} // namespace umbra

#endif // UMBRA_CORE_COLOR_H
