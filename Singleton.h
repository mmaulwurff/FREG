#ifndef SINGLETON_H
#define SINGLETON_H

#include <cassert>

template <typename T> class Singleton {
public:

    Singleton(T* const setInstance) {
        // Only one instance of singleton is allowed.
        assert(instance == nullptr);
        instance = setInstance;
    }

    static T* GetInstance() { return instance; }

private:

    Singleton(const Singleton& other) = delete;
    Singleton& operator=(const Singleton& other) = delete;

    static T* instance;
};

template <typename T>
T* Singleton<T>::instance = nullptr;

#endif // SINGLETON_H
