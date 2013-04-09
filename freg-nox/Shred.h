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
#include "blocks.h"

class QFile;
class World;

class Shred {
	Block * blocks[SHRED_WIDTH][SHRED_WIDTH][HEIGHT];
	uchar lightMap[SHRED_WIDTH][SHRED_WIDTH][HEIGHT];
	World * const world;
	const long longitude, latitude;

	ushort shredX, shredY;
	QList<Active *> activeList;

	public:
	long Longitude() const { return longitude; }
	long Latitude()  const { return latitude; }
	ushort ShredX() const { return shredX; }
	ushort ShredY() const { return shredY; }
	void PhysEvents();

	void AddActive(Active * const);
	bool RemActive(Active * const);

	World * GetWorld() const { return world; }

	void ReloadToNorth();
	void ReloadToEast();
	void ReloadToSouth();
	void ReloadToWest();

	Block * GetBlock(
			const ushort,
			const ushort,
			const ushort) const;
	///Puts block to coordinates xyz and activates it
	void SetBlock(
			Block * const block,
			const ushort x,
			const ushort y,
			const ushort z);
	///Puts block to coordinates and not activates it (e.g. in World::Move)
	void PutBlock(
			Block * const block,
			const ushort x,
			const ushort y,
			const ushort z);
	///Puts normal block to coordinates
	void PutNormalBlock(
			const subs sub,
			const ushort x,
			const ushort y,
			const ushort z);

	uchar LightMap(
			const ushort,
			const ushort,
			const ushort) const;
	bool SetLightMap(
			const uchar level,
			const ushort x,
			const ushort y,
			const ushort z);
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

	int LoadShred(QFile &);

	Shred(
			World * const,
			const ushort shred_x,
			const ushort shred_y,
			const long longi,
			const long lati);
	~Shred();
	
	void SetNewBlock(
			const int kind,
			const int sub,
			const ushort x,
			const ushort y,
			const ushort z);
	private:
	void RegisterBlock(
			Block * const,
			const ushort x,
			const ushort y,
			const ushort z);

	private:
	QString FileName() const;
	char TypeOfShred(
			const long longi,
			const long lati) const;

	void NormalUnderground(const ushort);
	void PlantGrass();
	void TestShred();
	void NullMountain();
	void Plain();
	void Forest(const long, const long);
	void Water( const long, const long);
	void Pyramid();
	void Mountain();
	//block combinations section (trees, buildings, etc):
	bool Tree(
			const ushort x,
			const ushort y,
			const ushort z,
			const ushort height);
}; //class Shred

#endif
