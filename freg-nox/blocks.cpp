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

QString & Block::FullName(QString & str) const {
	switch (sub) {
		case STAR: case SUN_MOON: case SKY:
		case AIR:        str="Air"; break;
		case WATER:      str="Ice"; break;
		case STONE:      str="Stone"; break;
		case MOSS_STONE: str="Moss stone"; break;
		case NULLSTONE:  str="Nullstone"; break;
		case GLASS:      str="Glass"; break;
		case SOIL:       str="Soil"; break;
		case HAZELNUT:   str="Hazelnut"; break;
		case WOOD:       str="Wood"; break;
		case GREENERY:   str="Leaves"; break;
		case ROSE:       str="Rose"; break;
		case A_MEAT:     str="Animal meat"; break;
		case H_MEAT:     str="Not animal meat"; break;
		default:
			fprintf(stderr,
				"Block::FullName(QString *): Block has unknown substance: %d",
				sub);
			str="Unknown block";
	}
	return str;
}

int Block::Damage(
		const ushort dmg,
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
		case NULLSTONE: case STAR:
			return durability;
		default: return durability-=dmg;
	}
}

bool Block::operator==(const Block & block) const {
	return ( block.Kind()==Kind() &&
			block.Sub()==Sub() &&
			block.note==note );
}

Block::Block(
		const int n,
		const short dur,
		const float w) //see blocks.h for default parameters
		:
		normal(false),
		sub(n),
		direction(NORTH),
		note(""),
		durability(dur)
{
	if ( w ) {
		weight=w;
		shown_weight=weight;
		return;
	}

	switch (sub) { //weights
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
		case AIR:       weight=0;    break;
		default: weight=1000;
	}
	shown_weight=weight;
}

Block::Block(QDataStream & str, const int sub_)
		:
		normal(false),
		sub(sub_)
{
	str >> weight >>
		direction >>
		durability >> note;
	shown_weight=weight;
}

usage_types Clock::Use() {
	QString temp;
	world->EmitNotify("Time is "+
		temp.setNum(world->TimeOfDay()/60)+
		((world->TimeOfDay()%60 < 10) ? ":0" : ":")+
		temp.setNum(world->TimeOfDay()%60)+'.');
	return NO;
}

int Active::Move(const int dir) {
	switch ( dir ) {
		case NORTH: --y_self; break;
		case SOUTH: ++y_self; break;
		case EAST:  ++x_self; break;
		case WEST:  --x_self; break;
		case UP:    ++z_self; break;
		case DOWN:  --z_self; break;
		default:
			fprintf(stderr,
				"Active::Move(dirs): unlisted dir: %d\n",
				dir);
			return 0;
	}
	if ( GetWorld()->GetShred(x_self, y_self)!=whereShred ) {
		whereShred->RemActive(this);
		whereShred=GetWorld()->GetShred(x_self, y_self);
		whereShred->AddActive(this);	
	}
	emit Moved(dir);
	return 0;
}

World * Active::GetWorld() const {
	return whereShred->GetWorld();
}

void Active::Register(Shred * const sh,
		const ushort x,
		const ushort y,
		const ushort z)
{
	whereShred=sh;
	if ( !whereShred )
		return;

	x_self=x;
	y_self=y;
	z_self=z;
	whereShred->AddActive(this);
}

void Active::Unregister() {
	if ( !whereShred )
		return;
	
	whereShred->RemActive(this);
}

Active::~Active() {
       	Unregister();
	emit Destroyed();
}

bool Animal::Act() {
	static ushort timeStep=0;
	if ( time_steps_in_sec>timeStep ) {
		++timeStep;
		return false;
	}

	timeStep=0;
	World * const world=GetWorld();
	if ( LIQUID==world->Kind(x_self, y_self, z_self+1) ) {
		if ( 0>=breath )
			return world->Damage(x_self, y_self, z_self,
					10, BREATH);
		--breath;
		return false;
	} else if ( breath<max_breath )
		++breath;

	if ( 0>=satiation )
		return world->Damage(x_self, y_self, z_self,
			1, HUNGER);

	--satiation;
	return false;
}

Inventory::Inventory(Shred * const sh,
		QDataStream & str)
		:
		inShred(sh)
{
	for (ushort i=0; i<inventory_size; ++i) {
		str >> inventory_num[i];
		inventory[i]=( inventory_num[i] ) ?
			inShred->BlockFromFile(str, 0, 0, 0) :
			0;
	}
}

bool Dwarf::Act() {
	return Animal::Act();
}

Block * Dwarf::DropAfterDamage() const {
	return whereShred->NewNormal(H_MEAT);
}

before_move_return Dwarf::BeforeMove(const int dir) {
	if ( dir==direction )
		GetWorld()->GetAll(x_self, y_self, z_self);
	return NOTHING;
}

before_move_return Pile::BeforeMove(const int dir) {
	direction=dir;
	GetWorld()->DropAll(x_self, y_self, z_self);
	return NOTHING;
}

bool Liquid::CheckWater(const int dir) const {
	ushort i_check, j_check, k_check;
	World * const world=GetWorld();
	if ( (world->Focus(x_self, y_self, z_self,
			i_check, j_check, k_check, dir)) )
		return false;

	if ( WATER==world->Sub(i_check, j_check, k_check) )
		return true;

	return false;
}

bool Liquid::Act() {
	World * const world=GetWorld();
	if ( !(rand()%10) &&
			!CheckWater(DOWN)  && !CheckWater(UP) &&
			!CheckWater(NORTH) && !CheckWater(SOUTH) &&
			!CheckWater(EAST)  && !CheckWater(WEST) )
		return world->Damage(x_self, y_self, z_self,
			max_durability, HEAT);

	int dir;
	switch ( rand()%20 ) {
		case 0: dir=NORTH; break;
		case 1: dir=EAST;  break;
		case 2: dir=SOUTH; break;
		case 3: dir=WEST;  break;
		default: return false;
	}
	world->Move(x_self, y_self, z_self, dir);
	return false;
}

bool Grass::Act() {
	if ( rand()%seconds_in_hour )
		return false;
	
	short i=x_self, j=y_self;
	switch ( rand()%4 /* increase this if grass grows too fast */) {
		case 0: ++i; break;
		case 1: --i; break;
		case 2: ++j; break;
		case 3: --j; break;
		default: return false;
	}

	World * const world=GetWorld();

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
	return false;
}

bool Bush::Act() {
	if ( 0==rand()%seconds_in_hour ) {
		Get(whereShred->NewNormal(HAZELNUT));
	}
	return false;
}

Block * Bush::DropAfterDamage() const {
	return whereShred->NewNormal(WOOD);
}

float Rabbit::Attractive(int kind) const {
	switch ( kind ) {
		case DWARF: return -9;
		case GRASS: return 0.1;
		case RABBIT: return 0.8;
		default: return 0;
	}
}

bool Rabbit::Act() {
	World * const world=GetWorld();
	float for_north=0, for_west=0;
	ushort x, y, z;
	int kind;
	for (x=x_self-6; x<=x_self+6; ++x)
	for (y=y_self-6; y<=y_self+6; ++y)
	for (z=z_self-6; z<=z_self+6; ++z)
		if ( world->InBounds(x, y, z) ) {
			kind=world->Kind(x, y, z);
			if ( (GRASS==kind || RABBIT==kind || DWARF==kind) && 
					world->DirectlyVisible(x_self, y_self, z_self, x, y, z) ) {
				if ( y!=y_self )
					for_north+=Attractive(kind)/(y_self-y);
				if ( x!=x_self )
					for_west +=Attractive(kind)/(x_self-x);
			}
		}

	if ( abs(for_north)>1 || abs(for_west)>1 ) {
		SetDir( ( abs(for_north)>abs(for_west) ) ?
			( ( for_north>0 ) ? NORTH : SOUTH ) :
			( ( for_west >0 ) ? WEST  : EAST  ) );
		if ( rand()%2 )
			world->Move(x_self, y_self, z_self, direction);
		else
			world->Jump(x_self, y_self, z_self);
	} else switch ( rand()%60 ) {
			case 0: world->Move(x_self, y_self, z_self, NORTH); break;
			case 1: world->Move(x_self, y_self, z_self, SOUTH); break;
			case 2: world->Move(x_self, y_self, z_self, EAST);  break;
			case 3: world->Move(x_self, y_self, z_self, WEST);  break;
		}

	if ( seconds_in_day*time_steps_in_sec/2>satiation ) {
		for (x=x_self-1; x<=x_self+1; ++x)
		for (y=y_self-1; y<=y_self+1; ++y)
		for (z=z_self-1; z<=z_self+1; ++z)
			if ( GREENERY==world->Sub(x, y, z) ) {
				world->Eat(x_self, y_self, z_self, x, y, z);
				return Animal::Act();
			}
	}

	return Animal::Act();
}

Block * Rabbit::DropAfterDamage() const { 
	return whereShred->NewNormal(A_MEAT);
}
