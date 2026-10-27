#ifndef UMBRA_CORE_COLOR_H
#define UMBRA_CORE_COLOR_H

#include "core/pixel.h"

#include <algorithm>
#include <cstdint>
#include <type_traits>

namespace umbra
{

/// @brief Represents a color with red, green and blue components.
struct Color
{
    friend constexpr bool operator==(const Color &, const Color &) = default;

    constexpr friend Color operator+(const Color &lhs, const Color &rhs)
    {
        return Color{lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b};
    }

    constexpr friend Color operator*(const float factor, const Color &color)
    {
        return Color{factor * color.r, factor * color.g, factor * color.b};
    }

    constexpr friend Color operator*(const Color &color, const float factor)
    {
        return Color{factor * color.r, factor * color.g, factor * color.b};
    }

    float r{};
    float g{};
    float b{};
};

// Basic assumptions.
static_assert(std::is_aggregate_v<Color>);
static_assert(std::is_trivially_copyable_v<Color>);

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

static_assert(Color{0.75f, 0.5f, 0.25f} + Color{0.5f, 0.875f, 2.0f} == Color{1.25f, 1.375f, 2.25f},
              "Color addition should work component-wise and must not clamp.");
static_assert(2.0f * Color{0.75f, 1.5f, 3.0f} == Color{1.5f, 3.0f, 6.0f},
              "Scaling a color should work component-wise and must not clamp.");
static_assert((2.0f * Color{0.75f, 1.5f, 3.0f}) == (Color{0.75f, 1.5f, 3.0f} * 2.0f),
              "Commutativity of scalar multiplication should hold.");
static_assert((2.0f * Color{0.75f, 1.5f, 3.0f}) + Color{0.25f, 0.5f, 1.0f} == Color{1.75f, 3.5f, 7.0f},
              "Combined scaling and addition should work component-wise and must not clamp.");

} // namespace umbra

#endif // UMBRA_CORE_COLOR_H
