#ifndef UMBRA_CORE_VECTOR3_H
#define UMBRA_CORE_VECTOR3_H

#include <cmath>

namespace umbra
{
constexpr float kVector3Epsilon = 1e-6f;

struct Vector3
{
    friend constexpr bool operator==(const Vector3 &, const Vector3 &) = default;

    constexpr friend Vector3 operator+(const Vector3 &lhs, const Vector3 &rhs)
    {
        return Vector3{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
    }

    constexpr friend Vector3 operator-(const Vector3 &lhs, const Vector3 &rhs)
    {
        return Vector3{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
    }

    float x{};
    float y{};
    float z{};
};

/// @brief Checks if two Vector3 instances are nearly equal within a given epsilon.
/// @param lhs The first Vector3 instance.
/// @param rhs The second Vector3 instance.
/// @param epsilon The tolerance for component-wise comparison. Defaults to kVector3Epsilon.
/// @return True if every component differs by less than `epsilon`.
constexpr bool NearlyEqual(const Vector3 &lhs, const Vector3 &rhs, const float epsilon = kVector3Epsilon)
{
    return (std::abs(lhs.x - rhs.x) < epsilon) && (std::abs(lhs.y - rhs.y) < epsilon) &&
           (std::abs(lhs.z - rhs.z) < epsilon);
}

// Static assertions to validate the behavior of Vector3 operations and NearlyEqual function.
static_assert(Vector3{} == Vector3{0.0f, 0.0f, 0.0f}, "Default constructor should initialize to zero.");
static_assert(Vector3{1.0f, 2.0f, 3.0f} + Vector3{4.0f, 5.0f, 6.0f} == Vector3{5.0f, 7.0f, 9.0f},
              "Addition operator should work correctly.");
static_assert(Vector3{4.0f, 5.0f, 6.0f} - Vector3{1.0f, 2.0f, 3.0f} == Vector3{3.0f, 3.0f, 3.0f},
              "Subtraction operator should work correctly.");

static_assert(NearlyEqual(Vector3{1.0f, 2.0f, 3.0f}, Vector3{1.0f + (kVector3Epsilon / 2.0f), 2.0f, 3.0f}),
              "NearlyEqual should accept a difference below epsilon.");
static_assert(!NearlyEqual(Vector3{1.0f, 2.0f, 3.0f}, Vector3{1.0f, 2.0f, 3.0f + (2.0f * kVector3Epsilon)}),
              "NearlyEqual should reject a difference above epsilon.");
static_assert(NearlyEqual(Vector3{1.0f, 2.0f, 3.0f}, Vector3{1.0f, 2.0f, 3.1f}, 0.5f),
              "NearlyEqual should honor a custom epsilon.");

} // namespace umbra

#endif // UMBRA_CORE_VECTOR3_H
