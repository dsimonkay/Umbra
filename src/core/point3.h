#ifndef UMBRA_CORE_POINT3_H
#define UMBRA_CORE_POINT3_H

#include "core/vector3.h"

#include <type_traits>

namespace umbra
{
namespace detail
{
/// @brief Concepts for verifying the availability of basic arithmetic operations between types.
/// @{
template <typename Lhs, typename Rhs>
concept CanAdd = requires(Lhs lhs, Rhs rhs) { (lhs + rhs); };

template <typename Lhs, typename Rhs>
concept CanSubtract = requires(Lhs lhs, Rhs rhs) { (lhs - rhs); };

template <typename Lhs, typename Rhs>
concept CanMultiply = requires(Lhs lhs, Rhs rhs) { (lhs * rhs); };
/// @}

} // namespace detail

/// @brief Represents a point in 3D space with x, y, and z coordinates.
struct Point3
{
    friend constexpr bool operator==(const Point3 &, const Point3 &) = default;

    friend constexpr Vector3 operator-(const Point3 &lhs, const Point3 &rhs)
    {
        // P − P → V
        return Vector3{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
    }

    friend constexpr Point3 operator+(const Point3 &point, const Vector3 &vector)
    {
        // P + V → P
        return Point3{point.x + vector.x, point.y + vector.y, point.z + vector.z};
    }

    friend constexpr Point3 operator-(const Point3 &point, const Vector3 &vector)
    {
        // P − V → P
        return Point3{point.x - vector.x, point.y - vector.y, point.z - vector.z};
    }

    float x{};
    float y{};
    float z{};
};

// Basic assumptions.
static_assert(std::is_aggregate_v<Point3>);
static_assert(std::is_trivially_copyable_v<Point3>);

static_assert(!detail::CanAdd<Point3, Point3>, "Adding two points makes no sense.");
static_assert(!detail::CanMultiply<Point3, float>, "Scaling a point makes no sense.");
static_assert(!detail::CanSubtract<Vector3, Point3>, "Vector - Point makes no sense.");
static_assert(!std::is_convertible_v<Point3, Vector3> && !std::is_convertible_v<Vector3, Point3>);

// Static assertions to validate the behavior of Point3 operations.
static_assert(Point3{} == Point3{0.0f, 0.0f, 0.0f}, "Default constructor should initialize to zero.");
static_assert(Point3{1.0f, 2.0f, 3.0f} + Vector3{4.0f, 5.0f, 6.0f} == Point3{5.0f, 7.0f, 9.0f},
              "Addition operator should work correctly.");
static_assert(Point3{4.0f, 6.0f, 8.0f} - Point3{1.0f, 2.0f, 3.0f} == Vector3{3.0f, 4.0f, 5.0f},
              "Subtraction operator should work correctly.");
static_assert(Point3{5.0f, 7.0f, 9.0f} - Vector3{4.0f, 5.0f, 6.0f} == Point3{1.0f, 2.0f, 3.0f},
              "Subtraction operator should work correctly.");

} // namespace umbra

#endif // UMBRA_CORE_POINT3_H
