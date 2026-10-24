#ifndef UMBRA_CORE_PIXEL_H
#define UMBRA_CORE_PIXEL_H

#include <cstdint>
#include <type_traits>

namespace umbra
{

/// @brief One 8-bit RGBA pixel, the final "display" format of the renderer.
/// @note The memory layout matches raylib's PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
///       so a whole Framebuffer can be uploaded to a texture without conversion.
struct Pixel
{
    friend constexpr bool operator==(const Pixel &, const Pixel &) = default;

    std::uint8_t r{0};
    std::uint8_t g{0};
    std::uint8_t b{0};
    std::uint8_t a{255};
};

// Basic assumptions.
static_assert(std::is_aggregate_v<Pixel>);
static_assert(std::is_trivially_copyable_v<Pixel>);
static_assert(sizeof(Pixel) == 4, "Pixel must stay tightly packed (texture upload relies on it)");

} // namespace umbra

#endif // UMBRA_CORE_PIXEL_H
