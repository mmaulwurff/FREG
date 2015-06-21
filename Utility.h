#ifndef UTILITY_H
#define UTILITY_H

#include <type_traits>

/// Checks if value is in bounds [lowerBound, upperBound).
template<typename T>
bool IsInBounds(const T value, const T lowerBound, const T upperBound) {
    return (lowerBound <= value && value < upperBound);
}

template<typename T>
constexpr T mBound(const T lower, const T n, const T upper) {
    static_assert(std::is_arithmetic<T>::value, "should be ariphmetic.");
    return std::max(std::min(n, upper), lower);
}

#endif // UTILITY_H

