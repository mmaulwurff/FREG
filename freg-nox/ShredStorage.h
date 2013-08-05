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

#ifndef SHRED_STORAGE_H
#define SHRED_STORAGE_H

#include <QHash>
class QByteArray;
class World;

struct LongLat {
	long longitude;
	long latitude;

	bool operator==(const LongLat &) const;
};

class ShredStorage {
	public:
	ShredStorage(const World *, ushort size,
			long longi_center, long lati_center);
	~ShredStorage();

	QByteArray * GetShredData(long longi, long lati) const;
	void SetShredData(QByteArray *, long longi, long lati);
	void Shift(int direction);

	private:
	QHash<LongLat, QByteArray *> storage;
	const ushort size;
	const World * const world;
}; // class ShredStorage

#endif
