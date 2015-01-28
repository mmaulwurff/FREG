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

#ifndef SHRED_STORAGE_H
#define SHRED_STORAGE_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <QHash>
#pragma GCC diagnostic pop

#include <thread>

class ShredStorage final {
public:
     ShredStorage(int size, qint64 longi_center, qint64 lati_center);
    ~ShredStorage();

    class QByteArray * GetShredData(qint64 longi, qint64 lati);
    void SetShredData(class QByteArray *, qint64 longi, qint64 lati);
    void Shift(int direction, qint64 longitude, qint64 latitude);

    void WriteToFileAllShredData() const;

private:
    Q_DISABLE_COPY(ShredStorage)

    void WriteShred(qint64 longi, qint64 lati) const ;
    void AddShred  (qint64 longi, qint64 lati);
    void Remove    (qint64 longi, qint64 lati);

    void asyncShift(int direction, qint64 longi_center, qint64 lati_center);

    /// -1 - default, 0 - no compression, 4 - best for CPU, 8 - optimal.
    static const int COMPRESSION_LEVEL = 8;

    typedef QPair<qint64 /*longitude*/, qint64 /*latitude*/> LongLat;
    QHash<LongLat, class QByteArray *> storage;
    const int size;

    // lambda stub, so preloadThread is always joinable.
    std::thread * preloadThread = new std::thread([](){});
};

#endif // SHRED_STORAGE_H
