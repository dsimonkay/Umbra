#include "core/hit_record.h"

#include <cassert>

namespace umbra
{

void HitRecord::SetFaceNormal(const Ray &ray, const Vector3 &outward_normal)
{
    assert((outward_normal.SquaredLength() == 1.0f) && "The outward normal is not a unit vector.");

    front_face = (ray.direction.Dot(outward_normal) < 0.0f);
    normal = front_face ? outward_normal : -outward_normal;
}

} // namespace umbra
