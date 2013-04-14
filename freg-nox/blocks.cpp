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

#include <QDataStream>
#include "blocks.h"
#include "world.h"
#include "Shred.h"
#include "CraftManager.h"
#include "BlockManager.h"

Block * Plate::DropAfterDamage() const { return block_manager.NewBlock(PLATE, Sub()); }

Block * Ladder::DropAfterDamage() const { return block_manager.NewBlock(LADDER, Sub()); }

//Block * Clock::DropAfterDamage() const { return block_manager.NewBlock(CLOCK, Sub()); }

/*usage_types Clock::Use() {
	world->EmitNotify(QString("Time is %1%2%3.").
		arg(world->TimeOfDay()/60).
		arg((world->TimeOfDay()%60 < 10) ? ":0" : ":").
		arg(world->TimeOfDay()%60));
	return NO;
}*/

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
	if ( DOWN==dir ) {
		falling=true;
		++fall_height;
	} else {
		if ( GetWorld()->GetShred(x_self, y_self)!=whereShred ) {
			whereShred->RemActive(this);
			whereShred=GetWorld()->GetShred(x_self, y_self);
			whereShred->AddActive(this);
		}
		fall_height=0;
	}
	emit Moved(dir);
	return 0;
}

void Active::Act() {
	if ( !falling && fall_height ) {
		if ( fall_height > safe_fall_height ) {
			const ushort dmg=(fall_height - safe_fall_height)*10;
			fall_height=0;
			GetWorld()->Damage(x_self, y_self, z_self-1, dmg);
			if ( GetWorld()->Damage(x_self, y_self, z_self, dmg) ) {
				return;
			} else {
				emit Updated();
			}
		} else {
			fall_height=0;
		}
	}
}

World * Active::GetWorld() const {
	return whereShred->GetWorld();
}

void Active::Register(
		Shred * const sh,
		const ushort x,
		const ushort y,
		const ushort z)
{
	if ( !whereShred ) {
		whereShred=sh;
		x_self=x;
		y_self=y;
		z_self=z;
		whereShred->AddActive(this);
	}
}

void Active::Unregister() {
	if ( whereShred ) {
		whereShred->RemActive(this);
		whereShred=0;
	}
}

Active::Active(
		QDataStream & str,
		const int sub,
		const quint8 transp) //see default in blocks.h
		:
		Block(str, sub, transp),
		whereShred(0)
{
	str >> timeStep >> fall_height >> falling;
}

Active::~Active() {
       	Unregister();
	emit Destroyed();
}

void Animal::Act() {
	World * const world=GetWorld();
	if ( World::TimeStepsInSec() > timeStep ) {
		++timeStep;
		return;
	}
	timeStep=0;
	if (
			world->InBounds(x_self, y_self, z_self+1) &&
				AIR!=world->Sub(x_self, y_self, z_self+1) &&
			world->InBounds(x_self, y_self, z_self-1) &&
				AIR!=world->Sub(x_self, y_self, z_self-1) &&
			world->InBounds(x_self+1, y_self, z_self) &&
				AIR!=world->Sub(x_self+1, y_self, z_self) &&
			world->InBounds(x_self-1, y_self, z_self) &&
				AIR!=world->Sub(x_self-1, y_self, z_self) &&
			world->InBounds(x_self, y_self+1, z_self) &&
				AIR!=world->Sub(x_self, y_self+1, z_self) &&
			world->InBounds(x_self, y_self-1, z_self) &&
				AIR!=world->Sub(x_self, y_self-1, z_self) )
	{
		if ( breath <= 0 ) {
			if ( world->Damage(x_self, y_self, z_self, 10, BREATH) )
				return;
		} else
			--breath;
	} else if ( breath < MAX_BREATH )
		++breath;

	if ( satiation <= 0 ) {
		if ( world->Damage(x_self, y_self, z_self, 5, HUNGER) )
			return;
	} else
		--satiation;
	if ( durability < MAX_DURABILITY )
		++durability;
	emit Updated();
	Active::Act();
}

Animal::Animal(
		QDataStream & str,
		const int sub)
		:
		Active(str, sub, NONSTANDARD)
{
	str >> breath >> satiation;
}

float Dwarf::TrueWeight() const {
	World * const world=GetWorld();
	return (
			(world->InBounds(x_self+1, y_self) &&
				world->GetBlock(x_self+1, y_self, z_self)->Catchable()) ||
			(world->InBounds(x_self-1, y_self) &&
				world->GetBlock(x_self-1, y_self, z_self)->Catchable()) ||
			(world->InBounds(x_self, y_self+1) &&
				world->GetBlock(x_self, y_self+1, z_self)->Catchable()) ||
			(world->InBounds(x_self, y_self-1) &&
				world->GetBlock(x_self, y_self-1, z_self)->Catchable()) ) ?
		0 : InvWeightAll()+60;
}

void Dwarf::Act() { Animal::Act(); }

Block * Dwarf::DropAfterDamage() const {
	return block_manager.NormalBlock(H_MEAT);
}

before_move_return Dwarf::BeforeMove(const int dir) {
	if ( dir==direction )
		GetWorld()->GetAll(x_self, y_self, z_self);
	return NOTHING;
}

Block * Chest::DropAfterDamage() const { return block_manager.NewBlock(CHEST, sub); }

before_move_return Pile::BeforeMove(const int dir) {
	ushort x_to, y_to, z_to;
	World * const world=GetWorld();
	if ( world->Focus(x_self, y_self, z_self, x_to, y_to, z_to, dir) )
		return NOTHING;
	Inventory * const inv=world->HasInventory(x_to, y_to, z_to);
	if ( inv )
		inv->GetAll(this);
	if ( IsEmpty() )
		ifToDestroy=true;
	return NOTHING;
}

void Pile::Act() {
	if ( ifToDestroy )
		GetWorld()->Damage(x_self, y_self, z_self, 0, TIME);
}

Pile::Pile(
		QDataStream & str,
		const int sub)
	:
		Active(str, sub, NONSTANDARD),
		Inventory(str)
{
	str >> ifToDestroy;
}

bool Liquid::CheckWater() const {
	World * const world=GetWorld();
	return ( WATER==world->Sub(x_self, y_self, z_self-1) ||
			WATER==world->Sub(x_self, y_self, z_self+1));
}

void Liquid::Act() {
	World * const world=GetWorld();
	//IDEA: turn off water drying up in ocean
	if ( WATER==Sub() ) {
		if ( CheckWater() ) {
			Restore();
		} else if ( world->Damage(x_self, y_self, z_self,
				1, HEAT) ) {
			return;
		}
	}
	switch ( rand()%20 ) {
		case 0: world->Move(x_self, y_self, z_self, NORTH); return;
		case 1: world->Move(x_self, y_self, z_self, EAST);  return;
		case 2: world->Move(x_self, y_self, z_self, SOUTH); return;
		case 3: world->Move(x_self, y_self, z_self, WEST);  return;
		default: return;
	}
}

void Grass::Act() {
	short i=x_self, j=y_self;
	switch ( rand()%(SECONDS_IN_HOUR*20) /* increase this if grass grows too fast */ ) {
		case 0: ++i; break;
		case 1: --i; break;
		case 2: ++j; break;
		case 3: --j; break;
		default: return;
	}

	World * const world=GetWorld();
	if ( world->InBounds(i, j, z_self) && world->Enlightened(i, j, z_self) ) {
		if ( AIR==world->Sub(i, j, z_self) &&
				world->InBounds(i, j, z_self-1) &&
				SOIL==world->Sub(i, j, z_self-1) )
			world->Build(block_manager.NewBlock(GRASS, Sub()), i, j, z_self);
		else if ( SOIL==world->Sub(i, j, z_self) &&
				world->InBounds(i, j, z_self+1) &&
				AIR==world->Sub(i, j, z_self+1) )
			world->Build(block_manager.NewBlock(GRASS, Sub()), i, j, z_self+1);
	}
}

float Rabbit::Attractive(int kind) const {
	switch ( kind ) {
		case DWARF: return -9;
		case GRASS: return 0.1f;
		case RABBIT: return 0.8f;
		default: return 0;
	}
}

void Rabbit::Act() {
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

	if ( SECONDS_IN_DAY/2 > satiation ) {
		for (x=x_self-1; x<=x_self+1; ++x)
		for (y=y_self-1; y<=y_self+1; ++y)
		for (z=z_self-1; z<=z_self+1; ++z)
			if ( GREENERY==world->Sub(x, y, z) ) {
				world->Eat(x_self, y_self, z_self, x, y, z);
				Animal::Act();
				return;
			}
	}
	Animal::Act();
}

Block * Rabbit::DropAfterDamage() const {
	return block_manager.NormalBlock(A_MEAT);
}

Block * Workbench::DropAfterDamage() const { return block_manager.NewBlock(WORKBENCH, Sub()); }

void Workbench::Craft() {
	while ( Number(0) ) { //remove previous product
		Block * const to_push=ShowBlock(0);
		Pull(0);
		block_manager.DeleteBlock(to_push);
	}
	craft_recipe recipe;
	for (ushort i=Start(); i<Size(); ++i)
		if ( Number(i) ) {
			craft_item * item=new craft_item;
			item->num=Number(i);
			item->kind=GetInvKind(i);
			item->sub=GetInvSub(i);
			recipe.append(item);
		}
	craft_item result;
	if ( craft_manager.Craft(recipe, result) ) {
		for (ushort i=0; i<result.num; ++i) {
			GetExact(block_manager.NewBlock(result.kind, result.sub), 0);
		}
	}
	for (ushort i=0; i<recipe.size(); ++i) {
		delete recipe.at(i);
	}
}

int Workbench::Drop(const ushort num, Inventory * const inv_to) {
	if ( !inv_to )
		return 1;
	if ( num>=Size() )
		return 6;
	if ( !Number(num) )
		return 6;
	if ( num==0 ) {
		while ( Number(0) ) {
			if ( !inv_to->Get(ShowBlock(0)) )
				return 2;
			Pull(0);
		}
		for (ushort i=Start(); i<Size(); ++i)
			while ( Number(i) ) {
				Block * const to_pull=ShowBlock(i);
				Pull(i);
				block_manager.DeleteBlock(to_pull);
			}
	} else {
		if ( !inv_to->Get(ShowBlock(num)) )
			return 2;
		Pull(num);
		Craft();
	}
	return 0;
}

Block * Door::DropAfterDamage() const { return block_manager.NewBlock(DOOR, Sub()); }

int Door::BeforePush(const int dir) {
	if ( locked || shifted || dir==World::Anti(GetDir()) )
		return NO_ACTION;
	movable=MOVABLE;
	NullWeight(true);
	if ( GetWorld()->Move(x_self, y_self, z_self, GetDir()) )
		shifted=true;
	movable=NOT_MOVABLE;
	NullWeight(false);
	return NO_ACTION;
}

void Door::Act() {
	if ( shifted ) {
		World * const world=GetWorld();
		ushort x, y, z;
		world->Focus(x_self, y_self, z_self, x, y, z,
			World::Anti(GetDir()));
		if ( AIR==world->Sub(x, y, z) ) {
			movable=MOVABLE;
			NullWeight(true);
			world->Move(x_self, y_self, z_self,
				World::Anti(GetDir()));
			shifted=false;
			movable=NOT_MOVABLE;
			NullWeight(false);
		}
	}
}

Door::Door(
		QDataStream & str,
		const int sub)
		:
		Active(str, sub, NONSTANDARD),
		movable(NOT_MOVABLE)
{
	str >> shifted >> locked;
}
