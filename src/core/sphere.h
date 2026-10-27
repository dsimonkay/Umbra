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
    [[nodiscard]] std::optional<HitRecord> Hit(const Ray &ray, const Interval &valid_interval) const;

    Point3 center{}; // The center of the sphere
    float radius{};  // @pre radius > 0
};

// Basic assumptions.
static_assert(std::is_aggregate_v<Sphere>);
static_assert(std::is_trivially_copyable_v<Sphere>);

} // namespace umbra

#endif // UMBRA_CORE_SPHERE_H