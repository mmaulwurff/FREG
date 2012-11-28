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

#include "blocks.h"
#include "world.h"
#include "Shred.h"

char * Block::FullName(char * const str) const {
	switch (sub) {
		case WATER:      return WriteName(str, "Ice");
		case STONE:      return WriteName(str, "Stone");
		case MOSS_STONE: return WriteName(str, "Moss stone");
		case NULLSTONE:  return WriteName(str, "Nullstone");
		case GLASS:      return WriteName(str, "Glass");
		case STAR: case SUN_MOON:
		case SKY:        return WriteName(str, "Air");
		case SOIL:       return WriteName(str, "Soil");
		case HAZELNUT:   return WriteName(str, "Hazelnut");
		case WOOD:       return WriteName(str, "Wood");
		case GREENERY:   return WriteName(str, "Leaves");
		case ROSE:       return WriteName(str, "Rose");
		case A_MEAT:     return WriteName(str, "Animal meat");
		case H_MEAT:     return WriteName(str, "Not animal meat");
		default:
			fprintf(stderr, "Block::FullName(char *): Block has unknown substance: %d", int(sub));
			return WriteName(str, "Unknown block");
	}
}

int Block::Damage(
		const unsigned short dmg,
		const damage_kinds dmg_kind=CRUSH) {
	if ( 0>=durability )
		return 0;

	switch (sub) {
		case GLASS: return durability=0;
		case MOSS_STONE:
		case STONE:
			return (MINE==dmg_kind) ?
				durability-=2*dmg :
				durability-=dmg;
		case GREENERY: case ROSE: case HAZELNUT: case WOOD:
			return (CUT==dmg_kind) ?
				durability-=2*dmg :
				durability-=dmg;
		case SAND: case SOIL:
			return (DIG==dmg_kind) ?
				durability-=2*dmg :
				durability-=dmg;
		case A_MEAT: case H_MEAT:
			return (THRUST==dmg_kind) ?
				durability-=2*dmg :
				durability-=dmg;
		case DIFFERENT: case AIR: case SKY: case SUN_MOON: case WATER:
		case NULLSTONE:
			return durability;
		default:
			return durability-=dmg;
	}
}

void Block::SaveAttributes(FILE * const out) const {
	if ( normal ) {
		fprintf(out, "_%hd_%d", normal, sub);
		return;
	}

	fprintf(out, "_%hd_%d_%f_%d_%hd",
		normal, sub, weight, direction, durability);
	if ( NULL!=note )
		fprintf(out, "_%lu/%s", strlen(note), note);
	else
		fprintf(out, "_0/");
}

bool Block::operator==(const Block & block) const {
	//return false;
	/*if ( block.Kind()!=Kind() )
		return false;
	fprintf(stderr, "Block::==: kind ok\n");*/
	/*if ( block.Sub()!=Sub() )
		return false;
	fprintf(stderr, "Block::==: sub ok\n");*/

	/*if ( !((NULL==block.note && NULL==note) ||
			(NULL!=block.note && NULL!=note &&
			strcpy(block.note, note))) )
		return false;

	fprintf(stderr, "Block::==:note ok\n");*/
	//return true;
	return ( block.Kind()==Kind() && block.Sub()==Sub() &&
		((NULL==block.note && NULL==note) ||
			(NULL!=block.note &&
				NULL!=note &&
				strcpy(block.note, note))) );
}

Block::Block(
		const subs n,
		const short dur,
		const double w) //see blocks.h for default parameters
		:
		normal(0),
		sub(n),
		direction(NORTH),
		note(NULL),
		durability(dur)
{
	if ( w ) {
		weight=w;
		shown_weight=weight;
		return;
	}

	//weights
	switch (sub) {
		case NULLSTONE: weight=4444; break;
		case SOIL:      weight=1500; break;
		case GLASS:     weight=2500; break;
		case WOOD:      weight=999;  break;
		case IRON:      weight=7874; break;
		case GREENERY:  weight=2;    break;
		case SAND:      weight=1250; break;
		case ROSE:
		case HAZELNUT:  weight=0.1;  break;
		case MOSS_STONE:
		case STONE:     weight=2600; break;
		case A_MEAT:    weight=1;    break;
		case H_MEAT:    weight=1;    break;
		default: weight=1000;
	}
	shown_weight=weight;
}

Block::Block(char * const str) : normal(0) {
	unsigned short note_len;
	if ( 0==sscanf(str, "%*d_%*d_%d_%f_%d_%hd_%hd",
				(int *)(&sub),
				&weight,
				(int *)(&direction),
				&durability,
				&note_len) ) {
		fprintf(stderr, "Block::Block: read failure, string: %s\n",
			str);
		sub=STONE;
		weight=2600;
		direction=NORTH;
		durability=max_durability;
		note_len=0;
	}

	shown_weight=weight;
	CleanString(str);
	if ( 0==note_len ) {
		note=NULL;
		return;
	}

	note=new char[note_length];
	sscanf(str, " %s", note);
	unsigned short len;
	for (len=0; ' '==str[len]; ++len);
	for (unsigned short i=len; i<len+note_len; ++i)
		str[i]=' ';
}

void Active::SafeMove() {
	//when player is on the border of shred, moving him causes World::ReloadShreds(dir). so, cheks are neede for liquids not to push player.
	unsigned short x_focus, y_focus, z_focus;
	World * world=whereShred->GetWorld();
	if ( !(world->Focus(x_self, y_self, z_self,
				x_focus, y_focus, z_focus, direction)) &&
			((Block *)(world->playerP)!=world->
			 	GetBlock(x_focus, y_focus, z_focus) ||
			 UP==direction || DOWN==direction ) )
		world->Move(x_self, y_self, z_self, direction);
}
void Active::SafeJump() {
	SetWeight(0);
	if ( whereShred->GetWorld()->Move(x_self, y_self, z_self, UP) ) {
		SetWeight();
		SafeMove();
	} else
		SetWeight();
}

void Active::Register(Shred * const sh,
		const unsigned short x,
		const unsigned short y,
		const unsigned short z)
{
	whereShred=sh;
	if ( NULL==whereShred )
		return;

	x_self=x;
	y_self=y;
	z_self=z;
	whereShred->AddActive(this);
}

void Active::Unregister() {
	if ( NULL==whereShred )
		return;
	
	whereShred->RemActive(this);
}

Active::~Active() { Unregister(); }

void Animal::Act() {
	if ( LIQUID==whereShred->GetWorld()->Kind(x_self, y_self, z_self+1) ) {
		if ( 0>=breath )
			whereShred->GetWorld()->Damage(x_self, y_self, z_self,
					10, BREATH, false);
		else
			--breath;
		return;
	} else if ( breath<max_breath )
		++breath;

	if ( 0<satiation )
		--satiation;
	else
		whereShred->GetWorld()->Damage(x_self, y_self, z_self,
				1, HUNGER, false);
}

Inventory::Inventory(Shred * const sh,
		char * const str,
		FILE * const in)
		:
		inShred(sh)
{
	for (unsigned short i=0; i<inventory_size; ++i)
	for (unsigned short j=0; j<max_stack_size; ++j)
		inventory[i][j]=inShred->
			BlockFromFile(in, 0, 0, 0);
	fgets(str, 300, in);
}

int Dwarf::Move(const dirs dir) {
	Active::Move(dir);
	if ( this==whereShred->GetWorld()->GetPlayerP() && (
			(x_self==shred_width-1 || x_self==shred_width*2 ||
			 y_self==shred_width-1 || y_self==shred_width*2)) ) {
		whereShred->GetWorld()->ReloadShreds(dir);
		return 1;
	}
	return 0;
}

void Dwarf::Act() {
	Animal::Act();
}

before_move_return Dwarf::BeforeMove(const dirs dir) {
	if (dir==direction)
		whereShred->GetWorld()->GetAll(x_self, y_self, z_self);
	return NOTHING;
}

before_move_return Pile::BeforeMove(const dirs dir) {
	direction=dir;
	whereShred->GetWorld()->DropAll(x_self, y_self, z_self);
	return NOTHING;
}

bool Liquid::CheckWater(const dirs dir) const {
	unsigned short i_check, j_check, k_check;
	if ( (whereShred->GetWorld()->Focus(x_self, y_self, z_self,
			i_check, j_check, k_check, dir)) )
		return false;

	if ( WATER==whereShred->GetWorld()->Sub(i_check, j_check, k_check) )
		return true;

	return false;
}

void Liquid::Act() {
	switch (rand()%4) {
		case 0: SetDir(NORTH); break;
		case 1:	SetDir(EAST);  break;
		case 2: SetDir(SOUTH); break;
		case 3:	SetDir(WEST);  break;
	}
	SafeMove();
}

void Grass::Act() {
	if ( rand()%seconds_in_hour )
		return;
	
	short i=x_self, j=y_self;
	switch ( rand()%4 /* increase this if grass grows too fast */) {
		case 0: ++i; break;
		case 1: --i; break;
		case 2: ++j; break;
		case 3: --j; break;
		default: return;
	}

	World * world=whereShred->GetWorld();

	if ( world->InBounds(i, j, z_self) ) {
		if ( AIR==world->Sub(i, j, z_self) &&
				world->InBounds(i, j, z_self-1) &&
				SOIL==world->Sub(i, j, z_self-1) )
			world->Build(new Grass(), i, j, z_self);
		else if ( SOIL==world->Sub(i, j, z_self) &&
				world->InBounds(i, j, z_self+1) &&
				AIR==world->Sub(i, j, z_self+1) )
			world->Build(new Grass(), i, j, z_self+1);
	}
}

void Rabbit::Act() {
	World * world=whereShred->GetWorld();
	float attractive=0;
	float for_north=0, for_west=0;
	short x, y, z;
	for (x=x_self-7; x<=x_self+7; ++x)
	for (y=y_self-7; y<=y_self+7; ++y)
	for (z=z_self-7; z<=z_self+7; ++z)
		if ( world->InBounds(x, y, z) ) {
			switch ( world->Kind(x, y, z) ) {
				case DWARF:  if (world->DirectlyVisible(x_self, y_self, z_self, x, y, z)) attractive=-9; break;
				case RABBIT: if (world->DirectlyVisible(x_self, y_self, z_self, x, y, z)) attractive=0.8; break;
				case GRASS:  if (world->DirectlyVisible(x_self, y_self, z_self, x, y, z)) attractive=0.1; break;
				default: attractive=0;
			}
			if ( attractive ) {
				if (y!=y_self)
					for_north+=attractive/(y_self-y);
				if (x!=x_self)
					for_west +=attractive/(x_self-x);
			}
		}

	if ( abs(for_north)>1 || abs(for_west)>1 ) {
		if ( abs(for_north)>abs(for_west) ) {
			if ( for_north>0 )
				SetDir(NORTH);
			else
				SetDir(SOUTH);
		} else {
			if ( for_west>0 )
				SetDir(WEST);
			else
				SetDir(EAST);
		}
		(rand()%2) ? SafeMove() : SafeJump();
	} else if ( 0==rand()%60 ) {
		switch (rand()%4) {
			case 0: SetDir(NORTH); break;
			case 1: SetDir(SOUTH); break;
			case 2: SetDir(EAST);  break;
			case 3: SetDir(WEST);  break;
		}
		(rand()%2) ? SafeMove() : SafeJump();
	}

	if ( seconds_in_day*time_steps_in_sec/2>satiation ) {
		for (x=x_self-1; x<=x_self+1; ++x)
		for (y=y_self-1; y<=y_self+1; ++y)
		for (z=z_self-1; z<=z_self+1; ++z)
			if ( GREENERY==world->Sub(x, y, z) ) {
				world->Eat(x_self, y_self, z_self, x, y, z);
				break;
			}
	}

	Animal::Act();
}
