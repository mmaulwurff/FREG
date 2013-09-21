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

#include <QFile>
#include <QString>
#include <qmath.h>
#include <QProcess>
#include <QStringList>
#include "worldmap.h"
#include "header.h"

WorldMap::WorldMap(const QString * const world_name) {
	map = new QFile(*world_name+"/map.txt");
	if ( map->open(QIODevice::ReadOnly | QIODevice::Text) ) {
		mapSize = int(qSqrt(1+4*map->size())-1)/2;
	} else {
		#ifdef Q_OS_LINUX
		QString program = "./mapgen";
		#else
		QString program = "mapgen.exe";
		#endif
		const ushort map_size = 75;
		QStringList arguments;
		arguments << QString("-s") << QString::number(map_size) <<
			QString("-r") << QString::number(qrand()) <<
			QString("-f") << *world_name+"/map.txt";
		QProcess map_generation;
		map_generation.start(program, arguments);
		if ( map_generation.waitForStarted() ) {
			fprintf(stderr, "hello map\n");
			map_generation.waitForFinished();
		}
		mapSize=( map->open(QIODevice::ReadOnly | QIODevice::Text) ) ?
			map_size : 1;
	}
}

char WorldMap::TypeOfShred(const long longi, const long lati) {
	if (
			longi >= mapSize || longi < 0 ||
			lati  >= mapSize || lati  < 0 )
	{
		return OUT_BORDER_SHRED;
	} else if ( !map->seek((mapSize+1)*longi+lati) ) {
		return DEFAULT_SHRED;
	}
	char c;
	map->getChar(&c);
	return c;
}

long WorldMap::MapSize() const { return mapSize; }

WorldMap::~WorldMap() { delete map; }
