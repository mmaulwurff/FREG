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

#ifndef SHRED_H
#define SHRED_H

#include <QLinkedList>
#include "header.h"

class QFile;
class World;
class Block;
class Active;

class Shred {
	public:
	Shred(World *, ushort shred_x, ushort shred_y,
			long longi, long lati,
			Shred * memory);
	~Shred();

	private:
	Block * blocks[SHRED_WIDTH][SHRED_WIDTH][HEIGHT];
	uchar lightMap[SHRED_WIDTH][SHRED_WIDTH][HEIGHT];
	World * const world;
	const long longitude, latitude;
	ushort shredX, shredY;

	// needed in Shred::ReloadTo... for active blocks not to reload twice
	// when they are registered both in frequent and rare lists.
	QLinkedList<Active *> activeListAll;
	QLinkedList<Active *> activeListFrequent;
	QLinkedList<Active *> activeListRare;
	QLinkedList<Active *> fallList;
	QLinkedList<Active *> shiningList;
	QLinkedList<Active *> deleteList;

	// memory, allocated for this shred.
	Shred * const memory;

	/// Lowest nullstone and sky are not in bounds.
	bool InBounds(ushort x, ushort y, ushort z) const;

	public:
	/// Returns y (line) shred coordinate on world map.
	long Longitude() const;
	/// Returns x (column) shred coordinate on world map.
	long Latitude()  const;
	ushort ShredX() const;
	ushort ShredY() const;
	void PhysEventsFrequent();
	void PhysEventsRare();

	void AddActive(Active *);
	void RemActive(Active *);
	void AddFalling(Active *);
	void RemFalling(Active *);
	void AddFalling(ushort x, ushort y, ushort z);
	void AddShining(Active *);
	void RemShining(Active *);
	void AddToDelete(Active *);

	QLinkedList<Active *>::const_iterator ShiningBegin() const;
	QLinkedList<Active *>::const_iterator ShiningEnd() const;

	World * GetWorld() const;

	void ReloadToNorth();
	void ReloadToEast();
	void ReloadToSouth();
	void ReloadToWest();

	Block * GetBlock(ushort x, ushort y, ushort z) const;
	/// Puts block to coordinates xyz and activates it
	void SetBlock(Block * block, ushort x, ushort y, ushort z);
	/// Puts block to coordinates, not activates it (e.g. in World::Move)
	void PutBlock(Block * block, ushort x, ushort y, ushort z);
	/// Puts normal block to coordinates
	void PutNormalBlock(int sub, ushort x, ushort y, ushort z);
	static Block * Normal(int sub);

	public: // Lighting section
	uchar Lightmap(  short x, short y, short z) const;
	uchar FireLight( short x, short y, short z) const;
	uchar SunLight(  short x, short y, short z) const;
	uchar LightLevel(short x, short y, short z) const;

	bool SetSunLight( short x, short y, short z, uchar level);
	bool SetFireLight(short x, short y, short z, uchar level);
	void SetLightmap( short x, short y, short z, uchar level);

	void SetAllLightMapNull();
	void ShineAll();

	private:
	void RemoveAllSunLight();
	void RemoveAllFireLight();
	void RemoveAllLight();

	public: // Information section
	int Sub(ushort x, ushort y, ushort z) const;

	bool LoadShred(QFile &);

	void SetNewBlock(int kind, int sub, ushort x, ushort y, ushort z,
			int dir=UP);
	char TypeOfShred(long longi, long lati) const;
	Shred * GetShredMemory() const;
	private:
	void RegisterBlock(Block *, ushort x, ushort y, ushort z);

	QString FileName() const;

	void NormalUnderground(ushort depth=0, int sub=SOIL);
	void CoverWith(int kind, int sub);
	/// Puts num things(kind-sub) in random places on shred surface.
	/** If on_water is false, this will not drop things on water,
	 *  otherwise on water too.
	 */
	void RandomDrop(ushort num, int kind, int sub, bool on_water=false);

	void PlantGrass();
	void TestShred();
	void NullMountain();
	void Plain();
	void Forest();
	void Water();
	void Pyramid();
	void Mountain();
	void Hill();
	void Desert();
	/// For testing purposes.
	void ChaosShred();

	// Block combinations section (trees, buildings, etc):
	bool Tree(ushort x, ushort y, ushort z, ushort height);
	private:
	void NormalCube(ushort x_start, ushort y_start, ushort z_start,
			ushort x_end, ushort y_end, ushort z_end, int sub);

	private: // Land generation
	void ShredLandAmplitudeAndLevel(long longi, long lati,
			ushort * l, float * a) const;
	void ShredNominalAmplitudeAndLevel(char shred_type,
			ushort* l, float* a) const;
	void AddWater();
	ushort FlatUndeground(short depth=0);
};//class Shred

#endif
