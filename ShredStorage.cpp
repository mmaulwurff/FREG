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

#include "ShredStorage.h"
#include "Shred.h"
#include <QFile>
#include <thread>

uint qHash(const ShredStorage::LongLat& longLat) {
    return  (quint64(longLat.longitude) & 0b11111111) |
            (quint64(longLat.latitude ) & 0b11111111 << 1);
}

ShredStorage::ShredStorage(const int size_,
        const qint64 longi_center, const qint64 lati_center)
    :
        storage(),
        size(size_),
        emptyWriteBuffers(),
        preloadThread(new std::thread([](){})) // stub, so always joinable.
{
    storage.reserve(size*size);
    emptyWriteBuffers.reserve(size*size);
    for (qint64 i=longi_center-size/2; i<=longi_center+size/2; ++i)
    for (qint64 j= lati_center-size/2; j<= lati_center+size/2; ++j) {
        emptyWriteBuffers.push_back(new QByteArray);
        AddShred(i, j);
    }
}

ShredStorage::~ShredStorage() {
    preloadThread->join();
    delete preloadThread;
    WriteToFileAllShredData();
    qDeleteAll(emptyWriteBuffers);
}

QByteArray* ShredStorage::GetByteArray() {
    QByteArray* const result = emptyWriteBuffers.back();
    emptyWriteBuffers.pop_back();
    result->reserve(40 * 1024);
    return result;
}

void ShredStorage::ReleaseByteArray(QByteArray* const array) const {
    array->clear();
    emptyWriteBuffers.push_back(array);
}

void ShredStorage::WriteToFileAllShredData() const {
    for (auto i=storage.constBegin(); i!=storage.constEnd(); ++i) {
        if ( i.value() ) {
            WriteShred(i.key().longitude, i.key().latitude);
        }
    }
}

void ShredStorage::Shift(const int direction,
        const qint64 longitude_center, const qint64 latitude_center)
{
    preloadThread->join();
    delete preloadThread;
    preloadThread = new std::thread(&ShredStorage::asyncShift, this,
        direction, longitude_center, latitude_center);
}

QByteArray* ShredStorage::GetShredData(const qint64 longi, const qint64 lati) {
    return storage.take(LongLat{longi, lati});
}

void ShredStorage::SetShredData(QByteArray* const data,
        const qint64 longi, const qint64 lati)
{
    storage.insert(LongLat{longi, lati}, data);
}

void ShredStorage::AddShred(const qint64 longitude, const qint64 latitude) {
    QFile file(Shred::FileName(longitude, latitude));
    QByteArray* byteArray;
    if ( file.open(QIODevice::ReadOnly) ) {
        byteArray = GetByteArray();
        *byteArray = qUncompress(file.readAll());
    } else {
        byteArray = nullptr;
    }
    storage.insert(LongLat{longitude, latitude}, byteArray);
}

void ShredStorage::WriteShred(const qint64 longi, const qint64 lati) const {
    QByteArray* const data = storage.value(LongLat{longi, lati});
    if ( data ) {
        QFile file(Shred::FileName(longi, lati));
        if ( file.open(QIODevice::WriteOnly) ) {
            file.write(qCompress(*data, COMPRESSION_LEVEL));
        }
        ReleaseByteArray(data);
    }
}

void ShredStorage::Remove(const qint64 longi, const qint64 lati) {
    storage.remove(LongLat{longi, lati});
}

void ShredStorage::asyncShift(const int direction,
        const qint64 longi_center, const qint64 lati_center)
{
    switch (direction) {
    default: Q_UNREACHABLE(); break;
    case NORTH:
        for (qint64 i=lati_center-size/2; i<=lati_center+size/2; ++i) {
            WriteShred(longi_center + size/2, i);
            Remove    (longi_center + size/2, i);
            AddShred  (longi_center - size/2, i);
        }
        break;
    case EAST:
        for (qint64 i=longi_center-size/2; i<=longi_center+size/2; ++i) {
            WriteShred(i, lati_center - size/2);
            Remove    (i, lati_center - size/2);
            AddShred  (i, lati_center + size/2);
        }
        break;
    case SOUTH:
        for (qint64 i=lati_center-size/2; i<=lati_center+size/2; ++i) {
            WriteShred(longi_center - size/2, i);
            Remove    (longi_center - size/2, i);
            AddShred  (longi_center + size/2, i);
        }
        break;
    case WEST:
        for (qint64 i=longi_center-size/2; i<=longi_center+size/2; ++i) {
            WriteShred(i, lati_center + size/2);
            Remove    (i, lati_center + size/2);
            AddShred  (i, lati_center - size/2);
        }
        break;
    }
}

bool ShredStorage::LongLat::operator==(const ShredStorage::LongLat& other)
const {
    return longitude == other.longitude && latitude == other.latitude;
}
