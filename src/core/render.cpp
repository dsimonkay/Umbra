#include "core/render.h"
#include "core/color.h"
#include "core/ray.h"

#include <cmath>

namespace umbra
{
namespace
{

float HitSphere(const Point3 &sphere_center, const float radius, const Ray &ray)
{
    const Vector3 origin_to_center = sphere_center - ray.origin;
    const auto a = ray.direction.SquaredLength();
    const auto h = ray.direction.Dot(origin_to_center);
    const auto c = origin_to_center.SquaredLength() - (radius * radius);
    const auto discriminant = (h * h) - (a * c);

    if (discriminant < 0)
    {
        return -1.0;
    }
    else
    {
        return (h - std::sqrt(discriminant)) / a;
    }
}

Color RayColor(const Ray &ray)
{
    constexpr auto sphere_center = Point3{0.0f, 0.0f, -1.0f};

    const auto t = HitSphere(sphere_center, 0.5f, ray);
    if (t > 0.0f)
    {
        const Vector3 normal = (ray.At(t) - sphere_center).UnitVector();
        return 0.5f * Color(normal.x + 1.0f, normal.y + 1.0f, normal.z + 1.0f);
    }

    const auto unit_direction = ray.direction.UnitVector();
    const auto a = 0.5f * (unit_direction.y + 1.0f);

    return (1.0f - a) * Color{1.0f, 1.0f, 1.0f} + a * Color{0.5f, 0.7f, 1.0f};
}

} // namespace

Pixel RenderPixel(const int x, const int y, const int image_width, const int image_height)
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

    return ToPixel(RayColor(ray));
}

} // namespace umbra
