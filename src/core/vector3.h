#ifndef UMBRA_CORE_VECTOR3_H
#define UMBRA_CORE_VECTOR3_H

#include <cmath>
#include <type_traits>

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

    constexpr friend Vector3 operator-(const Vector3 &arg) { return Vector3{-arg.x, -arg.y, -arg.z}; }

    constexpr friend Vector3 operator*(const float factor, const Vector3 &arg)
    {
        return Vector3{factor * arg.x, factor * arg.y, factor * arg.z};
    }

    constexpr friend Vector3 operator*(const Vector3 &arg, const float factor) { return factor * arg; }

    constexpr friend Vector3 operator/(const Vector3 &arg, const float factor)
    {
        return Vector3{arg.x / factor, arg.y / factor, arg.z / factor};
    }

    constexpr float SquaredLength() const { return (x * x) + (y * y) + (z * z); }
    /// @note This function is not constexpr because it uses std::sqrt.
    float Length() const { return std::sqrt(SquaredLength()); }

    /// @brief Returns the unit vector (normalized vector) of this vector.
    /// @pre The vector must not be the zero vector.
    /// @return A Vector3 instance with the same direction but a length of 1.
    Vector3 UnitVector() const { return *this / Length(); }

    constexpr Vector3 Cross(const Vector3 &other) const
    {
        return Vector3{y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
    }

    constexpr float Dot(const Vector3 &other) const { return x * other.x + y * other.y + z * other.z; }

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

// Basic assumptions.
static_assert(std::is_aggregate_v<Vector3>);
static_assert(std::is_trivially_copyable_v<Vector3>);

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

// Scalar multiplication and division tests.
static_assert((2.0f * Vector3{1.0f, 2.0f, 3.0f}) == Vector3{2.0f, 4.0f, 6.0f},
              "Scalar multiplication should work correctly.");
static_assert((Vector3{1.0f, 2.0f, 3.0f} * 2.0f) == Vector3{2.0f, 4.0f, 6.0f},
              "Scalar multiplication should work correctly.");
static_assert((Vector3{2.0f, 4.0f, 6.0f} / 2.0f) == Vector3{1.0f, 2.0f, 3.0f},
              "Scalar division should work correctly.");

// Unary negation test.
static_assert(-Vector3{1.0f, -2.0f, 3.0f} == Vector3{-1.0f, 2.0f, -3.0f}, "Unary negation should work correctly.");

// Squared length tests.
static_assert(Vector3{3.0f, 4.0f, 0.0f}.SquaredLength() == 25.0f, "Squared length should be calculated correctly.");
static_assert(Vector3{0.0f, 3.0f, 4.0f}.SquaredLength() == 25.0f, "Squared length should be calculated correctly.");
static_assert(Vector3{3.0f, 0.0f, 4.0f}.SquaredLength() == 25.0f, "Squared length should be calculated correctly.");

// Dot and cross product tests.
static_assert(Vector3{1.0f, 2.0f, 3.0f}.Dot(Vector3{4.0f, 5.0f, 6.0f}) == 32.0f, "Dot product should work correctly.");
static_assert(Vector3{1.0f, 2.0f, 3.0f}.Cross(Vector3{4.0f, 5.0f, 6.0f}) == Vector3{-3.0f, 6.0f, -3.0f},
              "Cross product should work correctly.");
static_assert(Vector3{1.0f, 2.0f, 3.0f}.Cross(Vector3{4.0f, 5.0f, 6.0f}).Dot(Vector3{1.0f, 2.0f, 3.0f}) == 0.0f,
              "Cross product should be orthogonal to the original vectors.");

} // namespace umbra

#endif // UMBRA_CORE_VECTOR3_H
