#ifndef UMBRA_CORE_INTERVAL_H
#define UMBRA_CORE_INTERVAL_H

#include <limits>

namespace umbra
{

/// @brief Represents a numerical interval with a minimum and maximum value.
///        It will be used to limit the range of a ray hitting objects.
struct Interval
{
    float min{0.0f};
    float max{std::numeric_limits<float>::max()};
};

} // namespace umbra

#endif // UMBRA_CORE_INTERVAL_H
