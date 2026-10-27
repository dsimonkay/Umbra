#ifndef UMBRA_CORE_INTERVAL_H
#define UMBRA_CORE_INTERVAL_H

#include <limits>
#include <type_traits>

namespace umbra
{

/// @brief Represents a numerical interval with a minimum and maximum value.
struct Interval
{
    float min{-std::numeric_limits<float>::infinity()};
    float max{std::numeric_limits<float>::infinity()};
};

// Basic assumptions.
static_assert(std::is_aggregate_v<Interval>);
static_assert(std::is_trivially_copyable_v<Interval>);

} // namespace umbra

#endif // UMBRA_CORE_INTERVAL_H
