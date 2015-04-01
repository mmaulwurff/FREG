#ifndef SORTEDARRAY_H
#define SORTEDARRAY_H

#include <algorithm>

template<typename Type, Type ... elements>
class SortedArray {
public:
    SortedArray() : data{elements...} { std::sort(data, std::end(data)); }

    const Type* end()   const { return std::end(data); }
    const Type* begin() const { return std::begin(data); }

    Type data[sizeof...(elements)];
};

#endif // SORTEDARRAY_H
