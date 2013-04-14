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

//Animal::
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

	Animal::Animal(QDataStream & str, const int sub) :
			Active(str, sub, NONSTANDARD)
	{
		str >> breath >> satiation;
	}

//Dwarf::
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

//Chest::
	Block * Chest::DropAfterDamage() const { return block_manager.NewBlock(CHEST, sub); }

//Pile::
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

	Pile::Pile(QDataStream & str, const int sub) :
			Active(str, sub, NONSTANDARD),
			Inventory(str)
	{
		str >> ifToDestroy;
	}

//Liquid::
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

//Grass::
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

//Rabbit::
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

//Workbench::
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

//Door::
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

	Door::Door(QDataStream & str, const int sub) :
			Active(str, sub, NONSTANDARD),
			movable(NOT_MOVABLE)
	{
		str >> shifted >> locked;
	}

//Weapon::
	QString & Weapon::FullName(QString & str) const {
		switch ( Sub() ) {
			case STONE: return str="Pebble";
			case IRON:  return str="Spike";
			case WOOD:  return str="Stick";
			default:
				fprintf(stderr,
					"Weapon::FullName: unlisted sub: %d\n",
					Sub());
			return str="Some weapon";
		}
	}

	int Weapon::Kind() const { return WEAPON; }

	ushort Weapon::DamageLevel() const {
		switch ( Sub() ) {
			case WOOD: return 4;
			case IRON: return 6;
			case STONE: return 5;
			default:
				fprintf(stderr,
					"Weapon::DamageLevel: unlisted sub: %d\n",
					Sub());
				return 1;
		}
	}

	int Weapon::BeforePush(const int) {
		return ( IRON==Sub() ) ? DAMAGE : NO_ACTION;
	}

	float Weapon::TrueWeight() const { return 1; }

	bool Weapon::IsWeapon() const { return true; }

	bool Weapon::CanBeOut() const { return false; }

	Weapon::Weapon(const int sub) :
			Block(sub, NONSTANDARD)
	{}

	Weapon::Weapon(QDataStream & str, const int sub) :
			Block(str, sub, NONSTANDARD)
	{}

//Block::
	QString & Block::FullName(QString & str) const {
		switch (sub) {
			case STAR: case SUN_MOON: case SKY:
			case AIR:        return str="Air";
			case WATER:      return str="Ice";
			case STONE:      return str="Stone";
			case MOSS_STONE: return str="Moss stone";
			case NULLSTONE:  return str="Nullstone";
			case GLASS:      return str="Glass";
			case SOIL:       return str="Soil";
			case HAZELNUT:   return str="Hazelnut";
			case WOOD:       return str="Wood";
			case GREENERY:   return str="Leaves";
			case ROSE:       return str="Rose";
			case A_MEAT:     return str="Animal meat";
			case H_MEAT:     return str="Not animal meat";
			case IRON:       return str="Iron block";
			default:
				fprintf(stderr,
					"Block::FullName: unlisted sub: %d.\n",
					sub);
				return str="Unknown block";
		}
	}

	void Block::SetTransparency(const quint8 transp) {
		if ( UNDEF==transp )
			switch ( sub ) {
				case AIR: transparent=INVISIBLE; break;
				case WATER: case GREENERY:
				case GLASS: transparent=BLOCK_TRANSPARENT; break;
				default: transparent=BLOCK_OPAQUE;
			}
		else
			transparent=transp;
	}

	int Block::Damage(
			const ushort dmg,
			const int dmg_kind=CRUSH)
	{
		switch ( sub ) {
			case DIFFERENT:
				if ( TIME==dmg_kind )
					return 0;
				//no break, only time damages DIFFERENT
			case NULLSTONE:
			case STAR:
			case AIR:
			case SKY:
			case SUN_MOON:
			case WATER:
				return durability;
			case MOSS_STONE:
			case STONE:
				return ( MINE==dmg_kind ) ?
					durability-=2*dmg :
					durability-=dmg;
			case GLASS:
				return durability=0;
			case GREENERY:
			case ROSE:
			case HAZELNUT:
			case WOOD:
				return (CUT==dmg_kind) ?
					durability-=2*dmg :
					durability-=dmg;
			case SAND:
			case SOIL:
				return (DIG==dmg_kind) ?
					durability-=2*dmg :
					durability-=dmg;
			case A_MEAT:
			case H_MEAT:
				return (THRUST==dmg_kind) ?
					durability-=2*dmg :
					durability-=dmg;
			default:
				return durability-=dmg;
		}
	}

	Block * Block::DropAfterDamage() const {
		return ( BLOCK==Kind() && GLASS!=sub ) ?
			block_manager.NormalBlock(sub) : 0;
	}

	void Block::SetInMemoryChunk(bool in) { inMemoryChunk=in; }

	bool Block::InMemoryChunk() const { return inMemoryChunk; }

	int  Block::Kind() const { return BLOCK; }

	bool Block::Catchable() const { return true; }

	int  Block::Movable() const { return ( AIR==Sub() ) ? ENVIRONMENT : NOT_MOVABLE; }

	int  Block::BeforePush(const int) { return NO_ACTION; }

	int  Block::Move(const int) { return 0; }

	bool Block::Armour() const { return false; }

	bool Block::IsWeapon() const { return false; }

	bool Block::Carving() const { return false; }

	int  Block::DamageKind() const { return CRUSH; }

	void Block::Restore() { durability=MAX_DURABILITY; }

	void Block::NullWeight(const bool null) { nullWeight=null; }

	void Block::SetDir(const int dir) { direction=dir; }

	int  Block::GetDir() const { return direction; }

	int  Block::Sub() const { return sub; }

	int  Block::Transparent() const { return transparent; }

	short Block::Durability() const { return durability; }

	float Block::Weight() const { return nullWeight ? 0 : TrueWeight(); }

	uchar Block::LightRadius() const { return 0; }

	QString & Block::GetNote(QString & str) const { return str=note; }

	ushort Block::DamageLevel() const { return 1; }

	before_move_return Block::BeforeMove(const int) { return NOTHING; }

	usage_types Block::Use() { return NO; }

	Inventory * Block::HasInventory() { return 0; }

	Animal * Block::IsAnimal() { return 0; }

	Active * Block::ActiveBlock() { return 0; }

	int Block::Temperature() const {
		switch (sub) {
			case WATER: return -100;
			default: return 0;
		}
	}

	bool Block::Inscribable() const {
		//TODO: prevent inscribing living creatures
		//IDEA: add names to living creatures (maybe taiming)
		return !(
			sub==AIR       ||
			sub==NULLSTONE ||
			sub==A_MEAT    ||
			sub==GREENERY  );
	}

	bool Block::CanBeOut() const {
		switch (sub) {
			case HAZELNUT: return false;
			default: return true;
		}
	}

	bool Block::Inscribe(const QString & str) {
		if ( Inscribable() ) {
			note=str;
			return true;
		}
		return false;
	}

	void Block::SaveAttributes(QDataStream &) const {}

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
			case HAZELNUT:  return 0.1f;
			case MOSS_STONE:
			case STONE:     return 2600;
			case A_MEAT:    return 1;
			case H_MEAT:    return 1;
			case AIR:       return 0;
			default: return 1000;
		}
	}

	void Block::SaveToFile(QDataStream & out) const {
		const bool normal=(this==block_manager.NormalBlock(Sub()));
		out << (quint16)Kind() << sub << normal;

		if ( normal ) {
			return;
		}

		out << nullWeight
			<< direction
			<< durability
			<< note;
		SaveAttributes(out);
	}

	Block::Block(const int sb, const quint8 transp) :
			inMemoryChunk(true),
			sub(sb),
			nullWeight(false),
			direction(UP),
			note(""),
			durability(MAX_DURABILITY)
	{
		SetTransparency(transp);
	}

	Block::Block(
			QDataStream & str,
			const int sub_,
			const quint8 transp)
			:
			inMemoryChunk(true),
			sub(sub_)
	{
		SetTransparency(transp);
		str >> nullWeight >>
			direction >>
			durability >> note;
	}

	Block::~Block() {}

//Bush::
	QString & Bush::FullName(QString & str) const { return str="Bush"; }

	int Bush::Kind() const { return BUSH; }

	int Bush::Sub() const { return Block::Sub(); }

	usage_types Bush::Use() { return Inventory::Use(); }

	Inventory * Bush::HasInventory() { return Inventory::HasInventory(); }

	int Bush::Movable() const { return NOT_MOVABLE; }

	float Bush::TrueWeight() const { return InvWeightAll()+20; }

	void Bush::Act() {
		if ( 0==rand()%(SECONDS_IN_HOUR*4) )
			Get(block_manager.NormalBlock(HAZELNUT));
	}

	Block * Bush::DropAfterDamage() const {
		return block_manager.NormalBlock(WOOD);
	}

	void Bush::SaveAttributes(QDataStream & out) const {
		Active::SaveAttributes(out);
		Inventory::SaveAttributes(out);
	}

	Bush::Bush(const int sub) :
			Active(sub),
			Inventory(bush_size)
	{}

	Bush::Bush(QDataStream & str, const int sub) :
			Active(str, sub),
			Inventory(str, bush_size)
	{}

//Clock::
	Block * Clock::DropAfterDamage() const { return block_manager.NewBlock(CLOCK, Sub()); }

	usage_types Clock::Use() {
		//TODO: restore clock work
		/*world->EmitNotify(QString("Time is %1%2%3.").
			arg(world->TimeOfDay()/60).
			arg((world->TimeOfDay()%60 < 10) ? ":0" : ":").
			arg(world->TimeOfDay()%60));*/
		return NO;
	}

	int Clock::Kind() const { return CLOCK; }

	QString & Clock::FullName(QString & str) const {
		switch ( sub ) {
			case IRON: return str="Iron clock";
			default:
				fprintf(stderr,
					"Clock::FullName: unlisted sub: %d\n",
					sub);
				return str="Strange clock";
		}
	}

	int Clock::BeforePush(const int) {
		Use();
		return NO_ACTION;
	}

	float Clock::TrueWeight() const { return 0.1f; }

	Clock::Clock(const int sub) :
			Block(sub, NONSTANDARD)
	{}

	Clock::Clock (QDataStream & str, const int sub) :
			Block(str, sub, NONSTANDARD)
	{}

//Active::
	QString & Active::FullName(QString & str) const {
		switch (sub) {
			case SAND: return str="Sand";
			default:
				fprintf(stderr,
					"Active:FullName(QString&): Unlisted sub: %d\n",
					sub);
				return str="Unkown active block";
		}
	}

	int Active::Kind() const { return ACTIVE; }

	Active * Active::ActiveBlock() { return this; }

	void Active::SetNotFalling() { falling=false; }
	bool Active::IsFalling() const { return falling; }

	ushort Active::X() const { return x_self; }
	ushort Active::Y() const { return y_self; }
	ushort Active::Z() const { return z_self; }

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

	World * Active::GetWorld() const { return whereShred->GetWorld(); }

	int Active::Movable() const { return MOVABLE; }

	bool Active::ShouldFall() const { return true; }

	before_move_return Active::BeforeMove(const int) { return NOTHING; }

	int Active::Damage(const ushort dmg, const int dmg_kind) {
		const int last_dur=durability;
		const int new_dur=Block::Damage(dmg, dmg_kind);
		if ( last_dur != new_dur )
			emit Updated();
		return new_dur;
	}

	void Active::ReloadToNorth() { y_self+=SHRED_WIDTH; }
	void Active::ReloadToSouth() { y_self-=SHRED_WIDTH; }
	void Active::ReloadToWest()  { x_self+=SHRED_WIDTH; }
	void Active::ReloadToEast()  { x_self-=SHRED_WIDTH; }

	void Active::SaveAttributes(QDataStream & out) const {
		out << timeStep << fall_height << falling;
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

	Active::Active(const int sub, const quint8 transp) :
			Block(sub, transp),
			fall_height(0),
			falling(false),
			timeStep(0),
			whereShred(0)
	{}

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

//Inventory::
	bool Inventory::Access() const { return true; }

	ushort Inventory::Start() const { return 0; }

	Inventory * Inventory::HasInventory() { return this; }

	ushort Inventory::Size() const { return size; }

	int Inventory::Drop(const ushort num, Inventory * const inv_to) {
		if ( !inv_to )
			return 1;
		if ( num>=Size() )
			return 6;
		if ( inventory[num].isEmpty() )
			return 6;
		if ( !inv_to->Get(inventory[num].top()) )
			return 2;
		Pull(num);
		return 0;
	}

	int Inventory::GetAll(Inventory * const from) {
		if ( !from )
			return 1;
		if ( !from->Access() )
			return 2;

		for (ushort i=0; i<from->Size(); ++i)
			while ( from->Number(i) )
				if ( from->Drop(i, this) )
					return 3;
		return 0;
	}

	usage_types Inventory::Use() { return OPEN; }

	void Inventory::Pull(const ushort num) {
		if ( !inventory[num].isEmpty() )
			inventory[num].pop();
	}

	void Inventory::SaveAttributes(QDataStream & out) const {
		for (ushort i=0; i<Size(); ++i) {
			out << Number(i);
			for (ushort j=0; j<Number(i); ++j)
				inventory[i].top()->SaveToFile(out);
		}
	}

	bool Inventory::Get(Block * const block) {
		if ( !block )
			return true;

		for (ushort i=Start(); i<Size(); ++i)
			if ( GetExact(block, i) )
				return true;
		return false;
	}

	bool Inventory::GetExact(Block * const block, const ushort num) {
		if ( inventory[num].isEmpty() ||
				( *block==*inventory[num].top() &&
				Number(num)<max_stack_size ) )
		{
			inventory[num].push(block);
			return true;
		}
		return false;
	}

	int Inventory::InscribeInv(const ushort num, const QString & str) {
		const int number=Number(num);
		if ( !number )
			return 0;
		if ( !inventory[num].top()->Inscribable() )
			return 1;
		const int sub=inventory[num].top()->Sub();
		if ( inventory[num].top()==block_manager.NormalBlock(sub) ) {
			for (ushort i=0; i<number; ++i)
				inventory[num].replace(i, block_manager.NormalBlock(sub));
		}
		for (ushort i=0; i<number; ++i)
			inventory[num].at(i)->Inscribe(str);
		return 0;
	}

	QString & Inventory::InvFullName(QString & str, const ushort i) const {
		return str=( inventory[i].isEmpty() ) ? "" :
			inventory[i].top()->FullName(str);
	}

	QString & Inventory::NumStr(QString & str, const ushort i) const {
		return str=( 1<Number(i) ) ?
			QString(" (%1x)").arg(Number(i)) :
			"";
	}

	float Inventory::GetInvWeight(const ushort i) const {
		return ( inventory[i].isEmpty() ) ? 0 :
			inventory[i].top()->Weight()*Number(i);
	}

	int Inventory::GetInvSub(const ushort i) const {
		return ( inventory[i].isEmpty() ) ? AIR :
			inventory[i].top()->Sub();
	}

	int Inventory::GetInvKind(const ushort i) const {
		return ( inventory[i].isEmpty() ) ? BLOCK :
			inventory[i].top()->Kind();
	}

	QString & Inventory::GetInvNote(QString & str, const ushort num) const {
		return str=inventory[num].top()->GetNote(str);
	}

	float Inventory::InvWeightAll() const {
		float sum=0;
		for (ushort i=0; i<Size(); ++i)
			sum+=GetInvWeight(i);
		return sum;
	}

	Block * Inventory::ShowBlock(const ushort num) const {
		return ( num>Size() || inventory[num].isEmpty() ) ?
			0 : inventory[num].top();
	}

	bool Inventory::HasRoom() {
		for (ushort i=Start(); i<Size(); ++i) {
			if ( inventory[i].isEmpty() ) {
				return true;
			}
		}
		return false;
	}

	bool Inventory::IsEmpty() {
		for (ushort i=0; i<Size(); ++i) {
			if ( !inventory[i].isEmpty() ) {
				return false;
			}
		}
		return true;
	}

	quint8 Inventory::Number(const ushort i) const { return inventory[i].size(); }

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

		if ( craft_manager.MiniCraft(item, result) ) {
			while ( !inventory[num].isEmpty() ) {
				Block * const to_drop=ShowBlock(num);
				Pull(num);
				block_manager.DeleteBlock(to_drop);
			}
			for (ushort i=0; i<result.num; ++i)
				Get(block_manager.NewBlock(result.kind, result.sub));
			return 0; //success
		}
		return 2; //no such recipe
	}

	Inventory::Inventory(const ushort sz) :
			size(sz)
	{
		inventory=new QStack<Block *>[Size()];
	}

	Inventory::Inventory(QDataStream & str, const ushort sz) :
			size(sz)
	{
		inventory=new QStack<Block *>[Size()];
		for (ushort i=0; i<Size(); ++i) {
			quint8 num;
			str >> num;
			while ( num-- )
				inventory[i].push(block_manager.BlockFromFile(str));
		}
	}

	Inventory::~Inventory() {
		for (ushort i=0; i<Size(); ++i)
			while ( !inventory[i].isEmpty() ) {
				Block * const block=inventory[i].pop();
				block_manager.DeleteBlock(block);
			}
		delete [] inventory;
	}

//Ladder::
	Block * Ladder::DropAfterDamage() const { return block_manager.NewBlock(LADDER, Sub()); }

	QString & Ladder::FullName(QString & str) const { return str="Ladder"; }

	int Ladder::Kind() const { return LADDER; }

	int Ladder::BeforePush(const int) { return MOVE_UP; }

	float Ladder::TrueWeight() const { return 20; }

	bool Ladder::Catchable() const { return true; }

	Ladder::Ladder(const int sub) :
			Block(sub)
	{}

	Ladder::Ladder(QDataStream & str, const int sub) :
			Block(str, sub)
	{}

//Plate::
	QString & Plate::FullName(QString & str) const {
		switch ( Sub() ) {
			case WOOD: return str="Wooden board";
			case IRON: return str="Iron plate";
			case STONE: return str="Stone slab";
			default:
				fprintf(stderr,
					"Plate::FullName: unlisted sub: %d",
					Sub());
				return str="Strange plate";
		}
	}

	int Plate::Kind() const { return PLATE; }

	Block * Plate::DropAfterDamage() const { return block_manager.NewBlock(PLATE, Sub()); }

	int Plate::BeforePush(const int) { return JUMP; }

	float Plate::TrueWeight() const { return 10; }

	Plate::Plate(const int sub)
			:
			Block(sub, NONSTANDARD)
	{}

	Plate::Plate(QDataStream & str, const int sub) :
			Block(str, sub, NONSTANDARD)
	{}
