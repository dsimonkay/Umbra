#include "core/scene.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

namespace umbra
{
namespace
{
using Catch::Matchers::WithinAbs;

constexpr float kEpsilon = 1.0e-5f;

TEST_CASE("A ray hitting (or not) scene elements", "[scene]")
{
    Scene scene{};
    constexpr float sphere_radius = 0.5f;
    const Ray ray{Point3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, 0.0f, -1.0f}};

    SECTION("Nothing gets hit in an empty scene")
    {
        CHECK_FALSE(scene.Hit(ray, Interval{}).has_value());
    }

    SECTION("A sphere placed outside the path of the ray does not get hit")
    {
        scene.spheres.push_back({Point3{-1.0f, 0.0f, -1.0f}, sphere_radius});
        const auto hit = scene.Hit(ray, Interval{});
        CHECK_FALSE(hit.has_value());
    }

    SECTION("A sphere placed along the path of the ray gets hit")
    {
        scene.spheres.push_back({Point3{0.0f, 0.0f, -1.0f}, sphere_radius});
        const auto hit = scene.Hit(ray, Interval{});
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit->t, WithinAbs(0.5, kEpsilon));
    }

    SECTION("When two spheres are placed along the ray path, the closer one gets hit (case 1: the closer one gets "
            "added last)")
    {
        scene.spheres.push_back({Point3{0.0f, 0.0f, -12.0f}, sphere_radius});
        scene.spheres.push_back({Point3{0.0f, 0.0f, -2.0f}, sphere_radius});
        const auto hit = scene.Hit(ray, Interval{});
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit->t, WithinAbs(1.5, kEpsilon));
    }

    SECTION("When two spheres are placed along the ray path, the closer one gets hit (case 2: the closer one gets "
            "added first)")
    {
        scene.spheres.push_back({Point3{0.0f, 0.0f, -2.0f}, sphere_radius});
        scene.spheres.push_back({Point3{0.0f, 0.0f, -12.0f}, sphere_radius});
        const auto hit = scene.Hit(ray, Interval{});
        REQUIRE(hit.has_value());
        REQUIRE_THAT(hit->t, WithinAbs(1.5, kEpsilon));
    }
}

} // namespace
} // namespace umbra
