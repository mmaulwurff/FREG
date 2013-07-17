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
#include "BlockManager.h"

//Qt version in Debian stable that time.
const quint8 DATASTREAM_VERSION=QDataStream::Qt_4_6;

long Shred::Longitude() const { return longitude; }
long Shred::Latitude()  const { return latitude; }
ushort Shred::ShredX() const { return shredX; }
ushort Shred::ShredY() const { return shredY; }

World * Shred::GetWorld() const { return world; }

bool Shred::LoadShred(QFile & file) {
	QDataStream in(qUncompress(file.readAll()));
	quint8 version;
	in >> version;
	if ( Q_UNLIKELY(DATASTREAM_VERSION!=version) ) {
		fprintf(stderr,
			"Wrong version: %d\nGenerating new shred.\n",
			DATASTREAM_VERSION);
		return false;
	}
	in.setVersion(DATASTREAM_VERSION);
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		PutNormalBlock(NULLSTONE, i, j, 0);
		lightMap[i][j][0]=0;
		for (ushort k=1; k<HEIGHT-1; ++k) {
			SetBlock(block_manager.BlockFromFile(in), i, j, k);
			lightMap[i][j][k]=0;
		}
		SetBlock(block_manager.BlockFromFile(in), i, j, HEIGHT-1);
		lightMap[i][j][HEIGHT-1]=1;
	}
	return true;
}

Shred::Shred(World * const world_,
		const ushort shred_x, const ushort shred_y,
		const long longi, const long lati)
	:
		world(world_),
		longitude(longi),
		latitude(lati),
		shredX(shred_x),
		shredY(shred_y)
{
	activeListFrequent.reserve(100);
	activeListRare.reserve(500);
	activeListAll.reserve(600);
	fallList.reserve(100);
	QFile file(FileName());
	if ( file.open(QIODevice::ReadOnly) && LoadShred(file) ) {
		return;
	}
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		PutNormalBlock(NULLSTONE, i, j, 0);
		lightMap[i][j][0]=0;
		for (ushort k=1; k<HEIGHT-1; ++k) {
			PutNormalBlock(AIR, i, j, k);
			lightMap[i][j][k]=0;
		}
		PutNormalBlock(( (qrand()%5) ? SKY : STAR ), i, j, HEIGHT-1);
		lightMap[i][j][HEIGHT-1]=1;
	}
	switch ( TypeOfShred(longi, lati) ) {
		case SHRED_NULLMOUNTAIN: NullMountain(); break;
		case SHRED_PLAIN: Plain(); break;
		case SHRED_TESTSHRED: TestShred(); break;
		case SHRED_PYRAMID: Pyramid(); break;
		case SHRED_HILL: Hill(); break;
		case SHRED_DESERT: Desert(); break;
		case SHRED_WATER: Water(); break;
		case SHRED_FOREST: Forest(); break;
		case SHRED_MOUNTAIN: Mountain(); break;
		case SHRED_EMPTY: /* empty shred */ break;
		case SHRED_NORMAL_UNDERGROUND: NormalUnderground(); break;
		default:
			Plain();
			fprintf(stderr,
				"Shred::Shred: unlisted type: %c, code %d\n",
				TypeOfShred(longi, lati),
				int(TypeOfShred(longi, lati)));
	}
} //Shred::Shred

Shred::~Shred() {
	foreach(Active * const active, activeListAll) {
		active->SetShredNull();
	}
	const long mapSize=world->MapSize();
	if (
			(longitude < mapSize) && (longitude >= 0) &&
			(latitude  < mapSize) && (latitude  >= 0) )
	{
		QFile file(FileName());
		if ( Q_UNLIKELY(!file.open(QIODevice::WriteOnly)) ) {
			fputs("Shred::~Shred: Write Error\n", stderr);
			return;
		}
		QByteArray shred_data;
		shred_data.reserve(100000);
		QDataStream outstr(&shred_data, QIODevice::WriteOnly);
		outstr << DATASTREAM_VERSION;
		outstr.setVersion(DATASTREAM_VERSION);
		for (ushort i=0; i<SHRED_WIDTH; ++i)
		for (ushort j=0; j<SHRED_WIDTH; ++j) {
			for (ushort k=1; k<HEIGHT-1; ++k) {
				blocks[i][j][k]->SaveToFile(outstr);
				block_manager.DeleteBlock(blocks[i][j][k]);
			}
			blocks[i][j][HEIGHT-1]->SaveToFile(outstr);
		}
		file.write(qCompress(shred_data));
		return;
	}
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j)
	for (ushort k=1; k<HEIGHT-1; ++k) {
		block_manager.DeleteBlock(blocks[i][j][k]);
	}
} //Shred::~Shred

void Shred::SetNewBlock(const int kind, const int sub,
		const ushort x, const ushort y, const ushort z,
		const int dir)
{
	block_manager.DeleteBlock(blocks[x][y][z]);
	Block * const block=block_manager.NewBlock(kind, sub);
	block->SetDir(dir);
	SetBlock(block, x, y, z);
}

void Shred::RegisterBlock(Block * const block,
		const ushort x, const ushort y, const ushort z)
{
	Active * const active=block->ActiveBlock();
	if ( active ) {
		active->Register(this,
			SHRED_WIDTH*shredX+x,
			SHRED_WIDTH*shredY+y, z);
	}
}

void Shred::PhysEventsFrequent() {
	for (int j=0; j<fallList.size(); ++j) {
		Active * const temp=fallList.at(j);
		const ushort weight=temp->Weight();
		if ( weight ) {
			const ushort x=temp->X();
			const ushort y=temp->Y();
			const ushort z=temp->Z();
			if ( weight<=GetBlock(x%SHRED_WIDTH, y%SHRED_WIDTH,
						z-1)->Weight()
					|| !world->Move(x, y, z, DOWN) )
			{
				RemFalling(temp);
				temp->FallDamage();
			}
			world->DestroyAndReplace(x, y, z);
		}
	}
	for (int j=0; j<activeListFrequent.size(); ++j) {
		Active * const active=activeListFrequent.at(j);
		active->ActFrequent();
		world->DestroyAndReplace(active->X(),active->Y(), active->Z());
	}
}
void Shred::PhysEventsRare() {
	for (int j=0; j<activeListRare.size(); ++j) {
		Active * const active=activeListRare.at(j);
		active->ActRare();
		world->DestroyAndReplace(active->X(),active->Y(), active->Z());
	}
}

int Shred::Sub(const ushort x, const ushort y, const ushort z) const {
	return blocks[x][y][z]->Sub();
}

void Shred::AddActive(Active * const active) {
	activeListAll.append(active);
	switch ( active->ShouldAct() ) {
		case FREQUENT:
			activeListFrequent.append(active);
		break;
		case FREQUENT_AND_RARE:
			activeListFrequent.append(active);
		//no break;
		case RARE:
			activeListRare.append(active);
		break;
	}
}

void Shred::RemActive(Active * const active) {
	activeListAll.removeOne(active);
	activeListFrequent.removeOne(active);
	activeListRare.removeOne(active);
}

void Shred::AddFalling(Active * const active) {
	if ( !active->IsFalling() && active->ShouldFall() ) {
		active->SetFalling(true);
		fallList.append(active);
	}
}

void Shred::RemFalling(Active * const active) {
	fallList.removeOne(active);
	active->SetFalling(false);
}

void Shred::AddFalling(const ushort x, const ushort y, const ushort z) {
	Active * const active=blocks[x][y][z]->ActiveBlock();
	if ( active ) {
		AddFalling(active);
	}
}

void Shred::AddShining(Active * const active) {
	shiningList.append(active);
}

void Shred::RemShining(Active * const active) {
	shiningList.removeOne(active);
}

void Shred::ReloadToNorth() {
	for (ushort i=0; i<activeListAll.size(); ++i) {
		activeListAll.at(i)->ReloadToNorth();
	}
	++shredY;
}
void Shred::ReloadToEast() {
	for (ushort i=0; i<activeListAll.size(); ++i) {
		activeListAll.at(i)->ReloadToEast();
	}
	--shredX;
}
void Shred::ReloadToSouth() {
	for (ushort i=0; i<activeListAll.size(); ++i) {
		activeListAll.at(i)->ReloadToSouth();
	}
	--shredY;
}
void Shred::ReloadToWest() {
	for (ushort i=0; i<activeListAll.size(); ++i) {
		activeListAll.at(i)->ReloadToWest();
	}
	++shredX;
}

Block *Shred::GetBlock(const ushort x, const ushort y, const ushort z) const {
	return blocks[x][y][z];
}

void Shred::SetBlock(Block * const block,
		const ushort x, const ushort y, const ushort z)
{
	RegisterBlock((blocks[x][y][z]=block), x, y, z);
}

void Shred::PutBlock(Block * const block,
		const ushort x, const ushort y, const ushort z)
{
	blocks[x][y][z]=block;
}

void Shred::PutNormalBlock(const int sub,
		const ushort x, const ushort y, const ushort z)
{
	blocks[x][y][z]=Normal(sub);
}

Block * Shred::Normal(const int sub) {
	return block_manager.NormalBlock(sub);
}

QString Shred::FileName() const {
	return QString("%1/y%2x%3").
		arg(world->WorldName()).
		arg(longitude).
		arg(latitude);
}

char Shred::TypeOfShred(const long longi, const long lati) const {
	return world->TypeOfShred(longi, lati);
}

//shred generators section
//these functions fill space between the lowest nullstone layer and sky.
//so use k from 1 to heigth-2.
void Shred::CoverWith(const int kind, const int sub) {
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		ushort k=HEIGHT-2;
		for ( ; AIR==Sub(i, j, k); --k);
		SetNewBlock(kind, sub, i, j, ++k);
	}
}

void Shred::RandomDrop(const ushort num, const int kind, const int sub,
		const bool on_water)
{
	for (ushort i=0; i<num; ++i) {
		ushort x=qrand()%SHRED_WIDTH;
		ushort y=qrand()%SHRED_WIDTH;
		for (ushort z=HEIGHT-2; z>0; --z) {
			if ( Sub(x, y, z)!=AIR ) {
				if( on_water || Sub(x, y, z)!=WATER ) {
					SetNewBlock(kind, sub, x, y, z+1);
				}
				break;
			}
		}
	}
}

void Shred::PlantGrass() {
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		ushort k;
		for (k=HEIGHT-2; GetBlock(i, j, k)->Transparent(); --k);
		if ( SOIL==Sub(i, j, k++) && AIR==Sub(i, j, k) ) {
			SetNewBlock(GRASS, GREENERY, i, j, k);
		}
	}
}

void Shred::TestShred() {
	const ushort level=FlatUndeground()+1;
	short row=1, column=-1;
	//row 1
	SetNewBlock(CLOCK, IRON, column+=2, row, level);
	SetNewBlock(CHEST, WOOD, column+=2, row, level);
	SetNewBlock(ACTIVE, SAND, column+=2, row, level);
	PutNormalBlock(GLASS, column+=2, 1, level);
	SetNewBlock(PILE, DIFFERENT, column+=2, row, level);
	SetNewBlock(PLATE, STONE, column+=2, row, level);
	PutNormalBlock(NULLSTONE, column+=2, row, level);
	//row 2
	column=-1;
	row+=2;
	SetNewBlock(LADDER, NULLSTONE, column+=2, row, level);
	//tall ladder
	for (ushort i=level+1; i<=level+5 && i<HEIGHT-1; ++i) {
		SetNewBlock(LADDER, WOOD, column, row, i);
	}
	SetNewBlock(DWARF, H_MEAT, column+=2, row, level);
	SetNewBlock(LIQUID, WATER, column+=2, row, level - 3);
	SetNewBlock(LIQUID, WATER, column, row, level - 3);
	SetNewBlock(LIQUID, WATER, column, row, level - 2);
	PutNormalBlock(AIR, column, row, level - 1);
	SetNewBlock(BUSH, GREENERY, column+=2, row, level);
	SetNewBlock(RABBIT, A_MEAT, column+=2, row, level - 2);
	PutNormalBlock(AIR, column, row, level - 1);
	SetNewBlock(WORKBENCH, IRON, column+=2, row, level);
	SetNewBlock(DOOR, GLASS, column+=2, row, level);
	blocks[column][row][level]->SetDir(NORTH);
	//row 3
	column=-1;
	row+=2;
	SetNewBlock(WEAPON, IRON, column+=2, row, level);
	SetNewBlock(BLOCK, SAND, column+=2, row, level);
	SetNewBlock(BLOCK, WATER, column+=2, row, level);
	SetNewBlock(ACTIVE, WATER, column+=2, row, level);
	SetNewBlock(DOOR, STONE, column+=2, row, level);
	blocks[9][5][level]->SetDir(NORTH);
	SetNewBlock(BLOCK, CLAY, column+=2, row, level);
	SetNewBlock(LIQUID, STONE, column+=2, row, level-1);
	// row 4
	column=-1;
	row+=2;
	SetNewBlock(TEXT, PAPER, column+=2, row, level);
	//suicide booth
	/*for (ushort i=1; i<4; ++i)
	for (ushort j=7; j<10; ++j)
	for (ushort k=level; k<level+5; ++k) {
		if ( k<HEIGHT-1 ) {
			PutNormalBlock(GLASS, i, j, k);
		}
	}
	SetNewBlock(RABBIT, A_MEAT, 2, 8, level);*/
} //Shred::TestShred

void Shred::NullMountain() {
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		ushort k;
		for (k=1; k<HEIGHT/2; ++k) {
			PutNormalBlock(NULLSTONE, i, j, k);
		}
		for ( ; k<HEIGHT-1; ++k) {
			if ( i==4 || i==5 || j==4 || j==5 ) {
				PutNormalBlock(NULLSTONE, i, j, k);
			}
		}
	}
}

void Shred::Pyramid() {
	//pyramid by Panzerschrek
	ushort level=FlatUndeground();
	if ( level > HEIGHT-1-16 ) {
		level=HEIGHT-1-16;
	}
	ushort z, dz, x, y;
	//пирамида
	for (z=level+1, dz=0; dz<8; z+=2, ++dz) {
		for (x=dz; x<(16 - dz); ++x) {
			blocks[x][dz][z] =
				blocks[x][15-dz][z]=
				blocks[x][dz][z+1] =
				blocks[x][15-dz][z+1]=Normal(STONE);
		}
		for (y=dz; y<(16-dz); ++y) {
			blocks[dz][y][z] =
				blocks[15-dz][y][z]=
				blocks[dz][y][z+1] =
				blocks[15-dz][y][z+1]=Normal(STONE);
		}
	}
	//вход
	blocks[SHRED_WIDTH/2][0][level+1]=Normal(AIR);
	//камера внутри
	for (z=HEIGHT/2-60, dz=0; dz<8; ++dz, ++z) {
		for (x=1; x<SHRED_WIDTH-1; ++x)
		for (y=1; y<SHRED_WIDTH-1; ++y) {
			blocks[x][y][z]=Normal(AIR);
		}
	}
	//шахта
	for (z=HEIGHT/2-52; z<=level; ++z) {
		blocks[SHRED_WIDTH/2][SHRED_WIDTH/2][z]=Normal(AIR);
	}
} //Shred::Pyramid

void Shred::NormalCube(const ushort x_start, const ushort y_start,
		const ushort z_start,
		const ushort x_size, const ushort y_size, const ushort z_size,
		const int sub)
{
	for (ushort i=x_start; i<x_start+x_size; ++i)
	for (ushort j=y_start; j<y_start+y_size; ++j)
	for (ushort k=z_start; k<z_start+z_size; ++k) {
		if ( InBounds(i, j, k) ) {
			PutNormalBlock(sub, i, j, k);
		}
	}
}

bool Shred::Tree(const ushort x, const ushort y, const ushort z,
		const ushort height)
{
	if (
			SHRED_WIDTH<=x+2 ||
			SHRED_WIDTH<=y+2 ||
			HEIGHT-1<=z+height ||
			height<2 )
	{
		return false;
	}
	ushort i, j, k;
	for (i=x; i<=x+2; ++i)
	for (j=y; j<=y+2; ++j)
	for (k=z; k<z+height; ++k) {
		if ( AIR!=Sub(i, j, k) && WATER!=Sub(i, j, k) ) {
			return false;
		}
	}
	for (k=z; k < z + height - 1; ++k) { //trunk
		PutNormalBlock(WOOD, x+1, y+1, k);
	}
	if ( ENVIRONMENT==GetBlock(x+1, y+1, z-1)->Movable() ) {
		block_manager.DeleteBlock(blocks[x+1][y+1][z-1]);
		PutNormalBlock(WOOD, x+1, y+1, z-1);
	}
	//branches
	if ( qrand()%2 ) SetNewBlock(BLOCK, WOOD, x,   y+1, z+height/2, WEST);
	if ( qrand()%2 ) SetNewBlock(BLOCK, WOOD, x+2, y+1, z+height/2, EAST);
	if ( qrand()%2 ) SetNewBlock(BLOCK, WOOD, x+1, y,   z+height/2, NORTH);
	if ( qrand()%2 ) SetNewBlock(BLOCK, WOOD, x+1, y+2, z+height/2, SOUTH);
	//leaves
	for (i=x; i<=x+2; ++i)
	for (j=y; j<=y+2; ++j)
	for (k=z+height/2; k<z+height; ++k) {
		if ( AIR==Sub(i, j, k) ) {
			PutNormalBlock(GREENERY, i, j, k);
		}
	}
	return true;
} //Shred::Tree

bool Shred::InBounds(const ushort x, const ushort y, const ushort z) const {
	return ( x<SHRED_WIDTH && y<SHRED_WIDTH && z && z<HEIGHT-1 );
}
