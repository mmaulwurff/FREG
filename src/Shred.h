	/*
	*This file is part of FREG.
	*
	*FREG is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*FREG is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#ifndef SHRED_H
#define SHRED_H

#include "world.h"
#include <QList>
#include <QString>


class Shred {
	Block * blocks[shred_width][shred_width][height];
	short lightMap[shred_width][shred_width][height-1];
	QString worldName;
	World * world;
	const long longitude;
	const long latitude;
	const unsigned short shredX;
	const unsigned short shredY;

	public:
	QList<Active *> activeList;
	void AddActive(Active * const);
	void RemActive(Active * const);

	void ReloadToNorth();
	void ReloadToEast();
	void ReloadToSouth();
	void ReloadToWest();

	class Block * Block(const unsigned short,
	              const unsigned short,
	              const unsigned short) const;
	void SetBlock(class Block *,
			const unsigned short,
			const unsigned short,
			const unsigned short);

	short LightMap(const unsigned short,
	               const unsigned short,
	               const unsigned short) const;
	void SetLightMap(const short,
			const unsigned short,
			const unsigned short,
			const unsigned short);
	void PlusLightMap(const short,
			const unsigned short,
			const unsigned short,
			const unsigned short);


	Shred(World * const, QString,
			const unsigned short, const unsigned short,
			const long, const long);
	~Shred();

	private:
	char * FileName(char * const) const;
	char TypeOfShred(const unsigned long, const unsigned long) const;
	int Transparent(
			const unsigned short i,
			const unsigned short j,
			const unsigned short k) {
		return world->TransparentNotSafe(i+shredX*shred_width,
				j+shredY*shred_width, k);
	}
	subs Sub(
			const unsigned short i,
			const unsigned short j,
			const unsigned short k) {
		return world->Sub(i+shredX*shred_width,
			j+shredY*shred_width, k);
	}
	class Block * NewNormal(subs sub) { return world->NewNormal(sub); }

	void NormalUnderground(const unsigned short);
	void PlantGrass();
	void TestShred();
	void NullMountain();
	void Plain();
	void Forest(const long, const long);
	void Water( const long, const long);
	void Hill(  const long, const long);
	//block combinations section (trees, buildings, etc)
	bool Tree(const unsigned short, const unsigned short,
			const unsigned short, const unsigned short);
};

#endif
