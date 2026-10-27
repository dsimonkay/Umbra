#include "core/render.h"
#include "core/color.h"
#include "core/scene.h"

#include <limits>

namespace umbra
{
namespace
{

Color RayColor(const Scene &scene, const Ray &ray)
{
    const auto maybe_hit = scene.Hit(ray, {0.001f, std::numeric_limits<float>::infinity()});
    if (maybe_hit.has_value())
    {
        const auto &normal = maybe_hit->normal;
        return 0.5f * Color{normal.x + 1.0f, normal.y + 1.0f, normal.z + 1.0f};
    }

    // Otherwise, compute the background color based on the ray direction.
    const auto unit_direction = ray.direction.UnitVector();
    const auto a = 0.5f * (unit_direction.y + 1.0f);

    return (1.0f - a) * Color{1.0f, 1.0f, 1.0f} + (a * Color{0.5f, 0.7f, 1.0f});
}

} // namespace

Pixel RenderPixel(const Scene &scene, const int x, const int y, const int image_width, const int image_height)
{
    // Camera
    constexpr float focal_length = 1.0f;
    constexpr float viewport_height = 2.0f;
    const float viewport_width = viewport_height * (static_cast<float>(image_width) / static_cast<float>(image_height));
    constexpr auto camera_center = Point3{0.0f, 0.0f, 0.0f};

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    const auto viewport_u = Vector3{viewport_width, 0.0f, 0.0f};
    const auto viewport_v = Vector3{0.0f, -viewport_height, 0.0f};

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    const auto pixel_delta_u = viewport_u / static_cast<float>(image_width);
    const auto pixel_delta_v = viewport_v / static_cast<float>(image_height);

    // Calculate the location of the upper left pixel.
    const auto viewport_upper_left =
        camera_center - Vector3{0.0f, 0.0f, focal_length} - (viewport_u / 2.0f) - (viewport_v / 2.0f);
    const auto pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);

    const auto pixel_center =
        pixel00_loc + (static_cast<float>(x) * pixel_delta_u) + (static_cast<float>(y) * pixel_delta_v);
    const auto ray_direction = pixel_center - camera_center;
    const auto ray = Ray{camera_center, ray_direction};

    return ToPixel(RayColor(scene, ray));
}

} // namespace umbra
