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
#include <QByteArray>
#include <QDataStream>
#include "Shred.h"
#include "world.h"

const int datastream_version=QDataStream::Qt_4_6; //Qt version in Debian stable now.

int Shred::LoadShred(QFile & file) {
		QByteArray read_data=file.readAll();
		QByteArray uncompressed=qUncompress(read_data);
		QDataStream in(uncompressed);
		quint8 version;
		in >> version;
		if ( datastream_version!=version ) {
			fprintf(stderr,
				"Wrong version: %d\nGenerating new shred.\n",
				datastream_version);
			return 1;
		}
		in.setVersion(datastream_version);
		for (ushort i=0; i<shred_width; ++i)
		for (ushort j=0; j<shred_width; ++j) {
			for (ushort k=0; k<height; ++k) {
				blocks[i][j][k]=BlockFromFile(in, i, j, k);
				lightMap[i][j][k]=0;
			}
			lightMap[i][j][height-1]=max_light_radius;
		}
		return 0;
}

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
	QFile file(FileName());
	if ( file.open(QIODevice::ReadOnly) && !LoadShred(file) )
		return;

	for (ushort i=0; i<shred_width; ++i)
	for (ushort j=0; j<shred_width; ++j) {
		blocks[i][j][0]=NewNormal(NULLSTONE);
		for (ushort k=1; k<height-1; ++k) {
			blocks[i][j][k]=NewNormal(AIR);
			lightMap[i][j][k]=0;
		}
		blocks[i][j][height-1]=NewNormal( (rand()%5) ? SKY : STAR );
		lightMap[i][j][height-1]=max_light_radius;
	}

	switch ( TypeOfShred(longi, lati) ) {
		case '#': NullMountain(); break;
		case '.': Plain(); break;
		case 't': TestShred(); break;
		case '%': Forest(longi, lati); break;
		case '~': Water( longi, lati); break;
		case '+': Hill(  longi, lati); break;
		case '_': /* empty shred */    break;
		case 'p': Pyramid();           break;
		default:
			Plain();
			fprintf(stderr,
				"Shred::Shred: unknown type of shred: %c\n",
				TypeOfShred(longi, lati));
	}
}

Shred::~Shred() {
	ushort i, j, k;
	const ulong mapSize=world->MapSize();
	if ( (longitude < mapSize) && (latitude < mapSize) ) {
		QFile file(FileName());
		if ( !file.open(QIODevice::WriteOnly) ) {
			fputs("Shred::~Shred: Write Error\n", stderr);
			return;
		}

		QByteArray shred_data;
		shred_data.reserve(200000);
		QDataStream outstr(&shred_data, QIODevice::WriteOnly);
		outstr << (quint8)datastream_version;
		outstr.setVersion(datastream_version);
		for (i=0; i<shred_width; ++i)
		for (j=0; j<shred_width; ++j)
		for (k=0; k<height; ++k) {
			blocks[i][j][k]->SaveToFile(outstr);
			if ( !(blocks[i][j][k]->Normal()) )
				delete blocks[i][j][k];
		}
		file.write(qCompress(shred_data));
		return;
	}

	for (i=0; i<shred_width; ++i)
	for (j=0; j<shred_width; ++j)
	for (k=0; k<height; ++k)
		if ( !(blocks[i][j][k]->Normal()) )
			delete blocks[i][j][k];
}

Block * Shred::CraftBlock(const int kind, const int sub) const {
	switch ( kind ) {
		case BLOCK:  return NewNormal(sub);
		case GRASS:  return new Grass();
		case PICK:   return new Pick(sub);
		case PLATE:  return new Plate(sub);
		case ACTIVE: return new Active(sub);
		case LADDER: return new Ladder(sub);
		case WEAPON: return new Weapon(sub);
		case CHEST:  return new Chest(0, sub);
		case DOOR:   return new Door(0, 0, 0, 0, sub);
		case CLOCK:  return new Clock(GetWorld(), sub);
		case WORKBENCH: return new Workbench(0, sub);
		default:
			fprintf(stderr,
				"Shred::CraftBlock: unlisted kind: %d\n",
				kind);
			return NewNormal(sub);
	}
}

Block * Shred::NewNormal(const int sub) const {
	return world->NewNormal(sub);
}

void Shred::PhysEvents() {
	for (int j=0; j<activeList.size(); ++j) {
		Active * const temp=activeList[j];
		const ushort x=temp->X();
		const ushort y=temp->Y();
		const ushort z=temp->Z();
		const float weight=Weight(x%shred_width, y%shred_width, z);
		if ( temp->ShouldFall() && weight ) {
			temp->SetNotFalling();
			if ( z > 0 &&
					weight > Weight(x%shred_width, y%shred_width, z-1) )
				world->Move(x, y, z, DOWN);
			else if ( z < height-1 &&
					weight < Weight(x%shred_width, y%shred_width, z+1) )
				world->Move(x, y, z, UP);
		}
		temp->Act();
	}
}

Block * Shred::BlockFromFile(
		QDataStream & str,
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
		case BLOCK:  return new Block (str, sub);
		case PICK:   return new Pick  (str, sub);
		case PLATE:  return new Plate (str, sub);
		case LADDER: return new Ladder(str, sub);
		case WEAPON: return new Weapon(str, sub);

		case BUSH:   return new Bush (this, str);
		case CHEST:  return new Chest(this, str, sub);
		case WORKBENCH: return new Workbench(this, str, sub);

		case RABBIT: return new Rabbit(this, i, j, k, str);
		case DWARF:  return new Dwarf (this, i, j, k, str);
		case PILE:   return new Pile  (this, i, j, k, str);
		case GRASS:  return new Grass (this, i, j, k, str);
		case ACTIVE: return new Active(this, i, j, k, str, sub);
		case LIQUID: return new Liquid(this, i, j, k, str, sub);
		case DOOR:   return new Door  (this, i, j, k, str, sub);

		case CLOCK:  return new Clock(str, world, sub);
		default:
			fprintf(stderr,
				"Shred::BlockFromFile: unlisted kind: %d, x: %hu, y: %hu, z: %hu.\n",
				kind, i, j, k);
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

bool Shred::RemActive(Active * const active) {
	return activeList.removeOne(active);
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
	const ulong mapSize=world->MapSize();
	if ( longi >= mapSize || lati  >= mapSize )
		return '#';

	QString temp;
	QFile map(world->WorldName(temp));
	if ( !map.open(QIODevice::ReadOnly | QIODevice::Text) )
		return '.';

	map.seek((mapSize+1)*longi+lati); //+1 is for '\n' in file
	char c;
	return ( map.getChar(&c) ) ? c : '.';
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
	blocks[1][1][height/2]=new Clock(world, IRON);
	blocks[3][1][height/2]=new Chest(this);
	blocks[5][1][height/2]=new Active(this,
		shredX*shred_width+5,
		shredY*shred_width+1, height/2, SAND);
	blocks[7][1][height/2]=NewNormal(GLASS);
	blocks[9][1][height/2]=new Pile(this,
		shredX*shred_width+9,
		shredY*shred_width+1, height/2, NewNormal(STONE));
	blocks[11][1][height/2]=new Plate(STONE);
	blocks[13][1][height/2]=NewNormal(NULLSTONE);

	blocks[1][3][height/2]=new Ladder(NULLSTONE);
	blocks[1][3][height/2+1]=new Ladder(WOOD);
	blocks[3][3][height/2]=new Dwarf(this,
		shredX*shred_width+3,
		shredY*shred_width+3, height/2);
	blocks[3][3][height/2]->Inscribe("Some dwarf");
	blocks[5][3][height/2-3]=new Liquid(this,
		shredX*shred_width+5,
		shredY*shred_width+3, height/2-3);
	blocks[5][3][height/2-2]=new Liquid(this,
		shredX*shred_width+5,
		shredY*shred_width+3, height/2-2);
	blocks[5][3][height/2-1]=NewNormal(AIR);
	blocks[7][3][height/2]=new Bush(this);
	blocks[9][3][height/2-2]=new Rabbit(this,
		shredX*shred_width+9,
		shredY*shred_width+3, height/2-2);
	blocks[9][3][height/2-1]=NewNormal(AIR);
	blocks[11][3][height/2]=new Workbench(this, IRON);
	blocks[13][3][height/2]=new Door(this,
		shredX*shred_width+13,
		shredY*shred_width+3, height/2, GLASS);
	blocks[13][3][height/2]->SetDir(NORTH);

	blocks[1][5][height/2]=new Weapon(IRON);

	for (ushort i=1; i<4; ++i)
	for (ushort j=7; j<10; ++j)
	for (ushort k=height/2; k<height/2+5; ++k)
		blocks[i][j][k]=NewNormal(GLASS);
	/*for (ushort k=height/2; k<height/2+6; ++k)
		blocks[2][8][k]=new Liquid(this,
		shredX*shred_width+2,
		shredY*shred_width+8, k);*/
	blocks[2][8][height/2]=new Rabbit(this,
		shredX*shred_width+2,
		shredY*shred_width+8, height/2);
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

void Shred::Pyramid()
{
	//pyramid by Panzerschrek
	//'p' - pyramid symbol
	NormalUnderground();
	unsigned short z, dz, x, y;

	//пирамида
	for( z= height/2, dz= 0; dz< 8; z +=2, dz++ )
	{
		for( x= dz; x< ( 16 - dz ); x++ )
		{
			blocks[x][dz][z]=
			blocks[x][15 - dz][z]=
			blocks[x][dz][z+1]=
			blocks[x][15 - dz][z+1]=NewNormal(STONE);
		}
		for( y= dz; y< ( 16 - dz ); y++ )
		{
			blocks[dz][y][z]=
			blocks[15 - dz][y][z]=
			blocks[dz][y][z + 1]=
			blocks[15 - dz][y][z + 1]=NewNormal(STONE);
		}
	}

	//вход
	blocks[shred_width/2][0][height/2]= NewNormal( AIR );

	//камера внутри
	for( z= height/2 - 60, dz=0; dz< 8; dz++, z++ )
	for( x= 1; x< shred_width - 1; x++ )
	for( y= 1; y< shred_width - 1; y++ )
		blocks[x][y][z]= NewNormal( AIR );

	//шахта
	for( z= height/2 - 52, dz= 0; dz< 52; z++, dz++ )
		blocks[shred_width/2][shred_width/2][z]= NewNormal( AIR );

	//летающая тарелка
	return;
	for( x=0; x< shred_width; x++ )
	for( y=0; y< shred_width; y++ )
	{
		float r= float( ( x - shred_width/2 ) * ( x - shred_width/2 ) )
		 + float( ( y - shred_width/2 ) * ( y - shred_width/2 ) );
		if( r < 64.0f )
			blocks[x][y][ 124 ]= NewNormal( STONE );
		if( r < 36.0f )
			blocks[x][y][ 125 ]= NewNormal( STONE );
	}
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
		if ( AIR!=Sub(i, j, k) )
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
