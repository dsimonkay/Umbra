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
    std::vector<Sphere> spheres{};
};

// Basic assumptions.
static_assert(std::is_aggregate_v<Scene>);
static_assert(!std::is_trivially_copyable_v<Scene>); // Because of the std::vector member.

} // namespace umbra

#endif // UMBRA_CORE_SCENE_H
