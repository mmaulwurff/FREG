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

#include <unistd.h>
#include "header.h"
#include "screen.h"
#include "world.h"
#include "blocks.h"
#include "Shred.h"

void World::Examine() const {
	unsigned short i, j, k;
	PlayerFocus(i, j, k);
	char str[note_length];

	scr->Notify( FullName(str, i, j, k), Kind(i, j, k), Sub(i, j, k));
	str[0]=0;
	
	if ( GetNote(str, i, j, k) )
		scr->NotifyAdd("Inscription:");
	scr->NotifyAdd(str);
	str[0]=0;

	sprintf(str, "Temperature: %d", Temperature(i, j, k));
	scr->NotifyAdd(str);
	str[0]=0;

	sprintf(str, "Durability: %hd", Durability(i, j, k));
	scr->NotifyAdd(str);
	str[0]=0;

	sprintf(str, "Weight: %.0f", Weight(i, j, k));
	scr->NotifyAdd(str);
	str[0]=0;
}

Block * World::GetBlock(
		const unsigned short x,
		const unsigned short y,
		const unsigned short z) const
	{ return GetShred(x, y)->GetBlock(x%shred_width, y%shred_width, z); }

void World::SetBlock(Block * block,
		const unsigned short x,
		const unsigned short y,
		const unsigned short z)
	{ GetShred(x, y)->SetBlock(block, x, y, z); }

short World::LightMap(const unsigned short i,
                      const unsigned short j,
                      const unsigned short k) const
	{ return GetShred(i, j)-> LightMap(i%shred_width, j%shred_width, k); }
void World::SetLightMap(const unsigned short level,
		const unsigned short i,
		const unsigned short j,
		const unsigned short k)
{
       	GetShred(i, j)-> SetLightMap(level, i%shred_width, j%shred_width, k);
}
void World::PlusLightMap(const unsigned short level,
		const unsigned short i,
		const unsigned short j,
		const unsigned short k)
{
	GetShred(i, j)->PlusLightMap(level, i%shred_width, j%shred_width, k);
}

void World::Shine(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k)
{
	float light_radius;
	//fprintf(stderr, "World::Shine: %hu, %hu, %hu\n", i, j, k);
	if ( !InBounds(i, j, k) ||
		NULL==GetBlock(i, j, k) ||
		0==(light_radius=GetBlock(i, j, k)->LightRadius()) )
		return;

	for (short x=ceil(i-light_radius); x<=floor(i+light_radius); ++x)
	for (short y=ceil(j-light_radius); y<=floor(j+light_radius); ++y)
	for (short z=ceil(k-light_radius); z<=floor(k+light_radius) &&
			z<height-1; ++z)
		if ( InBounds(x, y, z) &&
				Distance(i, j, k, x, y, z)<=light_radius &&
				DirectlyVisible(i, j, k, x, y, z) )
			PlusLightMap(max_light_radius/
				Distance(i, j, k, x, y, z)+1, x, y, z);
}

void World::SaveAllShreds() {
	for (unsigned short i=0; i<numShreds*numShreds; ++i)
		delete shreds[i];
}

void World::ReloadShreds(const dirs direction) {
	//ReloadShreds is called from Move,
	//so there is no need to use mutex in this function
	short x, y; //do not make unsigned, values <0 are needed for checks
	switch (direction) {
		case NORTH:
			--longitude;
			for (x=0; x<numShreds; ++x) {
				delete shreds[numShreds*(numShreds-1)+x];
				for (y=numShreds-2; y>=0; --y) {
					shreds[(y+1)*numShreds+x]=
						shreds[y*numShreds+x];
					shreds[(y+1)*numShreds+x]->
						ReloadToNorth();
				}
				shreds[x]=new Shred(this, worldName, x, 0,
						longitude, latitude);
			}
		break;
		case SOUTH:
			++longitude;
			for (x=0; x<numShreds; ++x) {
				delete shreds[x];
				for (y=1; y<numShreds; ++y) {
					shreds[(y-1)*numShreds+x]=
						shreds[y*numShreds+x];
					shreds[(y-1)*numShreds+x]->
						ReloadToSouth();
				}
				shreds[numShreds*(numShreds-1)+x]=new 
					Shred(this, worldName, x, numShreds-1,
						longitude, latitude);
			}
		break;
		case EAST:
			++latitude;
			for (y=0; y<numShreds; ++y) {
				delete shreds[y*numShreds];
				for (x=1; x<numShreds; ++x) {
					shreds[(x-1)+y*numShreds]=
						shreds[x+y*numShreds];
					shreds[(x-1)+y*numShreds]->
						ReloadToEast();
				}
				shreds[numShreds-1+y*numShreds]=new 
					Shred(this, worldName, numShreds-1, y,
						longitude, latitude);
			}
		break;
		case WEST:
			--latitude;
			for (y=0; y<numShreds; ++y) {
				delete shreds[numShreds-1+y*numShreds];
				for (x=numShreds-2; x>=0; --x) {
					shreds[(x+1)+y*numShreds]=
						shreds[x+y*numShreds];
					shreds[(x+1)+y*numShreds]->
						ReloadToWest();
				}
				shreds[y*numShreds]=new Shred(this,
						worldName, 0, y,
						longitude, latitude);
			}
		break;
		default: fprintf(stderr,
			"World::ReloadShreds(dirs): invalid direction.\n");
	}

	ReEnlightenAll();
}

void World::PhysEvents() {
	mutex_lock();

	//sun/moon moving, time increment
	++time_step;
	if ( !(time_step % time_steps_in_sec) ) {
		time_step=0;
		static bool if_star=false;
		unsigned short i=(TimeOfDay()<end_of_night) ?
			TimeOfDay()*(float)shred_width*3/end_of_night :
			(TimeOfDay()-end_of_night)*(float)shred_width*3/
				(seconds_in_day-end_of_night);
		SetBlock(NewNormal( if_star ? STAR : SKY ),
			i, int(shred_width*1.5), height-1);

		++time;

		i=(TimeOfDay()<end_of_night) ?
			TimeOfDay()*(float)shred_width*3/end_of_night :
			(TimeOfDay()-end_of_night)*(float)shred_width*3/
				(seconds_in_day-end_of_night);
		if_star=( STAR==GetBlock(i, int(shred_width*1.5),
			height-1)->Sub() ) ? true : false;
		SetBlock(NewNormal(SUN_MOON),
			i, int(shred_width*1.5), height-1);

		switch (time) {
			case end_of_evening:
			case end_of_night: ReEnlightenAll(); break;
		}
	}

	//blocks' own activities, falling
	for (unsigned short i=0; i<numShreds*numShreds; ++i)
	for (         short j=0; j<shreds[i]->activeList.size(); ++j) {
		Active * temp=shreds[i]->activeList[j];
		temp->Act();
		unsigned short x, y, z;
		temp->GetSelfXYZ(x, y, z);
		if ( 0==time_step && ' '!=temp->MakeSound() && NULL!=scr) {
			unsigned short playerX, playerY, playerZ;
			playerP->GetSelfXYZ(playerX, playerY, playerZ);
			unsigned short n;
			switch ( MakeDir(playerX, playerY, x, y) ) {
				case HERE:       n=4; break;
				case NORTH:      n=1; break;
				case NORTH_EAST: n=2; break;
				case EAST:       n=5; break;
				case SOUTH_EAST: n=8; break;
				case SOUTH:      n=7; break;
				case SOUTH_WEST: n=6; break;
				case WEST:       n=3; break;
				case NORTH_WEST: n=0; break;
				default:
					n=4;
					fprintf(stderr, "World::PhysEvents(): unlisted dir: %d\n", int(MakeDir(playerX, playerY, x, y)) );
			}
			
			unsigned short dist=Distance(playerX, playerY, playerZ,
				x, y, z);
			scr->GetSound(n, dist, temp->MakeSound(),
				temp->Kind(), temp->Sub() );
		}

		if ( temp->IfToDestroy() ) {
			--j;
			if ( NULL!=scr ) {
				if ( NULL!=scr->blockToPrintRight &&
						(Block *)(scr->blockToPrintRight)==GetBlock(x, y, z) )
					scr->blockToPrintRight=NULL;
				if ( NULL!=scr->blockToPrintLeft &&
						(Block *)(scr->blockToPrintLeft)==GetBlock(x, y, z) )
					scr->blockToPrintLeft=NULL;
			}
			delete GetBlock(x, y, z);
			SetBlock(NULL, x, y, z);
		} else if ( temp->ShouldFall() )
			Move(x, y, z, DOWN);
	}
	
	/*if (NULL!=scr) {
		scr->Print();
		if (0==time_step)
			scr->PrintSounds();
	}*/
	mutex_unlock();
}

char World::CharNumber(const unsigned short i, const unsigned short j, const unsigned short k) const {
	if ( height-1==k )
		return ' ';

	if ( (Block *)playerP==GetBlock(i, j, k) )
		switch ( playerP->GetDir() ) {
			case NORTH: return '^';
			case SOUTH: return 'v';
			case EAST:  return '>';
			case WEST:  return '<';
			case DOWN:  return 'x';
			case UP:    return '.';
			default:
				fprintf(stderr, "World::ChanNumber(int, int, int): unlisted dir: %d\n", (int)playerP->GetDir());
				return '*';
		}

	unsigned short playerX, playerY, playerZ;
	playerP->GetSelfXYZ(playerX, playerY, playerZ);
	if ( UP==GetPlayerDir() ) {
		if ( k > playerZ && k < playerZ+10 )
			return k-playerZ+'0';
	} else {
		if ( k==playerZ )
			return ' ';
		if ( k>playerZ-10 )
			return playerZ-k+'0';
	}
	return '+';
}

char World::CharNumberFront(const unsigned short i, const unsigned  short j) const {
	unsigned short ret;
	unsigned short playerX, playerY, playerZ;
	playerP->GetSelfXYZ(playerX, playerY, playerZ);
	if ( NORTH==playerP->GetDir() || SOUTH==playerP->GetDir() ) {
		if ( (ret=abs(playerY-j))<10 ) return ret+'0';
	} else
		if ( (ret=abs(playerX-i))<10 ) return ret+'0';
	return '+';
}

bool World::DirectlyVisible(float x_from, float y_from, float z_from,
		const unsigned short x_to, const unsigned short y_to, const unsigned short z_to) const {
	if (x_from==x_to && y_from==y_to && z_from==z_to)
		return true;

	unsigned short max=(abs(z_to-(int)z_from) > abs(y_to-(int)y_from)) ?
		abs(z_to-(int)z_from) :
		abs(y_to-(int)y_from);
	if (abs(x_to-x_from) > max)
		max=abs(x_to-x_from);

	float x_step=(float)(x_to-x_from)/max,
	      y_step=(float)(y_to-y_from)/max,
	      z_step=(float)(z_to-z_from)/max;

	for (unsigned short i=1; i<max; ++i)
		if ( !TransparentNotSafe(nearbyint(x_from+=x_step),
		                         nearbyint(y_from+=y_step),
		                         nearbyint(z_from+=z_step)) )
		   	return false;
	return true;
}

bool World::Visible(const unsigned short x_from, const unsigned short y_from, const unsigned short z_from,
                    const unsigned short x_to,   const unsigned short y_to,   const unsigned short z_to) const {
	short temp;
	if ((DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to)) ||
		(Transparent(x_to+(temp=(x_to>x_from) ? (-1) : 1), y_to, z_to) && DirectlyVisible(x_from, y_from, z_from, x_to+temp, y_to, z_to)) ||
		(Transparent(x_to, y_to+(temp=(y_to>y_from) ? (-1) : 1), z_to) && DirectlyVisible(x_from, y_from, z_from, x_to, y_to+temp, z_to)) ||
		(Transparent(x_to, y_to, z_to+(temp=(z_to>z_from) ? (-1) : 1)) && DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to+temp)))
			return true;
	return false;
}

int World::Move(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k,
		const dirs dir,
		const unsigned stop)
{
	mutex_lock();
	unsigned short newi, newj, newk;
	if ( NULL==GetBlock(i, j, k) ||
			NOT_MOVABLE==Movable(GetBlock(i, j, k)) ||
			Focus(i, j, k, newi, newj, newk, dir) ) {
		mutex_unlock();
		return 0;
	}
	if ( DESTROY==(GetBlock(i, j, k)->BeforeMove(dir)) ) {
		delete GetBlock(i, j, k);
		SetBlock(NULL, i, j, k);
		mutex_unlock();
		return 1;
	}
	if ( !stop || (ENVIRONMENT==GetBlock(i, j, k)->Movable() &&
			Equal(GetBlock(i, j, k), GetBlock(newi, newj, newk))) )
	{
		mutex_unlock();
		return 0;
	}
	short numberMoves=0;
	if ( ENVIRONMENT!=Movable(GetBlock(newi, newj, newk)) &&
			!(numberMoves=Move(newi, newj, newk, dir, stop-1)) ) {
		mutex_unlock();
		return 0;
	}

	Block * temp=GetBlock(i, j, k);
	SetBlock(GetBlock(newi, newj, newk), i, j, k);
	SetBlock(temp, newi, newj, newk);
	
	ReEnlighten(newi, newj, newk);

	if ( NULL!=GetBlock(i, j, k) )
		GetBlock(i, j, k)->Move( Anti(dir) );

	if ( GetBlock(newi, newj, newk)->Move(dir) )
		GetPlayerCoords(newi, newj, newk);

	if ( Weight(GetBlock(newi, newj, newk)) ) {
		if ( Weight(GetBlock(newi, newj, newk))>
				Weight(GetBlock(newi, newj, newk-1)) )
			numberMoves+=Move(newi, newj, newk, DOWN, stop-1);
		else if ( Weight(GetBlock(newi, newj, newk))<
				Weight(GetBlock(newi, newj, newk+1)) )
			numberMoves+=Move(newi, newj, newk, UP, stop-1);
	}

	++numberMoves;

	mutex_unlock();
	return numberMoves;
}

int World::PlayerMove() { return PlayerMove( playerP->GetDir() ); }

int World::PlayerMove(const dirs dir) {
	unsigned short playerX, playerY, playerZ;
	GetPlayerCoords(playerX, playerY, playerZ);
	scr->viewLeft=NORMAL;
	return Move( playerX, playerY, playerZ, dir );
}

void World::Jump(
		const unsigned short i,
		const unsigned short j,
		unsigned short k)
{
	mutex_lock();
	if ( NULL==GetBlock(i, j, k) ||
			MOVABLE!=GetBlock(i, j, k)->Movable() ) {
		mutex_unlock();
		return;
	}

	Block * to_move=GetBlock(i, j, k);
	GetBlock(i, j, k)->SetWeight(0);
	dirs dir=to_move->GetDir();
	short k_plus=Move(i, j, k, (DOWN==dir) ? DOWN : UP, 1);
	if ( k_plus ) {
		k+=((DOWN==dir) ? (-1) : 1) * k_plus;
		GetBlock(i, j, k)->SetWeight();
		if ( !Move( i, j, k, to_move->GetDir()) )
			Move(i, j, k, DOWN);
	} else
		GetBlock(i, j, k)->SetWeight();

	mutex_unlock();
}

void World::SetPlayerDir(const dirs dir) { playerP->SetDir(dir); }

dirs World::GetPlayerDir() const { return playerP->GetDir(); }

void World::GetPlayerCoords(
		unsigned short & x,
		unsigned short & y,
		unsigned short & z) const
	{ playerP->GetSelfXYZ(x, y, z); }

void World::GetPlayerCoords(
		unsigned short & x,
		unsigned short & y) const
	{ playerP->GetSelfXY(x, y); }

void World::GetPlayerZ(unsigned short & z) const { playerP->GetSelfZ(z); }
	
int World::Focus(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k,
		unsigned short & i_target,
		unsigned short & j_target,
		unsigned short & k_target,
		const dirs dir) const
{
	i_target=i;
	j_target=j;
	k_target=k;
	switch (dir) {
		case NORTH: --j_target; break;
		case SOUTH: ++j_target; break;
		case EAST:  ++i_target; break;
		case WEST:  --i_target; break;
		case DOWN:  --k_target; break;
		case UP:    ++k_target; break;
		default: fprintf(stderr, "World::Focus(int, int, int, int&, int&, int&, dirs): unlisted dir: %d\n", (int)dir);
	}
	return ( !InBounds(i_target, j_target, k_target) );
}

int World::Focus(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k,
		unsigned short & i_target,
		unsigned short & j_target,
		unsigned short & k_target) const
{
	return Focus( i, j, k, i_target, j_target, k_target,
		GetBlock(i, j, k)->GetDir() );
}

void World::Damage(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k,
		const unsigned short dmg, //see default in class definition
		const damage_kinds dmg_kind, //see default in class definition
		const bool destroy) //see default in class definition
{
	if ( !InBounds(i, j, k) || NULL==GetBlock(i, j, k) )
		return;
			
	if ( GetBlock(i, j, k)->Normal() )
		SetBlock(new Block(GetBlock(i, j, k)->Sub()), i, k, k);
		
	if ( 0<GetBlock(i, j, k)->Damage(dmg, dmg_kind) )
		return;

	Block * temp=GetBlock(i, j, k);
	if (temp==scr->blockToPrintLeft)
		scr->viewLeft=NORMAL;
	if (temp==scr->blockToPrintRight)
		scr->viewRight=NORMAL;
	
	Block * dropped=temp->DropAfterDamage();
	if ( PILE!=temp->Kind() && (temp->HasInventory() || NULL!=dropped) ) {
		Pile * new_pile=new Pile(GetShred(i, j), i, j, k);
		SetBlock(new_pile, i, j, k);
		if ( temp->HasInventory() )
			new_pile->GetAll(temp);
		if ( !(new_pile->Get(dropped)) )
			delete dropped;
	} else
		SetBlock(NULL, i, j, k);
	
	if (destroy)
		delete temp;
	else {
		temp->ToDestroy();
		temp->Unregister();
	}

	ReEnlighten(i, j, k);
}

void World::Use(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k)
{
	if ( !InBounds(i, j, k) || NULL==GetBlock(i, j, k) )
		return;

	switch ( GetBlock(i, j, k)->Use() ) {
		case OPEN:
			if (INVENTORY!=scr->viewLeft) {
				scr->viewLeft=INVENTORY;
				scr->blockToPrintLeft=GetBlock(i, j, k);
			} else
				scr->viewLeft=NORMAL;
		break;
		default: scr->viewLeft=NORMAL;
	}
}

bool World::Build(Block * block,
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) {
	if ( !(InBounds(i, j, k) &&
			NULL==GetBlock(i, j, k) &&
			block->CanBeOut()) ) {
		scr->Notify("You can not build.");
		return false;
	}

	block->Restore();
	if ( block->ActiveBlock() )
		((Active *)block)->Register(GetShred(i, j), i, j, k);
	SetBlock(block, i, j, k);
	ReEnlighten(i, j, k);
	return true;
}

void World::PlayerBuild(const unsigned short n) {
	Block * temp=playerP->Drop(n);
	unsigned short i, j, k;
	PlayerFocus(i, j, k);
	if ( !Build(temp, i, j, k) )
		playerP->Get(temp);
}

void World::Inscribe(Dwarf * const dwarf) {
	if (!dwarf->CarvingWeapon()) {
		scr->Notify("You need some tool for inscribing!\n");
		return;
	}
	unsigned short i, j, k;
	dwarf->GetSelfXYZ(i, j, k);
	unsigned short i_to, j_to, k_to;
	if ( !Focus(i, j, k, i_to, j_to, k_to) )
		Inscribe(i_to, j_to, k_to);
}

void World::Inscribe(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k)
{
	if ( !InBounds(i, j, k) || NULL==GetBlock(i, j, k) )
		return;

	if ( GetBlock(i, j, k)->Normal() )
		SetBlock(new Block(GetBlock(i, j, k)->Sub()),
			i, j, k);
	char str[note_length];
	GetBlock(i, j, k)->Inscribe(scr->GetString(str));
}

void World::Eat(Block * who, Block * food) {
	if ( NULL==who || NULL==food )
		return;
	if ( who->Eat(food) ) {
		delete food;
		food=NULL;
	}
}

void World::PlayerEat(unsigned short n) {
	if ( inventory_size<=n ) {
		scr->Notify("What?");
		return;
	}
	unsigned short playerX, playerY, playerZ;
	playerP->GetSelfXYZ(playerX, playerY, playerZ);
	Block * food=playerP->Drop(n);
	if ( !playerP->Eat(food) ) {
		playerP->Get(food);
		scr->Notify("You can't eat this.");
	} else
		scr->Notify("Yum!");
	if ( seconds_in_day*time_steps_in_sec < playerP->Satiation() )
		scr->NotifyAdd("You have gorged yourself!");
}

void World::Exchange(
		const unsigned short i_from,
		const unsigned short j_from,
		const unsigned short k_from,
		const unsigned short i_to,
		const unsigned short j_to,
		const unsigned short k_to,
		const unsigned short n)
{
	if ( mutex_trylock() )
		return;

	Inventory * inv_from=(Inventory *)(
			HasInventory(i_from, j_from, k_from) );
	Inventory * inv_to=(Inventory *)( HasInventory(i_to, j_to, k_to) );

	if ( NULL==inv_from ) {
		mutex_unlock();
		return;
	}

	if ( NULL!=inv_to ) {
		Block * temp=inv_from->Drop(n);
		if ( NULL!=temp && !inv_to->Get(temp) ) {
			inv_from->Get(temp);
			scr->Notify("Not enough room\n");
		}
		mutex_unlock();
		return;
	}
	
	Block * block=GetBlock(i_to, j_to, k_to);
	if ( NULL==block ) {
		Pile * newpile=new
			Pile(GetShred(i_to, j_to), i_to, j_to, k_to);
		newpile->Get( inv_from->Drop(n) );
		block=newpile;
	}

	mutex_unlock();
}

void World::ExchangeAll(
		const unsigned short i_from,
		const unsigned short j_from,
		const unsigned short k_from,
		const unsigned short i_to,
		const unsigned short j_to,
		const unsigned short k_to)
{
	if ( NULL==GetBlock(i_from, j_from, k_from) ||
			NULL==GetBlock(i_to, j_to, k_to) )
		return;       
	
	Inventory * to=(Inventory *)(GetBlock(i_to, j_to, k_to)->
			HasInventory());
	if ( NULL!=to )
		to->GetAll(GetBlock(i_from, j_from, k_from));
}

void World::Wield(Dwarf * const dwarf, const unsigned short n) {
	if ( inventory_size<=n )
		return;

	Block * temp=dwarf->Drop(n);
	if ( NULL==temp )
		return;

	if ( !dwarf->Wield(temp) )
		dwarf->Get(temp);
}

char * World::FullName(char * const str,
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
{
	if ( InBounds(i, j, k) )
		(NULL==GetBlock(i, j, k)) ?
			WriteName(str, "Air") :
			GetBlock(i, j, k)->FullName(str);
	return str;
}

subs World::Sub(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
	{ return GetShred(i, j)->Sub(i%shred_width, j%shred_width, k); }

kinds World::Kind(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
	{ return GetShred(i, j)->Kind(i%shred_width, j%shred_width, k); }

short World::Durability(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
{
	return (!InBounds(i, j, k) || NULL==GetBlock(i, j, k)) ?
		0 :
		GetBlock(i, j, k)->Durability();
}

int World::TransparentNotSafe(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const {
	return GetShred(i, j)->
		Transparent(i%shred_width, j%shred_width, k);
}

int World::Movable(const Block * const block) const
	{ return (NULL==block) ? ENVIRONMENT : block->Movable(); }

double World::Weight(const Block * const block) const
	{ return (NULL==block) ? 0 : block->Weight(); }

void * World::HasInventory(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
{
	return (!InBounds(i, j, k) || NULL==GetBlock(i, j, k)) ?
		NULL :
		GetBlock(i, j, k)->HasInventory();
}

void * World::ActiveBlock(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
{
	return (!InBounds(i, j, k) || NULL==GetBlock(i, j, k)) ?
		NULL :
		GetBlock(i, j, k)->ActiveBlock();
}

int World::Temperature(
		const unsigned short i_center,
		const unsigned short j_center,
		const unsigned short k_center) const
{
	if (!InBounds(i_center, j_center, k_center) ||
			NULL==GetBlock(i_center, j_center, k_center) ||
			height-1==k_center)
		return 0;

	short temperature=GetBlock(i_center, j_center, k_center)->
		Temperature();
	if ( temperature )
		return temperature;

	for (short i=i_center-1; i<=i_center+1; ++i)
	for (short j=j_center-1; j<=j_center+1; ++j)
	for (short k=k_center-1; k<=k_center+1; ++k)
		if (InBounds(i, j, k) && NULL!=GetBlock(i, j, k))
			temperature+=GetBlock(i, j, k)->Temperature();
	return temperature/2;
}

bool World::GetNote(char * const str,
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
{
	if ( InBounds(i, j, k) && NULL!=GetBlock(i, j, k) )
		return GetBlock(i, j, k)->GetNote(str);
	return false;
}

bool World::Equal(
		const Block * const block1,
		const Block * const block2) const
{
	if ( NULL==block1 && NULL==block2 ) return true;
	if ( NULL==block1 || NULL==block2 ) return false;
	return *block1==*block2;
}

char World::MakeSound(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
{
	return (NULL==GetBlock(i, j, k)) ?
		' ' :
		GetBlock(i, j, k)->MakeSound();
}

float World::LightRadius(
		const unsigned short i,
		const unsigned short j,
		const unsigned short k) const
{
	return (NULL==GetBlock(i, j, k)) ?
		0 :
		GetBlock(i, j, k)->LightRadius();
}

/*World::World(QString world_name) {
	World(world_name, 1, 1,
		shred_width, shred_width, height/2,
		end_of_night, 3);
}*/

World::World(QString world_name,
		const unsigned long longi,
		const unsigned long lati,
		const unsigned short spawn_x,
		const unsigned short spawn_y,
		const unsigned short spawn_z,
		const unsigned long time_,
		const unsigned short num_shreds)
		:
		time_step(0),
		time(time_),
		spawnX(spawn_x),
		spawnY(spawn_y),
		spawnZ(spawn_z),
		longitude(longi),
		latitude(lati),
		worldName(world_name),
		numShreds(num_shreds),
		mutex(QMutex::Recursive),
		scr(NULL)
{
	for (unsigned short i=STONE; i<AIR; ++i) {
		normal_blocks[i]=new Block(subs(i));
		normal_blocks[i]->SetNormal(1);
	}

	if ( numShreds<3 || numShreds%2!=1 )
		numShreds=3;

	shreds=new Shred *[numShreds*numShreds];
	unsigned short x, y;
	unsigned long i, j;
	for (i=longi-numShreds/2, x=0; x<numShreds; ++i, ++x)
	for (j=lati -numShreds/2, y=0; y<numShreds; ++j, ++y)
		shreds[y*numShreds+x]=new Shred(this, world_name, x, y, i, j);
	
	if ( DWARF!=Kind(spawnX, spawnY, spawnZ) ) {
		if (NULL!=GetBlock(spawnX, spawnY, spawnZ) &&
				!(GetBlock(spawnX, spawnY, spawnZ)->
					Normal()) )
			delete GetBlock(spawnX, spawnY, spawnZ);

		SetBlock((Block*)(playerP=new
			Dwarf(GetShred(spawnX, spawnY),
				spawnX, spawnY, spawnZ)),
			spawnX, spawnY, spawnZ);
		fprintf(stderr, "World::World: new player place\n");
	} else
		playerP=(Dwarf *)GetBlock(spawnX, spawnY, spawnZ);
	
	ReEnlightenAll();

	//thread=new Thread(this);
	//thread->start();
}

World::~World() {
	mutex_lock();
	//thread->Stop();
	//thread->wait();
	//delete thread;
	mutex_unlock();

	SaveAllShreds();

	for (unsigned short i=STONE; i<AIR; ++i)
		delete normal_blocks[i];

	GetPlayerCoords(spawnX, spawnY, spawnZ);
	fprintf(stderr, "hello\n");
	FILE * file=fopen((worldName+"_save").toAscii().constData(), "w");
	if ( NULL==file ) {
		fprintf(stderr, "World::~World: Savefile write error: %s\n",
				(worldName+"_save").toAscii().constData());
       		return;
	}

	fprintf(file, "longitude: %ld\nlatitude: %ld\nspawnX: %hd\nspawnY: %hd\nspawnZ: %hd\ntime: %ld\nWorld:%s\n",
			longitude, latitude,
			spawnX, spawnY, spawnZ,
			time, worldName.toAscii().constData());
	fclose(file);
}
