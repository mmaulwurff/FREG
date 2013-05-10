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

//Qt version in Debian stable now.
const int datastream_version=QDataStream::Qt_4_6;

long Shred::Longitude() const { return longitude; }
long Shred::Latitude()  const { return latitude; }
ushort Shred::ShredX() const { return shredX; }
ushort Shred::ShredY() const { return shredY; }

World * Shred::GetWorld() const { return world; }

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
		for (ushort i=0; i<SHRED_WIDTH; ++i)
		for (ushort j=0; j<SHRED_WIDTH; ++j) {
			PutNormalBlock(NULLSTONE, i, j, 0);
			lightMap[i][j][0]=0;
			for (ushort k=1; k<HEIGHT; ++k) {
				SetBlock(block_manager.BlockFromFile(in),
					i, j, k);
				lightMap[i][j][k]=0;
			}
			lightMap[i][j][HEIGHT-1]=MAX_LIGHT_RADIUS;
		}
		return 0;
}

Shred::Shred(
		World * const world_,
		const ushort shred_x,
		const ushort shred_y,
		const long longi,
		const long lati)
		:
		world(world_),
		longitude(longi),
		latitude(lati),
		shredX(shred_x),
		shredY(shred_y)
{
	activeListFrequent.reserve(100);
	activeListRare.reserve(500);
	activeListAll.reserve(1000);
	fallList.reserve(100);
	QFile file(FileName());
	if ( file.open(QIODevice::ReadOnly) && !LoadShred(file) ) {
		return;
	}
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		PutNormalBlock(NULLSTONE, i, j, 0);
		for (ushort k=1; k<HEIGHT-1; ++k) {
			PutNormalBlock(AIR, i, j, k);
			lightMap[i][j][k]=0;
		}
		PutNormalBlock(( (qrand()%5) ? SKY : STAR ), i, j, HEIGHT-1);
		lightMap[i][j][HEIGHT-1]=MAX_LIGHT_RADIUS;
	}
	switch ( TypeOfShred(longi, lati) ) {
		case '#': NullMountain(); break;
		case '.': Plain(); break;
		case 't': TestShred(); break;
		case '%': Forest(longi, lati); break;
		case '~': Water( longi, lati); break;
		case '_': /* empty shred */    break;
		case '-': NormalUnderground(); break;
		case 'p': Pyramid();           break;
		case '^': Mountain();          break;
		case ':': Desert();            break;
		default:
			Plain();
			fprintf(stderr,
				"Shred::Shred: unknown type of shred: %c\n",
				TypeOfShred(longi, lati));
	}
}

Shred::~Shred() {
	ushort i, j, k;
	const long mapSize=world->MapSize();
	if (
			(longitude < mapSize) && (longitude >= 0) &&
			(latitude  < mapSize) && (latitude  >= 0) )
	{
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
		for (i=0; i<SHRED_WIDTH; ++i)
		for (j=0; j<SHRED_WIDTH; ++j)
		for (k=1; k<HEIGHT; ++k) {
			blocks[i][j][k]->SaveToFile(outstr);
			block_manager.DeleteBlock(blocks[i][j][k]);
		}
		file.write(qCompress(shred_data));
		return;
	}

	for (i=0; i<SHRED_WIDTH; ++i)
	for (j=0; j<SHRED_WIDTH; ++j)
	for (k=1; k<HEIGHT-1; ++k) {
		block_manager.DeleteBlock(blocks[i][j][k]);
	}
}

void Shred::SetNewBlock(
		const int kind, const int sub,
		const ushort x, const ushort y, const ushort z)
{
	SetBlock( block_manager.NewBlock(kind, sub), x, y, z );
}

void Shred::RegisterBlock(
		Block * const block,
		const ushort x,
		const ushort y,
		const ushort z)
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
		Active * const temp=fallList[j];
		const ushort weight=temp->Weight();
		if ( weight ) {
			const ushort x=temp->X();
			const ushort y=temp->Y();
			const ushort z=temp->Z();
			if ( weight > Weight(
					x%SHRED_WIDTH,
					y%SHRED_WIDTH, z-1) )
			{
				if ( !world->Move(x, y, z, DOWN) ) {
					RemFalling(temp);
					temp->FallDamage();
				}
			} else {
				RemFalling(temp);
				temp->FallDamage();
			}
		}
	}
	for (int j=0; j<activeListFrequent.size(); ++j) {
		activeListFrequent[j]->ActFrequent();
	}
}
void Shred::PhysEventsRare() {
	for (int j=0; j<activeListRare.size(); ++j) {
		activeListRare[j]->ActRare();
	}
}

int Shred::Sub(
		const ushort x,
		const ushort y,
		const ushort z)
const {
	return blocks[x][y][z]->Sub();
}
int Shred::Kind(
		const ushort x,
		const ushort y,
		const ushort z)
const {
	return blocks[x][y][z]->Kind();
}
int Shred::Durability(
		const ushort x,
		const ushort y,
		const ushort z)
const {
	return blocks[x][y][z]->Durability();
}
int Shred::Movable(
		const ushort x,
		const ushort y,
		const ushort z)
const {
	return blocks[x][y][z]->Movable();
}
int Shred::Transparent(
		const ushort x,
		const ushort y,
		const ushort z)
const {
	return blocks[x][y][z]->Transparent();
}
ushort Shred::Weight(
		const ushort x,
		const ushort y,
		const ushort z)
const {
	return blocks[x][y][z]->Weight();
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
		default: break;
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

void Shred::AddFalling(
		const ushort x,
		const ushort y,
		const ushort z)
{
	Active * const active=blocks[x][y][z]->ActiveBlock();
	if ( active ) {
		AddFalling(active);
	}
}

void Shred::ReloadToNorth() {
	for (ushort i=0; i<activeListAll.size(); ++i) {
		activeListAll[i]->ReloadToNorth();
	}
	++shredY;
}
void Shred::ReloadToEast() {
	for (ushort i=0; i<activeListAll.size(); ++i) {
		activeListAll[i]->ReloadToEast();
	}
	--shredX;
}
void Shred::ReloadToSouth() {
	for (ushort i=0; i<activeListAll.size(); ++i) {
		activeListAll[i]->ReloadToSouth();
	}
	--shredY;
}
void Shred::ReloadToWest() {
	for (ushort i=0; i<activeListAll.size(); ++i) {
		activeListAll[i]->ReloadToWest();
	}
	++shredX;
}

Block * Shred::GetBlock(const ushort x, const ushort y, const ushort z)
const {
	return blocks[x][y][z];
}
void Shred::SetBlock(
		Block * const block,
		const ushort x,
		const ushort y,
		const ushort z)
{
	blocks[x][y][z]=block;
	RegisterBlock(block, x, y, z);
}

void Shred::PutBlock(
		Block * const block,
		const ushort x,
		const ushort y,
		const ushort z)
{
	blocks[x][y][z]=block;
}

void Shred::PutNormalBlock(
		const int sub,
		const ushort x, const ushort y, const ushort z,
		const int dir)
{
	blocks[x][y][z]=Normal(sub, dir);
}

Block * Shred::Normal(const int sub, const int dir) {
	return block_manager.NormalBlock(sub, dir);
}

QString Shred::FileName() const {
	return world->WorldName()+"/y"+
		QString::number(longitude)+"x"+
		QString::number(latitude);
}

char Shred::TypeOfShred(const long longi, const long lati) const {
	const long mapSize=world->MapSize();
	if (
			longi >= mapSize || longi < 0 ||
			lati  >= mapSize || lati  < 0 )
	{
		return '~';
	}
	QFile map(world->WorldName()+"/map.txt");
	if ( !map.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		return '.';
	}
	map.seek((mapSize+1)*longi+lati); //+1 is for '\n' in file
	char c;
	return ( map.getChar(&c) ) ? c : '.';
}

//shred generators section
//these functions fill space between the lowest nullstone layer and sky.
//so use k from 1 to heigth-2.
void Shred::NormalUnderground(const ushort depth, const int sub) {
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		ushort k;
		for (k=1; k<HEIGHT/2-6 && k<HEIGHT/2-depth-1; ++k) {
			PutNormalBlock(STONE, i, j, k);
		}
		PutNormalBlock(((qrand()%2) ? STONE : sub), i, j, k);
		for (++k; k<HEIGHT/2-depth; ++k) {
			PutNormalBlock(sub, i, j, k);
		}
	}
}

void Shred::CoverWith(const int kind, const int sub, const ushort thickness) {
	for (ushort n=thickness; n; --n) {
		for (ushort i=0; i<SHRED_WIDTH; ++i)
		for (ushort j=0; j<SHRED_WIDTH; ++j) {
			ushort k=HEIGHT-2;
			for ( ; AIR==Sub(i, j, k); --k);
			SetNewBlock(kind, sub, i, j, ++k);
		}
	}
}

void Shred::PlantGrass() {
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		ushort k;
		for (k=HEIGHT-2; Transparent(i, j, k); --k);
		if ( SOIL==Sub(i, j, k++) && AIR==Sub(i, j, k) ) {
			SetNewBlock(GRASS, GREENERY, i, j, k);
		}
	}
}

void Shred::TestShred() {
	NormalUnderground();

	//row 1
	SetNewBlock(CLOCK, IRON, 1, 1, HEIGHT/2);
	SetNewBlock(CHEST, WOOD, 3, 1, HEIGHT/2);
	SetNewBlock(ACTIVE, SAND, 5, 1, HEIGHT/2);
	PutNormalBlock(GLASS, 7, 1, HEIGHT/2);
	SetNewBlock(PILE, DIFFERENT, 9, 1, HEIGHT/2);
	SetNewBlock(PLATE, STONE, 11, 1, HEIGHT/2);
	PutNormalBlock(NULLSTONE, 13, 1, HEIGHT/2);

	//row 2
	SetNewBlock(LADDER, NULLSTONE, 1, 3, HEIGHT/2);
	SetNewBlock(LADDER, WOOD, 1, 3, HEIGHT/2+1);
	SetNewBlock(DWARF, H_MEAT, 3, 3, HEIGHT/2);
	SetNewBlock(LIQUID, WATER, 5, 3, HEIGHT/2-3);
	SetNewBlock(LIQUID, WATER, 5, 3, HEIGHT/2-3);
	SetNewBlock(LIQUID, WATER, 5, 3, HEIGHT/2-2);
	PutNormalBlock(AIR, 5, 3, HEIGHT/2-1);
	SetNewBlock(BUSH, GREENERY, 7, 3, HEIGHT/2);
	SetNewBlock(RABBIT, A_MEAT, 9, 3, HEIGHT/2-2);
	PutNormalBlock(AIR, 9, 3, HEIGHT/2-1);
	SetNewBlock(WORKBENCH, IRON, 11, 3, HEIGHT/2);
	SetNewBlock(DOOR, GLASS, 13, 3, HEIGHT/2);
	blocks[13][3][HEIGHT/2]->SetDir(NORTH);

	//row 3
	SetNewBlock(WEAPON, IRON, 1, 5, HEIGHT/2);
	SetNewBlock(BLOCK, SAND, 3, 5, HEIGHT/2);

	//suicide booth
	for (ushort i=1; i<4; ++i)
	for (ushort j=7; j<10; ++j)
	for (ushort k=HEIGHT/2; k<HEIGHT/2+5; ++k) {
		PutNormalBlock(GLASS, i, j, k);
	}
	SetNewBlock(RABBIT, A_MEAT, 2, 8, HEIGHT/2);
}

void Shred::NullMountain() {
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		ushort k;
		for (k=1; k<HEIGHT/2; ++k) {
			PutNormalBlock(NULLSTONE, i, j, k);
		}
		for ( ; k<HEIGHT-1; ++k) {
			if (i==4 || i==5 || j==4 || j==5) {
				PutNormalBlock(NULLSTONE, i, j, k);
			}
		}
	}
}

void Shred::Plain() {
	NormalUnderground();
	ushort i, num, x, y;
	//bush
	num=qrand()%4;
	for (i=0; i<=num; ++i) {
		x=qrand()%SHRED_WIDTH;
		y=qrand()%SHRED_WIDTH;
		if ( AIR==Sub(x, y, HEIGHT/2) ) {
			SetNewBlock(BUSH, WOOD, x, y, HEIGHT/2);
		}
	}
	//rabbits
	num=qrand()%4;
	for (i=0; i<=num; ++i) {
		x=qrand()%SHRED_WIDTH;
		y=qrand()%SHRED_WIDTH;
		if ( AIR==Sub(x, y, HEIGHT/2) ) {
			SetNewBlock(RABBIT, A_MEAT, x, y, HEIGHT/2);
		}
	}
	PlantGrass();
}

void Shred::Forest(const long longi, const long lati) {
	NormalUnderground();
	long i, j;
	ushort number_of_trees=0;
	for (i=longi-1; i<=longi+1; ++i)
	for (j=lati-1;  j<=lati+1;  ++j) {
		if ( '%'==TypeOfShred(i, j) ) {
			++number_of_trees;
		}
	}
	for (i=0; i<number_of_trees; ++i) {
		short x=qrand()%(SHRED_WIDTH-2);
		short y=qrand()%(SHRED_WIDTH-2);
		Tree(x, y, HEIGHT/2, 4+qrand()%5);
	}
	PlantGrass();
}

void Shred::Water(const long longi, const long lati) {
	ushort depth=1;
	char map[3][3];
	for (long i=longi-1; i<=longi+1; ++i)
	for (long j=lati-1;  j<=lati+1;  ++j) {
		if ( '~'==(map[i-longi+1][j-lati+1]=TypeOfShred(i, j)) ) {
			++depth;
		}
	}
	NormalUnderground(depth);
	ushort i, j, k;

	if ('~'!=map[1][0] && '~'!=map[0][1]) { //north-west rounding
		for (i=0; i<SHRED_WIDTH/2; ++i)
		for (j=0; j<SHRED_WIDTH/2; ++j)
			for (k=HEIGHT/2-depth; k<HEIGHT/2; ++k)
				if ( ((7-i)*(7-i) +
				      (7-j)*(7-j) +
				      (HEIGHT/2-k)*(HEIGHT/2-k)*16/depth/depth)
						> 49 )
					PutNormalBlock(SOIL, i, j, k);
	}
	if ('~'!=map[1][0] && '~'!=map[2][1]) { //south-west rounding
		for (i=0; i<SHRED_WIDTH/2; ++i)
		for (j=SHRED_WIDTH/2; j<SHRED_WIDTH; ++j)
			for (k=HEIGHT/2-depth; k<HEIGHT/2; ++k)
				if ( ((7-i)*(7-i)+
				      (8-j)*(8-j)+
				      (HEIGHT/2-k)*(HEIGHT/2-k)*16/depth/depth)
						> 49 )
					PutNormalBlock(SOIL, i, j, k);
	}
	if ('~'!=map[2][1] && '~'!=map[1][2]) { //south-east rounding
		for (i=SHRED_WIDTH/2; i<SHRED_WIDTH; ++i)
		for (j=SHRED_WIDTH/2; j<SHRED_WIDTH; ++j)
			for (k=HEIGHT/2-depth; k<HEIGHT/2; ++k)
				if ( ((8-i)*(8-i)+
				      (8-j)*(8-j)+
				      (HEIGHT/2-k)*(HEIGHT/2-k)*16/depth/depth)
						> 49 )
					PutNormalBlock(SOIL, i, j, k);
	}
	if ('~'!=map[1][2] && '~'!=map[0][1]) { //north-east rounding
		for (i=SHRED_WIDTH/2; i<SHRED_WIDTH; ++i)
		for (j=0; j<SHRED_WIDTH/2; ++j)
			for (k=HEIGHT/2-depth; k<HEIGHT/2; ++k)
				if ( ((8-i)*(8-i)+
				      (7-j)*(7-j)+
				      (HEIGHT/2-k)*(HEIGHT/2-k)*16/depth/depth)
						> 49 )
					PutNormalBlock(SOIL, i, j, k);
	}
	for (i=0; i<SHRED_WIDTH; ++i)
	for (j=0; j<SHRED_WIDTH; ++j)
	for (k=HEIGHT/2-depth; k<HEIGHT/2; ++k) {
		if ( AIR==Sub(i, j, k) ) {
			SetNewBlock(LIQUID, WATER, i, j, k);
		}
	}
	PlantGrass();
}

void Shred::Pyramid() {
	//pyramid by Panzerschrek
	//'p' - pyramid symbol
	NormalUnderground();
	unsigned short z, dz, x, y;

	//пирамида
	for( z= HEIGHT/2, dz= 0; dz< 8; z +=2, dz++ )
	{
		for( x= dz; x< ( 16 - dz ); x++ )
		{
			blocks[x][dz][z]=
			blocks[x][15 - dz][z]=
			blocks[x][dz][z+1]=
			blocks[x][15 - dz][z+1]=Normal(STONE);
		}
		for( y= dz; y< ( 16 - dz ); y++ )
		{
			blocks[dz][y][z]=
			blocks[15 - dz][y][z]=
			blocks[dz][y][z + 1]=
			blocks[15 - dz][y][z + 1]=Normal(STONE);
		}
	}

	//вход
	blocks[SHRED_WIDTH/2][0][HEIGHT/2]= Normal( AIR );

	//камера внутри
	for( z= HEIGHT/2 - 60, dz=0; dz< 8; dz++, z++ )
	for( x= 1; x< SHRED_WIDTH - 1; x++ )
	for( y= 1; y< SHRED_WIDTH - 1; y++ )
		blocks[x][y][z]= Normal( AIR );

	//шахта
	for( z= HEIGHT/2 - 52, dz= 0; dz< 52; z++, dz++ )
		blocks[SHRED_WIDTH/2][SHRED_WIDTH/2][z]= Normal( AIR );

	return;
	//летающая тарелка
	/*for( x=0; x< SHRED_WIDTH; x++ )
	for( y=0; y< SHRED_WIDTH; y++ )
	{
		float r= float( ( x - SHRED_WIDTH/2 ) * ( x - SHRED_WIDTH/2 ) )
		 + float( ( y - SHRED_WIDTH/2 ) * ( y - SHRED_WIDTH/2 ) );
		if( r < 64.0f )
			blocks[x][y][ 124 ]= Normal( STONE );
		if( r < 36.0f )
			blocks[x][y][ 125 ]= Normal( STONE );
	}*/
}

void Shred::Mountain() {
	//Доступные координаты:
	//x, y - от 0 до SHRED_WIDTH-1 включительно
	//k - от 1 до HEIGHT-2 включительно

	//заполнить нижнюю часть лоскута камнем и землёй
	NormalUnderground();

	//ushort == unsigned short
	//столб из камня, y=3, x=3
	//вид сверху:
	//*---->x
	//|
	//| +
	//|
	//v y
	//SetNewBlock устанавливает новый блок. Первый параметр - тип,
	//второй - вещество, потом координаты x, y, z;
	for(ushort k=HEIGHT/2; k<3*HEIGHT/4; ++k) {
		PutNormalBlock(STONE, 2, 2, k);
	}

	//стена из нуль-камня с запада на восток высотой 3 блока
	for(ushort i=4; i<10; ++i)
	for(ushort k=HEIGHT/2; k<HEIGHT/2+3; ++k) {
		PutNormalBlock(NULLSTONE, i, 2, k);
	}
	//в стене вырезать дырку (AIR - воздух):
	PutNormalBlock(AIR, 6, 2, HEIGHT/2+1);

	//блок из дерева появится с вероятностью 1/2.
	//Если qrand()%3 - с вероятностью 2/3 и т.д.
	if ( qrand()%2 ) {
		PutNormalBlock(WOOD, 7, 7, HEIGHT/2);
	}
}

void Shred::Desert() {
	NormalUnderground(4, SAND);
	CoverWith(ACTIVE, SAND, 4);
}

bool Shred::Tree(
		const ushort x, const ushort y, const ushort z,
		const ushort height)
{
	if ( SHRED_WIDTH<=x+2 ||
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
		if ( AIR!=Sub(i, j, k) ) {
			return false;
		}
	}
	for (k=z; k<z+height-1; ++k) { //trunk
		PutNormalBlock(WOOD, x+1, y+1, k);
	}
	//branches
	PutNormalBlock(WOOD, x,   y+1, z+height/2, WEST);
	PutNormalBlock(WOOD, x+2, y+1, z+height/2, EAST);
	//leaves
	for (i=x; i<=x+2; ++i)
	for (j=y; j<=y+2; ++j)
	for (k=z+height/2; k<z+height; ++k) {
		if ( AIR==Sub(i, j, k) ) {
			PutNormalBlock(GREENERY, i, j, k);
		}
	}
	return true;
}
