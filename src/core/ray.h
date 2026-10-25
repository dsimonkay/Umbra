#ifndef UMBRA_CORE_RAY_H
#define UMBRA_CORE_RAY_H

#include "core/point3.h"
#include "core/vector3.h"

#include <type_traits>

namespace umbra
{

/// @brief Represents a ray in 3D space with an origin and a direction.
struct Ray
{
    friend constexpr bool operator==(const Ray &, const Ray &) = default;

    constexpr Point3 At(const float t) const { return origin + t * direction; }

    Point3 origin{};
    Vector3 direction{}; ///< @note Not necessarily a unit vector.
};

// Basic assumptions.
static_assert(std::is_aggregate_v<Ray>);
static_assert(std::is_trivially_copyable_v<Ray>);

// Test the At function.
static_assert(Ray{Point3{1.0f, 2.0f, 3.0f}, Vector3{4.0f, 5.0f, 6.0f}}.At(0.0f) == Point3{1.0f, 2.0f, 3.0f},
              "At function with t=0 should return the origin.");
static_assert(Ray{Point3{1.0f, 2.0f, 3.0f}, Vector3{4.0f, 5.0f, 6.0f}}.At(1.0f) == Point3{5.0f, 7.0f, 9.0f},
              "At function with t=1 should return origin + direction.");
static_assert(Ray{Point3{1.0f, 2.0f, 3.0f}, Vector3{4.0f, 5.0f, 6.0f}}.At(2.0f) == Point3{9.0f, 12.0f, 15.0f},
              "At function with t=2 should return origin + 2 * direction.");
static_assert(Ray{Point3{1.0f, 2.0f, 3.0f}, Vector3{4.0f, 5.0f, 6.0f}}.At(-0.5f) == Point3{-1.0f, -0.5f, 0.0f},
              "At shall work correctly with a negative t as well.");

} // namespace umbra

#endif // UMBRA_CORE_RAY_H
