#ifndef UMBRA_CORE_SCENE_H
#define UMBRA_CORE_SCENE_H

#include "core/hit_record.h"
#include "core/interval.h"
#include "core/ray.h"
#include "core/sphere.h"

#include <optional>
#include <vector>

namespace umbra
{

/// @brief Represents a 3D scene to be rendered, containing multiple spheres.
struct Scene
{
    std::vector<Sphere> spheres{};
    [[nodiscard]] std::optional<HitRecord> Hit(const Ray &ray, Interval ray_t) const; // the closest one
};

} // namespace umbra

#endif // UMBRA_CORE_SCENE_H
