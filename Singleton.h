    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2015 Alexander 'mmaulwurff' Kromm
    *  mmaulwurff@gmail.com
    *
    * This file is part of FREG.
    *
    * FREG is free software: you can redistribute it and/or modify
    * it under the terms of the GNU General Public License as published by
    * the Free Software Foundation, either version 3 of the License, or
    * (at your option) any later version.
    *
    * FREG is distributed in the hope that it will be useful,
    * but WITHOUT ANY WARRANTY; without even the implied warranty of
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    * GNU General Public License for more details.
    *
    * You should have received a copy of the GNU General Public License
    * along with FREG. If not, see <http://www.gnu.org/licenses/>. */

#ifndef SINGLETON_H
#define SINGLETON_H

#include <cassert>

template <typename T> class Singleton {
public:

    explicit Singleton(T* const setInstance) {
        // Only one instance of singleton is allowed.
        assert(instance == nullptr);
        instance = setInstance;
    }

    static T* GetInstance() { return instance; }

protected:

    Singleton(const Singleton& other) = delete;
    Singleton& operator=(const Singleton& other) = delete;

private:

    static T* instance;
};

template <typename T>
T* Singleton<T>::instance = nullptr;

#endif // SINGLETON_H
