#ifndef UMBRA_CORE_HIT_RECORD_H
#define UMBRA_CORE_HIT_RECORD_H

#include "core/point3.h"
#include "core/ray.h"
#include "core/vector3.h"

namespace umbra
{

/// @brief Represents a record of a ray-object intersection.
struct HitRecord
{
    Point3 point{};    // The point at which the ray intersects the object
    Vector3 normal{};  // Always against the ray
    float t{};         // The parameter t at which the ray intersects the object
    bool front_face{}; // Whether the normal vector of the object is front-facing at the hitting point

    /// @brief Sets the hit record normal vector.
    /// @param ray The ray hitting a surface
    /// @param outward_normal The normal vector of that surface at the hitting point
    void SetFaceNormal(const Ray &ray, const Vector3 &outward_normal);
};

} // namespace umbra

#endif // UMBRA_CORE_HIT_RECORD_H
