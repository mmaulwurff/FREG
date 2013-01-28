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

#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <QBuffer>
#include "Shred.h"
#include "world.h"

const int datastream_version=QDataStream::Qt_4_6; //Qt version in Debian stable now.

Shred::Shred(World * const world_,
		const ushort shred_x,
		const ushort shred_y,
		const ulong longi,
		const ulong lati)
		:
		world(world_),
		longitude(longi),
		latitude(lati),
		shredX(shred_x),
		shredY(shred_y)
{
	ushort i, j, k;
	QFile file(FileName());
	if ( file.open(QIODevice::ReadOnly) ) {
		QByteArray read_data=file.readAll();
		file.close();
		QByteArray uncompressed=qUncompress(read_data);
		QBuffer buf(&uncompressed);
		buf.open(QIODevice::ReadOnly);
		QDataStream in(&buf);
		quint8 version;
		in >> version;
		if ( datastream_version!=version )
			fprintf(stderr,
				"Wrong version: %d\nBut trying to read.\n",
				datastream_version);
		in.setVersion(datastream_version);
		for (i=0; i<shred_width; ++i)
		for (j=0; j<shred_width; ++j) {
			for (k=0; k<height; ++k)
				blocks[i][j][k]=BlockFromFile(in, i, j, k);
			lightMap[i][j][height-1]=max_light_radius;
		}
		buf.close();
		return;
	}
	
	for (i=0; i<shred_width; ++i)
	for (j=0; j<shred_width; ++j) {
		blocks[i][j][0]=NewNormal(NULLSTONE);
		for (k=1; k<height-1; ++k)
			blocks[i][j][k]=NewNormal(AIR);
		blocks[i][j][height-1]=NewNormal( rand()%5 ? SKY : STAR );
		lightMap[i][j][height-1]=max_light_radius;
	}
			
	switch ( TypeOfShred(longi, lati) ) {
		case '#': NullMountain(); break;
		case '.': Plain(); break;
		case 't': TestShred(); break;
		case '%': Forest(longi, lati); break;
		case '~': Water( longi, lati); break;
		case '+': Hill(  longi, lati); break;
		default:
			Plain();
			fprintf(stderr,
				"Shred::Shred: unknown type of shred: %c\n",
				TypeOfShred(longi, lati));
	}
}

Shred::~Shred() {
	const ulong mapSize=world->MapSize();
	if ( (longitude < mapSize) && (latitude < mapSize) ) {
		QFile file(FileName());
		if ( !file.open(QIODevice::WriteOnly) ) {
			fputs("Shred::~Shred: Write Error\n", stderr);
			return;
		}

		QByteArray shred_data;
		shred_data.reserve(200000);
		QBuffer buf(&shred_data);
		buf.open(QIODevice::WriteOnly);
		QDataStream outstr(&buf);
		outstr << (quint8)datastream_version;
		outstr.setVersion(datastream_version);
		ushort i, j, k;
		for (i=0; i<shred_width; ++i)
		for (j=0; j<shred_width; ++j)
		for (k=0; k<height; ++k) {
			blocks[i][j][k]->SaveToFile(outstr);
			if ( !(blocks[i][j][k]->Normal()) )
				delete blocks[i][j][k];
		}
		file.write(qCompress(shred_data));
		buf.close();
		file.close();
		return;
	}

	ushort i, j, k;
	for (i=0; i<shred_width; ++i)
	for (j=0; j<shred_width; ++j)
	for (k=0; k<height; ++k)
		if ( !(blocks[i][j][k]->Normal()) )
			delete blocks[i][j][k];
}

Block * Shred::NewNormal(const int sub) const {
	return world->NewNormal(sub);
}

void Shred::PhysEvents() {
	for (short j=0; j<activeList.size(); ++j) {
		Active * temp=activeList[j];
		temp->Act();
		const ushort x=temp->X();
		const ushort y=temp->Y();
		const ushort z=temp->Z();

		if ( temp->ShouldFall() )
			world->Move(x, y, z, DOWN);
	}
}

Block * Shred::BlockFromFile(QDataStream & str,
		ushort i,
		ushort j,
		const ushort k) 
{
	quint16 kind, sub;
	bool normal;
	str >> kind >> sub >> normal;
	if ( normal ) {
		return NewNormal(sub);
	}
	
	i+=shredX*shred_width;
	j+=shredY*shred_width;

	//if some kind will not be listed here,
	//blocks of this kind just will not load,
	//unless kind is inherited from Inventory class or one
	//of its derivatives - in this case this may cause something bad.
	switch ( kind ) {
		case BLOCK:  return new Block(str, sub);
		case PICK:   return new Pick (str, sub);
		case CHEST:  return new Chest (this, str, sub);
		case BUSH:   return new Bush  (this, str, sub);
		case RABBIT: return new Rabbit(this, i, j, k, str, sub);
		case ACTIVE: return new Active(this, i, j, k, str, sub);
		case DWARF:  return new Dwarf (this, i, j, k, str, sub);
		case PILE:   return new Pile  (this, i, j, k, str, sub);
		case LIQUID: return new Liquid(this, i, j, k, str, sub);
		case GRASS:  return new Grass (this, i, j, k, str, sub);
		default:
			fprintf(stderr,
				"BlockFromFile(): unlisted kind: %d\n",
				kind);
			return NewNormal(sub);
	}
}

int Shred::Sub(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Sub();
}
int Shred::Kind(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Kind();
}
int Shred::Durability(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Durability();
}
int Shred::Movable(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Movable();
}
int Shred::Transparent(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Transparent();
}
float Shred::Weight(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Weight();
}

void Shred::AddActive(Active * const active) {
	activeList.append(active);
}

void Shred::RemActive(Active * const active) {
	activeList.removeOne(active);
}

void Shred::ReloadToNorth() {
	for (ushort i=0; i<activeList.size(); ++i)
		activeList[i]->ReloadToNorth();
	++shredY;
}
void Shred::ReloadToEast() {
	for (ushort i=0; i<activeList.size(); ++i)
		activeList[i]->ReloadToEast();
	--shredX;
}
void Shred::ReloadToSouth() {
	for (ushort i=0; i<activeList.size(); ++i)
		activeList[i]->ReloadToSouth();
	--shredY;
}
void Shred::ReloadToWest() {
	for (ushort i=0; i<activeList.size(); ++i)
		activeList[i]->ReloadToWest();
	++shredX;
}

Block * Shred::GetBlock(
		const ushort x,
		const ushort y,
		const ushort z) const {
	//fprintf(stderr, "Shred::GetBlock::x: %hu, y: %hu, z:%hu\n", x, y, z);
	return blocks[x][y][z];
}
void Shred::SetBlock(Block * block,
		const ushort x,
		const ushort y,
		const ushort z)
{
	blocks[x][y][z]=block;
}

QString Shred::FileName() const {
	QString str;
	world->WorldName(str);
	return str=str+"_shreds/"+
		QString::number(longitude)+'_'+
		QString::number(latitude);
}

char Shred::TypeOfShred(
		const ulong longi,
		const ulong lati) const
{
	//return '.';
	const ulong mapSize=world->MapSize();
	if ( longi >= mapSize || lati  >= mapSize )
		return '#';

	QString temp;
	QFile map(world->WorldName(temp));
	if ( !map.open(QIODevice::ReadOnly | QIODevice::Text) )
		return '.';

	QTextStream in_map(&map);
	in_map.seek((mapSize+1)*longi+lati); //+1 is for '\n' in file
	char c;
	in_map >> &c;
	return c;
}

//shred generators section
//these functions fill space between the lowest nullstone layer and sky. so use k from 1 to heigth-2.
void Shred::NormalUnderground(const ushort depth=0) {
	for (ushort i=0; i<shred_width; ++i)
	for (ushort j=0; j<shred_width; ++j) {
		ushort k;
		for (k=1; k<height/2-6 && k<height/2-depth-1; ++k)
			blocks[i][j][k]=NewNormal(STONE);
		blocks[i][j][k]=NewNormal((rand()%2) ? STONE : SOIL);
		for (++k; k<height/2-depth; ++k)
			blocks[i][j][k]=NewNormal(SOIL);
	}
}

void Shred::PlantGrass() {
	for (ushort i=0; i<shred_width; ++i)	
	for (ushort j=0; j<shred_width; ++j) {
		ushort k;
		for (k=height-2; Transparent(i, j, k); --k);
		if ( SOIL==Sub(i, j, k++) && AIR==Sub(i, j, k) )
			blocks[i][j][k]=new Grass(this,
				i+shredX*shred_width,
				j+shredY*shred_width, k);
	}
}

void Shred::TestShred() {
	NormalUnderground();
	//blocks[2][0][height/2]=new Chest(this);
	/*blocks[3][1][height/2]=new Active(this,
		shredX*shred_width+3,
		shredY*shred_width+1, height/2, SAND);*/
}

void Shred::NullMountain() {
	ushort i, j, k;
	for (i=0; i<shred_width; ++i)
	for (j=0; j<shred_width; ++j) {
		for (k=1; k<height/2; ++k)
			blocks[i][j][k]=NewNormal( (i==4 ||
			                            i==5 ||
			                            j==4 ||
			                            j==+5) ?
					NULLSTONE : STONE );

		for ( ; k<height-1; ++k)
			if (i==4 || i==5 || j==4 || j==5)
				blocks[i][j][k]=NewNormal(NULLSTONE);
	}
}

void Shred::Plain() {
	NormalUnderground();
	ushort i, num, x, y;

	//bush
	num=rand()%4;
	for (i=0; i<=num; ++i) {
		x=rand()%shred_width;
		y=rand()%shred_width;
		if ( AIR==Sub(x, y, height/2) )
			blocks[x][y][height/2]=new Bush(this);
	}

	//rabbits
	num=rand()%4;
	for (i=0; i<=num; ++i) {
		x=rand()%shred_width;
		y=rand()%shred_width;
		if ( AIR==Sub(x, y, height/2) )
			blocks[x][y][height/2]=new Rabbit(this,
				shredX*shred_width+x,
				shredY*shred_width+y, height/2);
	}

	PlantGrass();
}

void Shred::Forest(const long longi, const long lati) {
	NormalUnderground();

	long i, j;
	ushort number_of_trees=0;
	for (i=longi-1; i<=longi+1; ++i)
	for (j=lati-1;  j<=lati+1;  ++j)
		if ( '%'==TypeOfShred(i, j) )
			++number_of_trees;

	for (i=0; i<number_of_trees; ++i) {
		short x=rand()%(shred_width-2),
		      y=rand()%(shred_width-2);
		Tree(x, y, height/2, 4+rand()%5);
	}

	PlantGrass();
}

void Shred::Water(const long longi, const long lati) {
	ushort depth=1;
	char map[3][3];
	for (long i=longi-1; i<=longi+1; ++i)
	for (long j=lati-1;  j<=lati+1;  ++j)
		if ( '~'==(map[i-longi+1][j-lati+1]=TypeOfShred(i, j)) )
			++depth;

	NormalUnderground(depth);
	ushort i, j, k;

	if ('~'!=map[1][0] && '~'!=map[0][1]) { //north-west rounding
		for (i=0; i<shred_width/2; ++i)	
		for (j=0; j<shred_width/2; ++j)
			for (k=height/2-depth; k<height/2; ++k)
				if ( ((7-i)*(7-i) +
				      (7-j)*(7-j) +
				      (height/2-k)*(height/2-k)*16/depth/depth)
						> 49 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	if ('~'!=map[1][0] && '~'!=map[2][1]) { //south-west rounding
		for (i=0; i<shred_width/2; ++i)	
		for (j=shred_width/2; j<shred_width; ++j)
			for (k=height/2-depth; k<height/2; ++k)
				if ( ((7-i)*(7-i)+
				      (8-j)*(8-j)+
				      (height/2-k)*(height/2-k)*16/depth/depth)
						> 49 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	if ('~'!=map[2][1] && '~'!=map[1][2]) { //south-east rounding
		for (i=shred_width/2; i<shred_width; ++i)
		for (j=shred_width/2; j<shred_width; ++j)
			for (k=height/2-depth; k<height/2; ++k)
				if ( ((8-i)*(8-i)+
				      (8-j)*(8-j)+
				      (height/2-k)*(height/2-k)*16/depth/depth)
						> 49 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	if ('~'!=map[1][2] && '~'!=map[0][1]) { //north-east rounding
		for (i=shred_width/2; i<shred_width; ++i)
		for (j=0; j<shred_width/2; ++j)
			for (k=height/2-depth; k<height/2; ++k)
				if ( ((8-i)*(8-i)+
				      (7-j)*(7-j)+
				      (height/2-k)*(height/2-k)*16/depth/depth)
						> 49 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	for (i=0; i<shred_width; ++i)	
	for (j=0; j<shred_width; ++j)
	for (k=height/2-depth; k<height/2; ++k)
		if ( AIR==Sub(i, j, k) )
			blocks[i][j][k]=new Liquid(this,
			                i+shred_width*shredX,
			                j+shred_width*shredY, k);

	PlantGrass();
}

void Shred::Hill(const long longi, const long lati) {
	ushort hill_height=1;
	for (long i=longi-1; i<=longi+1; ++i)
	for (long j=lati-1;  j<=lati+1;  ++j)
		if ( '+'==TypeOfShred(i, j) )
			++hill_height;

	NormalUnderground();

	for (ushort i=0; i<shred_width; ++i)	
	for (ushort j=0; j<shred_width; ++j)
	for (ushort k=height/2; k<height/2+hill_height; ++k)
		if (((4.5-i)*(4.5-i)+
		     (4.5-j)*(4.5-j)+
		     (height/2-0.5-k)*(height/2-0.5-k)*16/hill_height/hill_height)<=16)
			blocks[i][j][k]=NewNormal(SOIL);
	
	PlantGrass();
}

bool Shred::Tree(
		const ushort x,
		const ushort y,
		const ushort z,
		const ushort height)
{
	if ( shred_width<=x+2 ||
			shred_width<=y+2 ||
			::height-1<=z+height ||
			height<2 )
		return false;

	ushort i, j, k;
	for (i=x; i<=x+2; ++i)
	for (j=y; j<=y+2; ++j)
	for (k=z; k<z+height; ++k)
		if ( AIR==Sub(i, j, k) )
			return false;

	for (k=z; k<z+height-1; ++k) //trunk
		blocks[x+1][y+1][k]=NewNormal(WOOD);

	for (i=x; i<=x+2; ++i) //leaves
	for (j=y; j<=y+2; ++j)
	for (k=z+height/2; k<z+height; ++k)
		if ( AIR==Sub(i, j, k) )
			blocks[i][j][k]=NewNormal(GREENERY);
	
	return true;
}
