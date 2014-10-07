    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2014 Alexander 'mmaulwurff' Kromm
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

#include <QFile>
#include "ShredStorage.h"
#include "World.h"
#include "Shred.h"

/// -1 - default for zlib, 0 - no compression, 4 - best for CPU, 8 - optimal.
const int COMPRESSION_LEVEL = 8;

bool LongLat::operator==(const LongLat & coords) const {
    return ( longitude==coords.longitude &&
              latitude==coords.latitude );
}

LongLat::LongLat(const qint64 longi, const qint64 lati) :
        longitude(longi),
        latitude(lati)
{}

uint qHash(const LongLat & coords) {
    return ((coords.longitude & 0xffff) << 16) |
            (coords.latitude  & 0xffff);
}

ShredStorage::ShredStorage(const int size_,
        const qint64 longi_center, const qint64 lati_center)
    :
        storage(),
        size(size_)
{
    storage.reserve(size*size);
    for (qint64 i=longi_center-size/2; i<=longi_center+size/2; ++i)
    for (qint64 j= lati_center-size/2; j<= lati_center+size/2; ++j) {
        AddShredData(i, j);
    }
}

ShredStorage::~ShredStorage() {
    if ( preloadThread != nullptr ) {
        preloadThread->wait();
        delete preloadThread;
    }
    for (auto i=storage.constBegin(); i!=storage.constEnd(); ++i) {
        if ( i.value() ) {
            WriteToFileShredData(i.key().longitude, i.key().latitude);
        }
    }
}

void ShredStorage::Shift(const int direction,
        const qint64 longitude_center, const qint64 latitude_center)
{
    if ( preloadThread != nullptr ) {
        preloadThread->wait();
        delete preloadThread;
    }
    preloadThread = new PreloadThread(this, direction,
        longitude_center, latitude_center, size);
    preloadThread->start();
}

QByteArray* ShredStorage::GetShredData(const qint64 longi, const qint64 lati) {
    return storage.take(LongLat(longi, lati));
}

void ShredStorage::SetShredData(QByteArray * const data,
        const qint64 longi, const qint64 lati)
{
    const LongLat coords(longi, lati);
    delete storage.value(coords);
    storage.insert(coords, data);
}

void ShredStorage::AddShredData(const qint64 longitude, const qint64 latitude){
    QFile file(Shred::FileName(world->WorldName(), longitude, latitude));
    storage.insert(LongLat(longitude, latitude),
        ( file.open(QIODevice::ReadOnly) ?
            new QByteArray(qUncompress(file.readAll())) : nullptr ));
}

void ShredStorage::WriteToFileShredData(const qint64 longi, const qint64 lati){
    const QByteArray * const data = storage.value(LongLat(longi, lati));
    if ( data != nullptr ) {
        QFile file(Shred::FileName(world->WorldName(), longi, lati));
        if ( file.open(QIODevice::WriteOnly) ) {
            file.write(qCompress(*data, COMPRESSION_LEVEL));
        }
        delete data;
    }
}

void ShredStorage::Remove(const qint64 longi, const qint64 lati) {
    storage.remove(LongLat(longi, lati));
}

PreloadThread::PreloadThread(ShredStorage * const stor, const int dir,
        const qint64 longi_c, const qint64 lati_c, const int sz)
    :
        storage(stor),
        direction(dir),
        longi_center(longi_c),
        lati_center(lati_c),
        size(sz)
{}

void PreloadThread::run() {
    switch (direction) {
    case NORTH:
        for (long i=lati_center-size/2; i<=lati_center+size/2; ++i) {
            storage->WriteToFileShredData(longi_center+size/2, i);
            storage->Remove(longi_center+size/2, i);
            storage->AddShredData(longi_center-size/2, i);
        }
    break;
    case SOUTH:
        for (long i=lati_center-size/2; i<=lati_center+size/2; ++i) {
            storage->WriteToFileShredData(longi_center-size/2, i);
            storage->Remove(longi_center-size/2, i);
            storage->AddShredData(longi_center+size/2, i);
        }
    break;
    case EAST:
        for (long i=longi_center-size/2; i<=longi_center+size/2; ++i) {
            storage->WriteToFileShredData(i, lati_center-size/2);
            storage->Remove(i, lati_center-size/2);
            storage->AddShredData(i, lati_center+size/2);
        }
    break;
    case WEST:
        for (long i=longi_center-size/2; i<=longi_center+size/2; ++i) {
            storage->WriteToFileShredData(i, lati_center+size/2);
            storage->Remove(i, lati_center+size/2);
            storage->AddShredData(i, lati_center-size/2);
        }
    break;
    default: Q_UNREACHABLE(); break;
    }
}
