#ifndef UMBRA_CORE_SCENE_H
#define UMBRA_CORE_SCENE_H

#include "core/hit_record.h"
#include "core/interval.h"
#include "core/ray.h"
#include "core/sphere.h"

#include <optional>
#include <type_traits>
#include <vector>

namespace umbra
{

/// @brief Represents a 3D scene to be rendered, containing multiple spheres.
struct Scene
{
    /// @brief Computes the closest hit of a scene object against a ray.
    /// @param ray The emitted ray, trying to hit an object/a surface
    /// @param valid_interval The interval within a hit counts as valid
    /// @return The optional hit record
    [[nodiscard]] std::optional<HitRecord> Hit(const Ray &ray, Interval valid_interval) const;

    std::vector<Sphere> spheres{};
};

// Basic assumptions.
static_assert(std::is_aggregate_v<Scene>);
static_assert(!std::is_trivially_copyable_v<Scene>); // Because of the std::vector member.

} // namespace umbra

#endif // UMBRA_CORE_SCENE_H
