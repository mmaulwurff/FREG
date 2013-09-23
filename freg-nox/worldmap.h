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
	* along with FREG. If not, see <http://www.gnu.org/licenses/>. */

#ifndef WORLDMAP_H
#define WORLDMAP_H

class QFile;
class QString;

class WorldMap {
	public:
	explicit WorldMap(const QString *);
	~WorldMap();

	long MapSize() const;
	char TypeOfShred(long longi, long lati);

	private:
	long mapSize;
	QFile * map;
}; // class WorldMap

#endif // WORLDMAP_H
