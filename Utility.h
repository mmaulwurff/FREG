#ifndef UTILITY_H
#define UTILITY_H

/// Checks if value is in bounds [lowerBound, upperBound).
template<typename T>
bool IsInBounds(const T value, const T lowerBound, const T upperBound) {
    return (lowerBound <= value && value < upperBound);
}

#endif // UTILITY_H

