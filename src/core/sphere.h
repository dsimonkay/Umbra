#ifndef UMBRA_CORE_SPHERE_H
#define UMBRA_CORE_SPHERE_H

#include "core/hit_record.h"
#include "core/interval.h"
#include "core/point3.h"
#include "core/ray.h"

#include <optional>

namespace umbra
{

/// @brief Represents a sphere in 3D space.
struct Sphere
{
    Point3 center{}; // The center of the sphere
    float radius{};  // @pre radius > 0
    [[nodiscard]] std::optional<HitRecord> Hit(const Ray &ray, const Interval ray_t) const;
};

} // namespace umbra

#endif // UMBRA_CORE_SPHERE_H