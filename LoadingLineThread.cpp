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

#include "LoadingLineThread.h"

LoadingLineThread::LoadingLineThread() :
    running(true),
    thread([this] {
        unsigned c = 0;
        while (running) {
            putchar((c & 0b0111111) + '0');
            fflush(stdout);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            putchar(8);
            ++c;
        }
        putchar(8);
    })
{}

void LoadingLineThread::stop() {
    running = false;
    thread.join();
}
