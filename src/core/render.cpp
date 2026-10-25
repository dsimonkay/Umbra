#include "core/render.h"
#include "core/color.h"
#include "core/ray.h"

#include <cstdint>

namespace umbra
{

Pixel RenderPixel(const int x, const int y, const int width, const int height)
{
    const float r = static_cast<float>(x) / static_cast<float>(width - 1);  // 0.0 at the left edge, 1.0 at the right
    const float g = static_cast<float>(y) / static_cast<float>(height - 1); // 0.0 at the top, 1.0 at the bottom
    const float b = 0.0f;

    return ToPixel(Color{r, g, b}, 255U);
}

} // namespace umbra
