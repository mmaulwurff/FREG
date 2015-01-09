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

#include <QHash>
#include <QThread>

class QByteArray;

struct LongLat final {
    LongLat(qint64 longitude, qint64 latitude);
    bool operator==(const LongLat &) const;

    LongLat & operator=(const LongLat &) = delete;

    const qint64 longitude;
    const qint64 latitude;
};

class PreloadThread;

class ShredStorage final {
public:
     ShredStorage(int size, qint64 longi_center, qint64 lati_center);
    ~ShredStorage();

    QByteArray * GetShredData(qint64 longi, qint64 lati);
    void SetShredData(QByteArray *, qint64 longi, qint64 lati);
    void Shift(int direction, qint64 longitude, qint64 latitude);

    void AddShredData(qint64 longi, qint64 lati);
    void WriteToFileShredData(qint64 longi, qint64 lati) const ;
    void WriteToFileAllShredData() const;

    void Remove(qint64 longi, qint64 lati);

private:
    Q_DISABLE_COPY(ShredStorage)

    /// -1 - default, 0 - no compression, 4 - best for CPU, 8 - optimal.
    static const int COMPRESSION_LEVEL = 8;

    QHash<LongLat, QByteArray *> storage;
    const int size;
    PreloadThread * preloadThread = nullptr;
}; // class ShredStorage

class PreloadThread final : public QThread {
    Q_OBJECT
    Q_DISABLE_COPY(PreloadThread)
public:
    PreloadThread(ShredStorage *, int direction,
            qint64 longi_center, qint64 lati_center, int size);

protected:
    void run() override;

private:
    ShredStorage * const storage;
    const int direction;
    const qint64 longi_center;
    const qint64 lati_center;
    const int size;
}; // class PreloadThread

#endif // SHRED_STORAGE_H
