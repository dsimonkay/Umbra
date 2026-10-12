#include "core/render.hpp"

#include <cstdint>

namespace umbra
{

Pixel RenderPixel(const int x, const int y, const int width, const int height)
{
    const float r = static_cast<float>(x) / static_cast<float>(width - 1);  // 0.0 at the left edge, 1.0 at the right
    const float g = static_cast<float>(y) / static_cast<float>(height - 1); // 0.0 at the top, 1.0 at the bottom
    const float b = 0.0f;

    return Pixel{static_cast<std::uint8_t>(r * 255.0f), static_cast<std::uint8_t>(g * 255.0f),
                 static_cast<std::uint8_t>(b * 255.0f), 255};
}

} // namespace umbra
