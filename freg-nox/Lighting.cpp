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

//This file provides simple (also mad) lighting for freg.

#include "world.h"
#include "Shred.h"

//private
uchar World::LightRadius(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return GetShred(x, y)->
		LightRadius(x%shred_width, y%shred_width, z);
}

//private. use Enlightened instead, which is smart wrapper of this.
uchar World::LightMap(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return GetShred(x, y)->
		LightMap(x%shred_width, y%shred_width, z);
}

//private
bool World::SetLightMap(
		const ushort level,
		const ushort x,
		const ushort y,
		const ushort z)
{
	return GetShred(x, y)->
		SetLightMap(level, x%shred_width, y%shred_width, z);
}

//private. make block emit shining
void World::Shine(
		const ushort i,
		const ushort j,
		const ushort k,
		const ushort level,
		const bool init) //see default in class
{
	if ( !InBounds(i, j, k) || 0==level )
		return;

	const bool set=SetLightMap(level, i, j, k);
	if ( !Transparent(i, j, k) ) {
		if ( set )
			emit Updated(i, j, k);
		if ( !init )
			return;
	}

	Shine(i-1, j, k, level-1);
	Shine(i+1, j, k, level-1);
	Shine(i, j-1, k, level-1);
	Shine(i, j+1, k, level-1);
	Shine(i, j, k-1, level-1);
	Shine(i, j, k+1, level-1);
}

//private
void World::SunShine(
		const ushort i,
		const ushort j)
{
	ushort light_lev=max_light_radius;
	ushort k=height-2;
	ushort transparent;
	do {
		transparent=Transparent(i, j, k);
		if ( SetLightMap(light_lev, i, j, k) &&
				2!=transparent )
			emit Updated(i, j, k);
		if ( 1==transparent )
			--light_lev;
		--k;
	} while ( light_lev && transparent );
}

//private. called when onet block is moved, built, or destroyed.
void World::ReEnlighten(
		const ushort i,
		const ushort j,
		const ushort k)
{
	if ( NIGHT!=PartOfDay() )
		SunShine(i, j);
	Shine(i, j, k, LightRadius(i, j, k), true);
	emit Updated(i, j, k);
}

//private. called when world is created and when is it time to
//change world lighting (sun rises and falls)
void World::ReEnlightenTime() {
	for (ushort i=0; i<NumShreds()*NumShreds(); ++i)
		shreds[i]->SetAllLightMap();
	ReEnlightenAll();
}

//private. called from ReEnlightenTime and from World::ReloadShreds
void World::ReEnlightenAll() {
	disconnect(this, SIGNAL(Updated(
		const ushort,
		const ushort,
		const ushort)), 0, 0);
	disconnect(this, SIGNAL(UpdatedAround(
		const ushort,
		const ushort,
		const ushort,
		const ushort)), 0, 0);
	
	for (ushort i=0; i<NumShreds()*NumShreds(); ++i)
		shreds[i]->ShineAll();

	if ( NIGHT!=PartOfDay() )
		for (ushort i=0; i<shred_width*numShreds; ++i)
		for (ushort j=0; j<shred_width*numShreds; ++j)
			SunShine(i, j);
	emit UpdatedAll();
	emit ReConnect();
}

uchar World::Enlightened(
		const ushort i,
		const ushort j,
		const ushort k) const
{
	return InBounds(i, j, k) ?
		LightMap(i, j, k) :
		0;
}

//returns ligting of the block.
//if block side lighting is required, just remove first return.
uchar World::Enlightened(
		const ushort i,
		const ushort j,
		const ushort k,
		const int dir) const
{
	return Enlightened(i, j, k);
	//next provides lighting of block side, not all block
	ushort x, y, z;
	Focus(i, j, k, x, y, z, dir);
	return qMin(Enlightened(i, j, k),
		Enlightened(x, y, z));
}

uchar World::SunLight(
		const ushort i,
		const ushort j,
		const ushort k) const
{
	return Enlightened(i, j, k);
}

uchar World::FireLight(
		const ushort i,
		const ushort j,
		const ushort k) const
{
	return Enlightened(i, j, k);
}

uchar Shred::LightRadius(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->LightRadius();
}

uchar Shred::LightMap(
		const ushort i,
		const ushort j,
		const ushort k) const
{
	return lightMap[i][j][k];
}

bool Shred::SetLightMap(
		const uchar level,
		const ushort i,
		const ushort j,
		const ushort k)
{
	if ( lightMap[i][j][k]>=level )
		return false;
	lightMap[i][j][k]=level;
	return true;
}

//set lightmap of all shred to level. default level is 0.
void Shred::SetAllLightMap(const uchar level) {
	for (ushort i=0; i<shred_width; ++i)
	for (ushort j=0; j<shred_width; ++j)
	for (ushort k=0; k<height-1; ++k)
		lightMap[i][j][k]=level;
}

//make all shining blocks of shred shine.
void Shred::ShineAll() {
	for (short j=0; j<activeList.size(); ++j) {
		Active const * const temp=activeList[j];
		if ( temp )
			world->Shine(temp->X(), temp->Y(), temp->Z(),
				temp->LightRadius(), true);
	}	
}
