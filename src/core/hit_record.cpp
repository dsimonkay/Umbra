#include "core/hit_record.h"

#include <cassert>
#include <cmath>

namespace umbra
{

void HitRecord::SetFaceNormal(const Ray &ray, const Vector3 &outward_normal)
{
    constexpr float epsilon = 1.0e-6f;
    assert((std::abs(outward_normal.SquaredLength() - 1.0f) < epsilon) && "The outward normal is not a unit vector.");

    front_face = (ray.direction.Dot(outward_normal) < 0.0f);
    normal = front_face ? outward_normal : -outward_normal;
}

} // namespace umbra
