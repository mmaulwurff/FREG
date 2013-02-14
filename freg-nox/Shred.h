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

#include <QList>
#include <QString>
#include <QDataStream>
#include "header.h"
#include "blocks.h"

class World;

class Shred {
	Block * blocks[shred_width][shred_width][height];
	uchar lightMap[shred_width][shred_width][height];
	World * const world;
	const ulong longitude, latitude;

	ushort shredX, shredY;
	QList<Active *> activeList;

	public:
	ulong Longitude() const { return longitude; }
	ulong Latitude()  const { return latitude; }
	ushort ShredX() const { return shredX; }
	ushort ShredY() const { return shredY; }
	void PhysEvents();

	void AddActive(Active * const);
	void RemActive(Active * const);

	World * GetWorld() const { return world; }

	void ReloadToNorth();
	void ReloadToEast();
	void ReloadToSouth();
	void ReloadToWest();

	Block * GetBlock(
			const ushort,
			const ushort,
			const ushort) const;
	void SetBlock(Block *,
			const ushort,
			const ushort,
			const ushort);

	uchar LightMap(
			const ushort,
			const ushort,
			const ushort) const;
	bool SetLightMap(
			const uchar level,
			const ushort,
			const ushort,
			const ushort);
	void SetAllLightMap(const uchar=0);
	void ShineAll();

	int Sub(
			const ushort,
			const ushort,
			const ushort) const;
	int Kind(
			const ushort,
			const ushort,
			const ushort) const;
	int Durability(
			const ushort,
			const ushort,
			const ushort) const;
	int Movable(
			const ushort,
			const ushort,
			const ushort) const;
	int Transparent(
			const ushort,
			const ushort,
			const ushort) const;
	float Weight(
			const ushort,
			const ushort,
			const ushort) const;
	uchar LightRadius(
			const ushort,
			const ushort,
			const ushort) const;


	Shred(World * const,
			const ushort,
			const ushort,
			const ulong,
			const ulong);
	~Shred();
	
	Block * CraftBlock(const int kind, const int sub) {
		switch ( kind ) {
			case BLOCK: return NewNormal(sub);
			case PICK:  return new Pick(sub);
			case CHEST: return new Chest(0, sub);
			case CLOCK: return new Clock(GetWorld(), sub);
			default:
				fprintf(stderr,
					"Shred::CraftBlock: unlisted kind: %d\n",
					kind);
				return NewNormal(sub);
		}
	}
	Block * NewNormal(const int sub) const;
	Block * BlockFromFile(QDataStream &,
			ushort,
			ushort,
			const ushort);

	private:
	QString FileName() const;
	char TypeOfShred(
			const ulong,
			const ulong) const;

	void NormalUnderground(const ushort);
	void PlantGrass();
	void TestShred();
	void NullMountain();
	void Plain();
	void Forest(const long, const long);
	void Water( const long, const long);
	void Hill(  const long, const long);
	//block combinations section (trees, buildings, etc):
	bool Tree(
			const ushort x,
			const ushort y,
			const ushort z,
			const ushort height);
};

#endif
