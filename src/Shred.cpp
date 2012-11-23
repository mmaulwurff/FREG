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

#include "Shred.h"
#include "world.h"
#include "blocks.h"

Shred::Shred(World * const world_,
		QString world_name,
		const unsigned short shred_x,
		const unsigned short shred_y,
		const long longi,
		const long lati)
		:
		worldName(world_name),
		world(world_),
		longitude(longi),
		latitude(lati),
		shredX(shred_x),
		shredY(shred_y)
		//activeList()
{
	char str[50];
	FILE * in=fopen(FileName(str), "r");

	unsigned short i, j, k;

	if ( NULL!=in ) {
		for (i=0; i<shred_width; ++i)
		for (j=0; j<shred_width; ++j)
			for (k=0; k<height-1; ++k)
				blocks[i][j][k]=world->BlockFromFile(in,
					i+shred_width*shred_x,
					j+shred_width*shred_y, k);
		fclose(in);
		return;
	}

	for (i=0; i<shred_width; ++i)
	for (j=0; j<shred_width; ++j) {
		blocks[i][j][0]=NewNormal(NULLSTONE);
		for (k=1; k<height-1; ++k)
			blocks[i][j][k]=NULL;
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
				"Shred::Shred: unknown type of shred: %c",
				TypeOfShred(longi, lati));
	}
}

Shred::~Shred() {
	char str[50];
	FILE * out=fopen(FileName(str), "w");

	unsigned short i, j, k;

	if ( NULL!=out ) {
		for (i=0; i<shred_width; ++i)
		for (j=0; j<shred_width; ++j)
		for (k=0; k<height-1; ++k)
			if (NULL!=blocks[i][j][k]) {
				blocks[i][j][k]->SaveToFile(out);
				if ( !(blocks[i][j][k]->Normal()) )
					delete blocks[i][j][k];
			} else
				fprintf(out, "-1\n");
		fclose(out);
		return;
	}

	for (i=0; i<shred_width; ++i)
	for (j=0; j<shred_width; ++j)
	for (k=0; k<height-1; ++k)
		if ( NULL!=blocks[i][j][k] && !(blocks[i][j][k]->Normal()) )
			delete blocks[i][j][k];
}

int Shred::Transparent(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
{
	return ( NULL==blocks[i][j][k] ) ? 2 : blocks[i][j][k]->Transparent();
}

subs Shred::Sub(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
	{ return ( NULL==blocks[i][j][k] ) ? AIR : blocks[i][j][k]->Sub(); }

kinds Shred::Kind(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
	{ return ( NULL==blocks[i][j][k] ) ? BLOCK : blocks[i][j][k]->Kind(); }

void Shred::AddActive(const Active * const active) {
	activeList.append(*active);
}

void Shred::RemActive(const Active * const active) {
	activeList.removeOne(*active);
}

void Shred::ReloadToNorth() {
	for (unsigned short i=0; i<activeList.size(); ++i)
		activeList[i].ReloadToNorth();
}
void Shred::ReloadToEast() {
	for (unsigned short i=0; i<activeList.size(); ++i)
		activeList[i].ReloadToEast();
}
void Shred::ReloadToSouth() {
	for (unsigned short i=0; i<activeList.size(); ++i)
		activeList[i].ReloadToSouth();
}
void Shred::ReloadToWest() {
	for (unsigned short i=0; i<activeList.size(); ++i)
		activeList[i].ReloadToWest();
}

Block * Shred::GetBlock(
		const unsigned short x,
		const unsigned short y,
		const unsigned short z) const {
	//fprintf(stderr, "Shred::GetBlock::x: %hu, y: %hu, z:%hu\n", x, y, z);
	return blocks[x][y][z];
}
void Shred::SetBlock(Block * block,
		const unsigned short x,
		const unsigned short y,
		const unsigned short z)
{
	blocks[x%shred_width][y%shred_width][z]=block;
}

short Shred::LightMap(const unsigned short i,
                      const unsigned short j,
                      const unsigned short k) const
	{ return lightMap[i][j][k]; }
void Shred::SetLightMap(const short level,
		const unsigned short i,
		const unsigned short j,
		const unsigned short k)
	{ lightMap[i][j][k]=level; }
void Shred::PlusLightMap(const short level,
		const unsigned short i,
		const unsigned short j,
		const unsigned short k)
{
	lightMap[i][j][k]+=level;
	if ( lightMap[i][j][k]>10 )
		lightMap[i][j][k]=10;
}

char * Shred::FileName(char * const str) const {
	sprintf(str, "%s_shreds/%ld_%ld", worldName.toAscii().constData(),
		longitude, latitude);
	return str;
}

char Shred::TypeOfShred(
		const unsigned long longi,
		const unsigned long lati) const {
	FILE * map=fopen(worldName.toAscii().constData(), "r");
	if ( NULL==map )
		return '.';

	unsigned long mapSize;
	while ( '#'==getc(map) ) //'#' is for comment
		while ( '\n'!=getc(map) );

	if ( !fscanf(map, "ize:%lu\n", &mapSize) ) {
		fprintf(stderr, "Shred::TypoOfShred:Map read error.\n");
		return '#';
	}

	if ( longi > mapSize || lati  > mapSize )
		return '#';

	fseek(map, (mapSize+1)*longi+lati, SEEK_CUR); //+1 is for '\n' in file
	char c=fgetc(map);
	fclose(map);
	return c;
}

//shred generators section
//these functions fill space between the lowest nullstone layer and sky. so use k from 1 to heigth-2.
//unfilled blocks are air.
void Shred::NormalUnderground(const unsigned short depth=0) {
	for (unsigned short i=0; i<shred_width; ++i)
	for (unsigned short j=0; j<shred_width; ++j) {
		unsigned short k;
		for (k=1; k<height/2-6 && k<height/2-depth-1; ++k)
			blocks[i][j][k]=NewNormal(STONE);
		blocks[i][j][++k]=NewNormal((rand()%2) ? STONE : SOIL);
		for (++k; k<height/2-depth; ++k)
			blocks[i][j][k]=NewNormal(SOIL);
	}
}

void Shred::PlantGrass() {
	for (unsigned short i=0; i<shred_width; ++i)	
	for (unsigned short j=0; j<shred_width; ++j) {
		unsigned short k;
		for (k=height-2; Transparent(i, j, k); --k);
		if ( SOIL==Sub(i, j, k++) && NULL==blocks[i][j][k] )
			blocks[i][j][k]=new Grass(this,
				i+shredX*shred_width,
				j+shredY*shred_width, k);
	}
}

void Shred::TestShred() {
	NormalUnderground();
	blocks[2][0][height/2]=new Chest(this);
	blocks[3][1][height/2]=new Active(this,
		shredX*shred_width+3,
		shredY*shred_width+1, height/2, SAND);
}

void Shred::NullMountain() {
	unsigned short i, j, k;
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

	unsigned short i, random;

	//bush
	random=rand()%2;
	for (i=0; i<random; ++i) {
		short x=rand()%shred_width,
		      y=rand()%shred_width;
		if ( NULL==blocks[x][y][height/2] )
			blocks[x][y][height/2]=new Bush(this);
	}

	//rabbits
	random=rand()%2;
	for (i=0; i<random; ++i) {
		short x=rand()%shred_width,
		      y=rand()%shred_width;
		if ( NULL==blocks[x][y][height/2] )
			blocks[x][y][height/2]=new Rabbit(this,
				shredX*shred_width+x,
				shredY*shred_width+y, height/2);
	}

	PlantGrass();
}

void Shred::Forest(const long longi, const long lati) {
	NormalUnderground();

	long i, j;
	unsigned short number_of_trees=0;
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
	unsigned short depth=1;
	char map[3][3];
	for (long i=longi-1; i<=longi+1; ++i)
	for (long j=lati-1;  j<=lati+1;  ++j)
		if ( '~'==(map[i-longi+1][j-lati+1]=TypeOfShred(i, j)) )
			++depth;

	NormalUnderground(depth);
	unsigned short i, j, k;

	if ('~'!=map[1][0] && '~'!=map[0][1]) { //north-west rounding
		for (i=0; i<shred_width/2; ++i)	
		for (j=0; j<shred_width/2; ++j)
			for (k=height/2-depth; k<height/2; ++k)
				if ( ((4-i)*(4-i) +
				      (4-j)*(4-j) +
				      (height/2-k)*(height/2-k)*16/depth/depth)
						> 16 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	if ('~'!=map[1][0] && '~'!=map[2][1]) { //south-west rounding
		for (i=0; i<shred_width/2; ++i)	
		for (j=shred_width/2; j<shred_width; ++j)
			for (k=height/2-depth; k<height/2; ++k)
				if ( ((4-i)*(4-i)+
				      (5-j)*(5-j)+
				      (height/2-k)*(height/2-k)*16/depth/depth)
						> 16 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	if ('~'!=map[2][1] && '~'!=map[1][2]) { //south-east rounding
		for (i=shred_width/2; i<shred_width; ++i)
		for (j=shred_width/2; j<shred_width; ++j)
			for (k=height/2-depth; k<height/2; ++k)
				if ( ((5-i)*(5-i)+
				      (5-j)*(5-j)+
				      (height/2-k)*(height/2-k)*16/depth/depth)
						> 16 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	if ('~'!=map[1][2] && '~'!=map[0][1]) { //north-east rounding
		for (i=shred_width/2; i<shred_width; ++i)
		for (j=0; j<shred_width/2; ++j)
			for (k=height/2-depth; k<height/2; ++k)
				if ( ((5-i)*(5-i)+
				      (4-j)*(4-j)+
				      (height/2-k)*(height/2-k)*16/depth/depth)
						> 16 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	for (i=0; i<shred_width; ++i)	
	for (j=0; j<shred_width; ++j)
	for (k=height/2-depth; k<height/2; ++k)
		if ( NULL==blocks[i][j][k] )
			blocks[i][j][k]=new Liquid(this,
			                i+shred_width*shredX,
			                j+shred_width*shredY, k);

	PlantGrass();
}

void Shred::Hill(const long longi, const long lati) {
	unsigned short hill_height=1;
	for (long i=longi-1; i<=longi+1; ++i)
	for (long j=lati-1;  j<=lati+1;  ++j)
		if ( '+'==TypeOfShred(i, j) )
			++hill_height;

	NormalUnderground();

	for (unsigned short i=0; i<shred_width; ++i)	
	for (unsigned short j=0; j<shred_width; ++j)
	for (unsigned short k=height/2; k<height/2+hill_height; ++k)
		if (((4.5-i)*(4.5-i)+
		     (4.5-j)*(4.5-j)+
		     (height/2-0.5-k)*(height/2-0.5-k)*16/hill_height/hill_height)<=16)
			blocks[i][j][k]=NewNormal(SOIL);
	
	PlantGrass();
}

bool Shred::Tree(const unsigned short x,
                 const unsigned short y,
                 const unsigned short z,
                 const unsigned short height) {
	if ( shred_width*3<=x+2 ||
	    shred_width*3<=y+2 ||
	    ::height-1<=z+height ||
	    height<2 )
		return false;

	unsigned short i, j, k;
	for (i=x; i<=x+2; ++i)
	for (j=y; j<=y+2; ++j)
	for (k=z; k<z+height; ++k)
		if ( NULL!=blocks[i][j][k] )
			return false;

	for (k=z; k<z+height-1; ++k) //trunk
		blocks[x+1][y+1][k]=NewNormal(WOOD);

	for (i=x; i<=x+2; ++i) //leaves
	for (j=y; j<=y+2; ++j)
	for (k=z+height/2; k<z+height; ++k)
		if ( NULL==blocks[i][j][k] )
			blocks[i][j][k]=NewNormal(GREENERY);
	
	return true;
}
