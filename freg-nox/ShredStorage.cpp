	/* freg, Free-Roaming Elementary Game with open and interactive world
	*  Copyright (C) 2012-2013 Alexander 'mmaulwurff' Kromm
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
	* along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#include <QByteArray>
#include <QFile>
#include "ShredStorage.h"
#include "world.h"
#include "Shred.h"

bool LongLat::operator==(const LongLat & coords) const {
	return ( longitude==coords.longitude &&
	          latitude==coords.latitude );
}

uint qHash(const LongLat coords) {
	// there should not be collisions.
	return ((coords.longitude & 0x3f) << 6) + (coords.latitude & 0x3f);
}

ShredStorage::ShredStorage(const World * const world_,
		const ushort size_,
		const long longi_center, const long lati_center)
	:
		size(size_),
		world(world_)
{
	storage.reserve(size*size);
	for (long i=longi_center-size/2; i<=longi_center+size/2; ++i)
	for (long j= lati_center-size/2; j<= lati_center+size/2; ++j) {
		QFile file(Shred::FileName(world->WorldName(), i, j));
		const LongLat coords={i, j};
		storage.insert(coords, ( file.open(QIODevice::ReadOnly) ?
			new QByteArray(file.readAll()) : 0 ));
	}
}

ShredStorage::~ShredStorage() {
	QHash<LongLat, QByteArray *>::const_iterator i=storage.constBegin();
	for ( ; i!=storage.constEnd(); ++i ) {
		if ( i.value() ) {
			QFile file(Shred::FileName(world->WorldName(),
				i.key().longitude, i.key().latitude));
			if ( file.open(QIODevice::WriteOnly) ) {
				file.write(*i.value());
			}
			delete i.value();
		}
	}
}

void ShredStorage::Shift(const int /* direction */) {}

QByteArray * ShredStorage::GetShredData(const long longi, const long lati)
const {
	const LongLat coords={longi, lati};
	return storage.value(coords);
}

void ShredStorage::SetShredData(QByteArray * const data,
		const long longi, const long lati)
{
	const LongLat coords={longi, lati};
	delete storage.value(coords);
	storage.insert(coords, data);
}
