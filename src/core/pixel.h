#ifndef UMBRA_CORE_PIXEL_H
#define UMBRA_CORE_PIXEL_H

#include <cstdint>

namespace umbra
{

/// @brief One 8-bit RGBA pixel, the final "display" format of the renderer.
/// @note The memory layout matches raylib's PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
///       so a whole Framebuffer can be uploaded to a texture without conversion.
struct Pixel
{
    std::uint8_t r{0};
    std::uint8_t g{0};
    std::uint8_t b{0};
    std::uint8_t a{255};
};

/// @brief Compares two Pixels for equality.
/// @param lhs The first Pixel to compare.
/// @param rhs The second Pixel to compare.
/// @return True if all components of the Pixels are equal, false otherwise.
constexpr bool operator==(const Pixel lhs, const Pixel rhs)
{
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b) && (lhs.a == rhs.a);
}

static_assert(sizeof(Pixel) == 4, "Pixel must stay tightly packed (texture upload relies on it)");

} // namespace umbra

#endif // UMBRA_CORE_PIXEL_H
