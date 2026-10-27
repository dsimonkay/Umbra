#ifndef UMBRA_CORE_HIT_RECORD_H
#define UMBRA_CORE_HIT_RECORD_H

#include "core/point3.h"
#include "core/vector3.h"

namespace umbra
{

/// @brief Represents a record of a ray-object intersection.
struct HitRecord
{
    Point3 point{};   // The point at which the ray intersects the object
    Vector3 normal{}; // Always against the ray
    float t{};        // The parameter t at which the ray intersects the object
    bool front_face{};
};

} // namespace umbra

#endif // UMBRA_CORE_HIT_RECORD_H
