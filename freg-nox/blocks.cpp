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
		const int dmg_kind=CRUSH) {
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
		case DIFFERENT:
			if (TIME==dmg_kind)
				return 0;
			//no break;
		case AIR: case SKY: case SUN_MOON: case WATER:
		case NULLSTONE: case STAR:
			return durability;
		default: return durability-=dmg;
	}
}

bool Block::operator==(const Block & block) const {
	return ( block.Kind()==Kind() &&
			block.Sub()==Sub() &&
			block.Durability()==Durability() &&
			block.note==note );
}

float Block::TrueWeight() const {
	switch ( Sub() ) {
		case NULLSTONE: return 4444;
		case SOIL:      return 1500;
		case GLASS:     return 2500;
		case WOOD:      return 999;
		case IRON:      return 7874;
		case GREENERY:  return 2;
		case SAND:      return 1250;
		case ROSE:
		case HAZELNUT:  return 0.1;
		case MOSS_STONE:
		case STONE:     return 2600;
		case A_MEAT:    return 1;
		case H_MEAT:    return 1;
		case AIR:       return 0;
		default: return 1000;
	}
}

Block::Block(QDataStream & str, const int sub_)
		:
		normal(false),
		sub(sub_)
{
	str >> nullWeight >>
		direction >>
		durability >> note;
}

usage_types Clock::Use() {
	world->EmitNotify(QString("Time is %1%2%3.").
		arg(world->TimeOfDay()/60).
		arg((world->TimeOfDay()%60 < 10) ? ":0" : ":").
		arg(world->TimeOfDay()%60));
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
				"Active::Move: unlisted dir: %d\n",
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

int Inventory::MiniCraft(const ushort num) {
	const ushort size=inventory[num].size();
	if ( !size )
		return 1; //empty
	craft_item item={
		size,
		GetInvKind(num),
		GetInvSub(num)
	};
	craft_item result;

	if ( inShred->GetWorld()->MiniCraft(item, result) ) {
		while ( !inventory[num].isEmpty() ) {
			Block * const todrop=Drop(num);
			if ( !todrop->Normal() )
				delete todrop;
		}
		for (ushort i=0; i<result.num; ++i)
			Get(inShred->CraftBlock(result.kind, result.sub));
		return 0; //success
	}
	return 2; //no such recipe
}

Inventory::Inventory(
		Shred * const sh,
		QDataStream & str,
		const ushort sz)
		:
		size(sz),
		inShred(sh)
{
	inventory=new QStack<Block *>[Size()];
	for (ushort i=0; i<Size(); ++i) {
		quint8 num;
		str >> num;
		while ( num-- )
			inventory[i].push(inShred->BlockFromFile(str, 0, 0, 0));
	}
}

bool Dwarf::Act() { return Animal::Act(); }

Block * Dwarf::DropAfterDamage() const {
	return whereShred->NewNormal(H_MEAT);
}

before_move_return Dwarf::BeforeMove(const int dir) {
	if ( dir==direction )
		GetWorld()->GetAll(x_self, y_self, z_self);
	return NOTHING;
}

bool Pile::Act() {
	return ( ifToDestroy ) ?
		GetWorld()->Damage(x_self, y_self, z_self, 0, TIME) :
		false;
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
			case 0:
				SetDir(NORTH);
				world->Move(x_self, y_self, z_self, NORTH);
			break;
			case 1:
				SetDir(SOUTH);
				world->Move(x_self, y_self, z_self, SOUTH);
			break;
			case 2:
				SetDir(EAST);
				world->Move(x_self, y_self, z_self, EAST);
			break;
			case 3:
				SetDir(WEST);
				world->Move(x_self, y_self, z_self, WEST);
			break;
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
