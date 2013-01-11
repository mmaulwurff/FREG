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
#include <blocks.h>

class World;

class Shred {
	Block * blocks[shred_width][shred_width][height];
	const Block air;
	short lightMap[shred_width][shred_width][height-1];
	QString worldName;
	World * world;
	const ulong longitude, latitude;

	unsigned short shredX;
	unsigned short shredY;
	QList<Active *> activeList;

	public:
	void PhysEvents();

	void AddActive(Active * const);
	void RemActive(Active * const);

	World * GetWorld() const { return world; }

	void ReloadToNorth();
	void ReloadToEast();
	void ReloadToSouth();
	void ReloadToWest();

	Block * GetBlock(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;
	void SetBlock(Block *,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &);

	short LightMap(const unsigned short &,
	               const unsigned short &,
	               const unsigned short &) const;
	void SetLightMap(const short &,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &);
	void PlusLightMap(const short &,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &);

	int Sub(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	int Kind(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	int Durability(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	int Movable(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	int Transparent(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	double Weight(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;


	Shred(World * const,
			QString,
			const unsigned short &,
			const unsigned short &,
			const unsigned long &,
			const unsigned long &);
	~Shred();

	Block * BlockFromFile(FILE * const,
			unsigned short,
			unsigned short,
			const unsigned short);

	private:
	char * FileName(char * const) const;
	char TypeOfShred(
			const unsigned long,
			const unsigned long);
	Block * NewNormal(const subs & sub) const;

	void NormalUnderground(const unsigned short);
	void PlantGrass();
	void TestShred();
	void NullMountain();
	void Plain();
	void Forest(const long &, const long &);
	void Water( const long &, const long &);
	void Hill(  const long &, const long &);
	//block combinations section (trees, buildings, etc)
	bool Tree(
			const unsigned short & x,
			const unsigned short & y,
			const unsigned short & z,
			const unsigned short & height);
};

#endif
