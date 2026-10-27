#include "core/sphere.h"

#include <cassert>
#include <cmath>

namespace umbra
{
std::optional<HitRecord> Sphere::Hit(const Ray &ray, const Interval &valid_interval) const
{
    assert((radius > 0.0f) && "Sphere radius must be positive");

    HitRecord hit{};

    const Vector3 origin_center = center - ray.origin;
    const auto a = ray.direction.SquaredLength();
    const auto h = ray.direction.Dot(origin_center);
    const auto c = origin_center.SquaredLength() - (radius * radius);

    const auto discriminant = (h * h) - (a * c);
    if (discriminant < 0)
    {
        return std::nullopt;
    }

    const auto sqrtd = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (h - sqrtd) / a;
    if ((root <= valid_interval.min) || (valid_interval.max <= root))
    {
        root = (h + sqrtd) / a;
        if ((root <= valid_interval.min) || (root >= valid_interval.max))
        {
            return std::nullopt;
        }
    }

    hit.t = root;
    hit.point = ray.At(hit.t);
    const auto normal = (hit.point - center) / radius;
    hit.SetFaceNormal(ray, normal);

    return hit;
}

} // namespace umbra
