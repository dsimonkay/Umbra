#include "core/scene.h"

namespace umbra
{

[[nodiscard]] std::optional<HitRecord> Scene::Hit(const Ray &ray, Interval valid_interval) const
{
    std::optional<HitRecord> maybe_result{};

    for (const auto &sphere : spheres)
    {
        if (const auto hit = sphere.Hit(ray, valid_interval); hit.has_value())
        {
            maybe_result = hit;
            // Narrow the interval, implicitly looking for closer hits
            valid_interval.max = hit->t;
        }
    }

    return maybe_result;
}

} // namespace umbra
