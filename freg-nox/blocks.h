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

#ifndef BLOCKS_H
#define BLOCKS_H

#include "header.h"
#include <QObject>
#include <QStack>
#include <QDataStream>

class World;
class Shred;
class Inventory;
class Active;
class Animal;

class Block { //blocks without special physics and attributes
	bool normal;
	void SetTransparency(const quint8 transp) {
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

	protected:
	quint8 transparent;
	quint16 sub;
	bool nullWeight;
	quint8 direction;
	QString note;
	qint16 durability;

	public:
	virtual QString & FullName(QString & str) const;
	virtual int Kind() const { return BLOCK; }
	virtual float TrueWeight() const;
	virtual bool Catchable() const { return false; }
	virtual bool CanBeIn() const { return true; }
	virtual bool CanBeOut() const {
		switch (sub) {
			case HAZELNUT: return false;
			default: return true;
		}
	}
	virtual int Movable() const {
		return ( AIR==Sub() ) ? ENVIRONMENT : NOT_MOVABLE;
	}
	virtual bool Inscribe(const QString & str) {
		if ( Inscribable() ) {
			note=str;
			return true;
		}
		return false;
	}
	virtual before_move_return BeforeMove(const int) { return NOTHING; }
	virtual int BeforePush(const int) { return NO_ACTION; }
	virtual int Move(const int) { return 0; }
	virtual usage_types Use() { return NO; }
	virtual int Damage(const ushort dmg, const int dmg_kind);
	virtual short MaxDurability() const {
		switch ( Sub() ) {
			case GREENERY: return 1;
			case GLASS: return 2;
			default: return max_durability;
		}
	}
	virtual Block * DropAfterDamage() const {
		return ( BLOCK==Kind() && GLASS!=sub ) ?
			new Block(sub) : 0;
	}

	virtual Inventory * HasInventory() { return 0; }
	virtual Animal * IsAnimal() { return 0; }
	virtual Active * ActiveBlock() { return 0; }

	virtual bool Armour() const { return false; }
	virtual bool IsWeapon() const { return false; }
	virtual bool Carving() const { return false; }
	virtual int DamageKind() const { return CRUSH; }
	virtual ushort DamageLevel() const { return 1; }

	virtual uchar LightRadius() const { return 0; }
	virtual int Temperature() const {
		switch (sub) {
			case WATER: return -100;
			default: return 0;
		}
	}
	virtual bool Inscribable() const {
		//TODO: prevent inscribing living creatures
		return !(
			sub==AIR       ||
			sub==NULLSTONE ||
			sub==A_MEAT    ||
			sub==GREENERY  );
	}

	virtual void SaveAttributes(QDataStream &) const {}

	void Restore() { durability=MaxDurability(); }
	void NullWeight(const bool null) { nullWeight=null; }
	float Weight() const { return nullWeight ? 0 : TrueWeight(); }
	void SetDir(const int dir) { direction=dir; }
	void SetNormal(const bool n) { normal=n; }

	bool Normal() const { return normal; }
	int  GetDir() const { return direction; }
	int  Sub() const { return sub; }
	short Durability() const { return durability; }
	QString & GetNote(QString & str) const { return str=note; }
	int Transparent() const { return transparent; }

	bool operator==(const Block &) const;

	void SaveToFile(QDataStream & out) const {
		out << (quint16)Kind() << sub << normal;

		if ( normal )
			return;

		out << nullWeight
			<< direction
			<< durability
			<< note;
		SaveAttributes(out);
	}

	Block(
			const int sb=STONE,
			const short dur=max_durability,
			const quint8 transp=UNDEF)
			:
			normal(false),
			sub(sb),
			nullWeight(false),
			direction(UP),
			note(""),
			durability(dur)
	{
		SetTransparency(transp);
	}
	Block(
			QDataStream &,
			const int sub_,
			const quint8 transp=UNDEF);
	virtual ~Block() {}
}; //class Block

class Plate : public Block {
	QString & FullName(QString & str) const {
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
	int Kind() const { return PLATE; }
	Block * DropAfterDamage() const { return new Plate(Sub()); }
	int BeforePush(const int) { return JUMP; }
	float TrueWeight() const { return 10; }

	public:
	Plate(const int sub)
			:
			Block(sub, max_durability, NONSTANDARD)
	{}
	Plate(
			QDataStream & str,
			const int sub)
			:
			Block(str, sub, NONSTANDARD)
	{}
}; //class Plate

class Ladder : public Block {
	QString & FullName(QString & str) const { return str="Ladder"; }
	int Kind() const { return LADDER; }
	Block * DropAfterDamage() const { return new Ladder(Sub()); }
	int BeforePush(const int) { return MOVE_UP; }
	float TrueWeight() const { return 20; }
	virtual bool Catchable() const { return true; }

	public:
	Ladder(const int sub) :
			Block(sub)
	{}
	Ladder(
			QDataStream & str,
			const int sub)
			:
			Block(str, sub)
	{}
}; //class Ladder

class Clock : public Block {
	World * world;

	public:
	int Kind() const { return CLOCK; }
	QString & FullName(QString & str) const {
		switch ( sub ) {
			case IRON: return str="Iron clock";
			default:
				fprintf(stderr,
					"Clock::FullName: unlisted sub: %d\n",
					sub);
				return str="Strange clock";
		}
	}

	Block * DropAfterDamage() const { return new Clock(world, Sub()); }
	short MaxDurability() const { return 2; }
	usage_types Use();
	int BeforePush(const int) {
		Use();
		return NO_ACTION;
	}
	float TrueWeight() const { return 0.1; }

	Clock(
			World * const w,
			const int sub)
			:
			Block(sub, 2, NONSTANDARD),
			world (w)
	{}
	Clock (
			QDataStream & str,
			World * const w,
			const int sub)
			:
			Block(str, sub, NONSTANDARD),
			world(w)
	{}
}; //class Clock

class Weapon : public Block {
	public:
	QString & FullName(QString & str) const {
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
	int Kind() const { return WEAPON; }
	ushort DamageLevel() const {
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
	int BeforePush(const int) {
		return ( IRON==Sub() ) ? DAMAGE : NO_ACTION;
	}

	float TrueWeight() const { return 1; }
	bool IsWeapon() const { return true; }
	bool CanBeOut() const { return false; }

	Weapon(
			const int sub)
			:
			Block(sub, max_durability, NONSTANDARD)
	{}
	Weapon(
			QDataStream & str,
			const int sub)
			:
			Block(str, sub, NONSTANDARD)
	{}
}; //class Weapon

class Pick : public Weapon {
	public:
	int Kind() const { return PICK; }
	int DamageKind() const { return MINE; }
	ushort DamageLevel() const {
		switch ( Sub() ) {
			case IRON: return 10;
			default:
				fprintf(stderr,
					"Pick::DamageLevel: unlisted sub: %d\n",
					Sub());
				return 1;
		}
	}
	QString & FullName(QString & str) const {
		switch ( sub ) {
			case IRON: return str="Iron pick";
			default:
				fprintf(stderr,
					"Pick::FullName(QString&): Pick has unknown substance: %d\n",
					sub);
				return str="Strange pick";
		}
	}

	bool Carving() const { return true; }
	float TrueWeight() const {
		switch ( Sub() ) {
			case IRON: return 10;
			default:
				fprintf(stderr,
					"Pick::Pick: unlisted sub: %d\n",
					Sub());
				return 8;
		}
	}

	Pick(
			const int sub)
			:
			Weapon(sub)
	{}
	Pick(
			QDataStream & str,
			const int sub)
			:
			Weapon(str, sub)
	{}
}; //class Pick

class Active : public QObject, public Block {
	Q_OBJECT

	quint8 fall_height;
	bool falling;

	protected:
	quint8 timeStep;
	ushort x_self, y_self, z_self;
	Shred * whereShred;

	signals:
	void Moved(int);
	void Destroyed();
	void Updated();

	public:
	World * GetWorld() const;
	QString & FullName(QString & str) const {
		switch (sub) {
			case SAND: return str="Sand";
			default:
				fprintf(stderr,
					"Active:FullName(QString&): Unlisted sub: %d\n",
					sub);
				return str="Unkown active block";
		}
	}
	int Kind() const { return ACTIVE; }

	Active * ActiveBlock() { return this; }
	int Move(const int);
	void SetNotFalling() { falling=false; }

	ushort X() const { return x_self; }
	ushort Y() const { return y_self; }
	ushort Z() const { return z_self; }

	///Block's  action, called every game turn.
	/**
	 * Active::Act() should be mentioned in derived classes'
	 * Act() (if reimplemented) before the end.
	 * This makes common damage from falling.
	 */
	virtual void Act();

	int Movable() const { return MOVABLE; }
	virtual bool ShouldFall() const { return true; }
	before_move_return BeforeMove(const int) { return NOTHING; }
	int Damage(const ushort dmg, const int dmg_kind) {
		const int last_dur=durability;
		const int new_dur=Block::Damage(dmg, dmg_kind);
		if ( last_dur != new_dur )
			emit Updated();
		return new_dur;
	}

	void ReloadToNorth() { y_self+=shred_width; }
	void ReloadToSouth() { y_self-=shred_width; }
	void ReloadToWest()  { x_self+=shred_width; }
	void ReloadToEast()  { x_self-=shred_width; }

	void SaveAttributes(QDataStream & out) const {
		out << timeStep << fall_height << falling;
	}

	void Register(Shred *,
			const ushort,
			const ushort,
			const ushort);
	void Unregister();

	Active(
			const int sub,
			const short dur=max_durability,
			const quint8 transp=UNDEF)
			:
			Block(sub, dur, transp),
			fall_height(0),
			falling(false),
			timeStep(0),
	       		whereShred(0)
	{}
	Active(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			const int sub,
			const short dur=max_durability,
			const quint8 transp=UNDEF)
			:
			Block(sub, dur, transp),
			fall_height(0),
			falling(false),
			timeStep(0)
	{
		Register(sh, x, y, z);
	}
	Active(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str,
			const int sub,
			const quint8 transp=UNDEF);
	virtual ~Active();
}; //class Active

class Animal : public Active {
	Q_OBJECT

	protected:
	quint16 breath;
	quint16 satiation;

	public:
	QString & FullName(QString&) const=0;

	ushort Breath() const { return breath; }
	ushort Satiation() const { return satiation; }
	virtual int Eat(Block * const)=0;

	void Act();

	void SaveAttributes(QDataStream & out) const {
		Active::SaveAttributes(out);
		out << breath << satiation;
	}

	Animal * IsAnimal() { return this; }

	Animal(
			Shred * const sh,
			const ushort i,
			const ushort j,
			const ushort k,
			const int sub=A_MEAT)
			:
			Active(sh, i, j, k, sub, max_durability, NONSTANDARD),
			breath(max_breath),
			satiation(seconds_in_day)
	{}
	Animal(
			Shred * const sh,
			const ushort i,
			const ushort j,
			const ushort k,
			QDataStream & str,
			const int sub);
	virtual ~Animal() {}
}; //class Animal

class Inventory {
	static const ushort max_stack_size=9;
	const ushort size;
	QStack<Block *> * inventory;
	Shred * inShred;

	public:
	virtual QString & FullName(QString&) const=0;
	virtual int Kind() const=0;
	virtual int Sub() const=0;
	virtual float TrueWeight() const=0;
	virtual bool Access() const { return true; }
	virtual ushort Start() const { return 0; }
	virtual Inventory * HasInventory() { return this; }
	virtual int Drop(const ushort num, Inventory * const inv_to);
	virtual bool Get(Block * const block);
	virtual int GetAll(Inventory * const from);
	virtual usage_types Use() { return OPEN; }
	virtual void Pull(const ushort num) {
		if ( !inventory[num].isEmpty() )
			inventory[num].pop();
	}
	virtual void SaveAttributes(QDataStream & out) const {
		for (ushort i=0; i<Size(); ++i) {
			out << Number(i);
			for (ushort j=0; j<Number(i); ++j)
				inventory[i].top()->SaveToFile(out);
		}
	}

	World * InWorld() const;
	Shred * InShred() const { return inShred; }
	void SetShred(Shred * const sh) { inShred=sh; }
	ushort Size() const { return size; }
	void Register(Shred * const sh) { inShred=sh; }
	bool GetExact(Block * const block, const ushort num);
	int MiniCraft(const ushort num);
	int InscribeInv(const ushort num, const QString & str);
	QString & InvFullName(QString & str, const ushort i) const {
		return str=( inventory[i].isEmpty() ) ? "" :
			inventory[i].top()->FullName(str);
	}
	QString & NumStr(QString & str, const ushort i) const {
		return str=( 1<Number(i) ) ?
			QString(" (%1x)").arg(Number(i)) :
			"";
	}
	float GetInvWeight(const ushort i) const {
		return ( inventory[i].isEmpty() ) ? 0 :
			inventory[i].top()->Weight()*Number(i);
	}
	int GetInvSub(const ushort i) const {
		return ( inventory[i].isEmpty() ) ? AIR :
			inventory[i].top()->Sub();
	}
	int GetInvKind(const ushort i) const {
		return ( inventory[i].isEmpty() ) ? BLOCK :
			inventory[i].top()->Kind();
	}
	QString & GetInvNote(QString & str, const ushort num) const {
		return str=inventory[num].top()->GetNote(str);
	}
	float InvWeightAll() const {
		float sum=0;
		for (ushort i=0; i<Size(); ++i)
			sum+=GetInvWeight(i);
		return sum;
	}
	quint8 Number(const ushort i) const {
		return inventory[i].size();
	}
	Block * ShowBlock(const ushort num) const {
		if ( num>Size() || inventory[num].isEmpty() )
			return 0;
		return inventory[num].top();
	}

	bool HasRoom() {
		for (ushort i=Start(); i<Size(); ++i)
			if ( inventory[i].isEmpty() )
				return true;
		return false;
	}
	bool IsEmpty() {
		for (ushort i=0; i<Size(); ++i)
			if ( !inventory[i].isEmpty() )
				return false;
		return true;
	}

	//it is not recommended to make inventory size more than 26,
	//because it will not be convenient to deal with inventory
	//in console version.
	Inventory(
			Shred * const sh,
			const ushort sz=26)
			:
			size(sz),
			inShred(sh)
	{
		inventory=new QStack<Block *>[Size()];
	}
	Inventory(
			Shred * const,
			QDataStream & str,
			const ushort size=26);
	virtual ~Inventory() {
		for (ushort i=0; i<Size(); ++i)
			while ( !inventory[i].isEmpty() ) {
				Block * const block=inventory[i].pop();
				if ( !block->Normal() )
					delete block;
			}
		delete [] inventory;
	}
}; //class Inventory

class Dwarf : public Animal, public Inventory {
	Q_OBJECT

	static const uchar onHead=0;
	static const uchar inRight=1;
	static const uchar inLeft=2;
	static const uchar onBody=3;
	static const uchar onLegs=4;

	public:
	bool CarvingWeapon() const {
		return ( (ShowBlock(inRight) && ShowBlock(inRight)->Carving()) ||
		         (ShowBlock(inLeft)  && ShowBlock(inLeft )->Carving()) );
	}

	int Kind() const { return DWARF; }
	int Sub() const { return Block::Sub(); }
	QString & FullName(QString & str) const {
		return str="Dwarf"+note;
	}
	bool CanBeIn() const { return false; }
	float TrueWeight() const { return ShouldFall() ? InvWeightAll()+60 : 0; }
	bool ShouldFall() const;
	ushort Start() const { return 5; }
	int DamageKind() const {
		if ( Number(inRight) )
			return ShowBlock(inRight)->DamageKind();
		if ( Number(inLeft) )
			return ShowBlock(inLeft)->DamageKind();
		return CRUSH;
	}
	ushort DamageLevel() const {
		ushort level=1;
		if ( Number(inRight) )
			level+=ShowBlock(inRight)->DamageLevel();
		if ( Number(inLeft) )
			level+=ShowBlock(inRight)->DamageLevel();
		return level;
	}

	before_move_return BeforeMove(const int);
	void Act();

	int Eat(Block * const to_eat) {
		if ( !to_eat )
			return 1;

		switch ( to_eat->Sub() ) {
			case HAZELNUT: satiation+=seconds_in_hour; break;
			case H_MEAT:   satiation+=seconds_in_hour*2.5; break;
			case A_MEAT:   satiation+=seconds_in_hour*2; break;
			default: return 2; //not ate
		}

		if ( seconds_in_day < satiation )
			satiation=1.1*seconds_in_day;

		return 0; //ate
	}

	Inventory * HasInventory() { return Inventory::HasInventory(); }
	bool Access() const { return false; }
	int Wield(const ushort num) {
		Block *  const block=ShowBlock(num);
		if  ( !block )
			return 1;

		if ( block->IsWeapon() ) {
			if ( !ShowBlock(inRight) ) {
				GetExact(block, inRight);
				Pull(num);
				return 0;
			}
			if ( !ShowBlock(inLeft) ) {
				GetExact(block, inLeft);
				Pull(num);
				return 0;
			}
		}
		//TODO: clothes, armour
		return 2;
	}
	Block * DropAfterDamage() const;

	void SaveAttributes(QDataStream & out) const {
		Animal::SaveAttributes(out);
		Inventory::SaveAttributes(out);
	}

	uchar LightRadius() const { return 3; }

	Dwarf(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z)
			:
			Animal(sh, x, y, z, H_MEAT),
			Inventory(sh)
	{
		Get(new Clock(GetWorld(), IRON));
		Inscribe("Urist");
	}
	Dwarf(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str)
			:
			Animal(sh, x, y, z, str, H_MEAT),
			Inventory(sh, str)
	{}
	virtual ~Dwarf() {}
}; //class Dwarf

class Chest : public Block, public Inventory {
	public:
	int Kind() const { return CHEST; }
	int Sub() const { return Block::Sub(); }
	QString & FullName(QString & str) const {
		switch (sub) {
			case WOOD: return str="Wooden chest";
			case STONE: return str="Stone chest";
			default:
				fprintf(stderr,
					"Chest::FullName(QString&): Chest has unknown substance: %d\n",
					sub);
				return str="Chest";
		}
	}
	Inventory * HasInventory() { return Inventory::HasInventory(); }

	usage_types Use() { return Inventory::Use(); }

	Block * DropAfterDamage() const { return new Chest(0, sub); }
	float TrueWeight() const {
		switch ( Sub() ) {
			case WOOD:  return 100+InvWeightAll();
			case IRON:  return 150+InvWeightAll();
			case STONE: return 120+InvWeightAll();
			default:
				fprintf(stderr,
					"Chest::Chest: unlisted sub: %d\n",
					Sub());
				return 100+InvWeightAll();
		}
	}

	void SaveAttributes(QDataStream & out) const {
		Inventory::SaveAttributes(out);
	}

	Chest(
			Shred * const sh,
			const int s=WOOD)
			:
			Block(s),
			Inventory(sh)
	{
		Get(new Pick(IRON));
	}
	Chest(
			Shred * const sh,
			QDataStream & str,
			const int sub)
			:
			Block(str, sub),
			Inventory(sh, str)
	{}
}; //class Chest

class Pile : public Active, public Inventory {
	Q_OBJECT

	bool ifToDestroy;

	public:
	int Kind() const { return PILE; }
	int Sub() const { return Block::Sub(); }
	QString & FullName(QString & str) const { return str="Pile"; }

	Inventory * HasInventory() { return Inventory::HasInventory(); }
	usage_types Use() { return Inventory::Use(); }
	float TrueWeight() const { return InvWeightAll(); }

	void Act();

	before_move_return BeforeMove(const int dir);
	int Drop(const ushort n, Inventory * const inv) {
		const int ret=Inventory::Drop(n, inv);
		ifToDestroy=IsEmpty();
		return ret;
	}
	void Pull(const ushort num) {
		Inventory::Pull(num);
		ifToDestroy=IsEmpty();
	}

	bool CanBeIn() const { return false; }

	void SaveAttributes(QDataStream & out) const {
		Active::SaveAttributes(out);
		Inventory::SaveAttributes(out);
		out << ifToDestroy;
	}

	Pile(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			Block * const block=NULL)
			:
			Active(sh, x, y, z, DIFFERENT, 1, NONSTANDARD),
			Inventory(sh),
			ifToDestroy(false)
	{
		Get(block);
	}
	Pile(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str);
}; //class Pile

class Liquid : public Active {
	Q_OBJECT

	bool CheckWater(const int dir) const;

	public:
	int Movable() const { return ENVIRONMENT; }

	int Kind() const { return LIQUID; }
	QString & FullName(QString & str) const {
		switch (sub) {
			case WATER: return str="Water";
			case STONE: return str="Lava";
			default:
				fprintf(stderr,
					"Liquid::FullName(QString&): Liquid has unknown substance: %d\n",
					sub);
				return str="Unknown liquid";
		}
	}

	int Damage(
			const ushort dam,
			const int dam_kind)
	{
		return ( HEAT==dam_kind ) ?
			durability-=dam :
			durability;
	}

	void Act();

	int Temperature() const { return ( WATER==sub ) ? 0 : 1000; }

	Liquid(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			const int sub=WATER)
			:
			Active(sh, x, y, z, sub)
	{}
	Liquid(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str,
			const int sub)
			:
			Active(sh, x, y, z, str, sub)
	{}
}; //class Liquid

class Grass : public Active {
	Q_OBJECT
	static const quint8 grass_dur=1;
	public:
	QString & FullName(QString & str) const {
		switch ( sub ) {
			case GREENERY: return str="Grass";
			default:
				fprintf(stderr,
					"Grass::FullName(QString&): unlisted sub: %d\n",
					sub);
				return str="Unknown plant";
		}
	}
	int Kind() const { return GRASS; }
	short MaxDurability() const { return grass_dur; }

	bool ShouldFall() const { return false; }

	before_move_return BeforeMove(const int) { return DESTROY; }
	void Act();

	Grass(
			Shred * const sh=0,
			const ushort x=0,
			const ushort y=0,
			const ushort z=0)
			:
			Active(sh, x, y, z, GREENERY, grass_dur)
	{}
	Grass(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str)
			:
			Active(sh, x, y, z, str, GREENERY)
	{}
}; //class Grass

class Bush : public Active, public Inventory {
	Q_OBJECT
	static const quint8 bush_dur=20;
	public:
	QString & FullName(QString & str) const { return str="Bush"; }
	int Kind() const { return BUSH; }
	int Sub() const { return Block::Sub(); }
	short MaxDurability() const { return bush_dur; }

	usage_types Use() { return Inventory::Use(); }
	Inventory * HasInventory() { return Inventory::HasInventory(); }
	int Movable() const { return NOT_MOVABLE; }
	float TrueWeight() const { return InvWeightAll()+20; }

	void Act();

	Block * DropAfterDamage() const;

	void SaveAttributes(QDataStream & out) const {
		Active::SaveAttributes(out);
		Inventory::SaveAttributes(out);
	}

	Bush(
			Shred * const sh)
			:
			Active(sh, 0, 0, 0, WOOD, bush_dur),
	       		Inventory(sh)
	{}
	Bush(
			Shred * const sh,
			QDataStream & str)
			:
			Active(sh, 0, 0, 0, str, WOOD),
			Inventory(sh, str)
	{}
}; //class Bush

class Rabbit : public Animal {
	Q_OBJECT

	float Attractive(int kind) const;

	public:
	QString & FullName(QString & str) const { return str="Rabbit"; }
	int Kind() const { return RABBIT; }

	void Act();
	float TrueWeight() const { return 20; }

	int Eat(Block * const to_eat) {
		if ( NULL==to_eat )
			return 2;

		if ( GREENERY==to_eat->Sub() ) {
			satiation+=seconds_in_hour*4;
			return 1;
		}
		return 0;
	}

	Block * DropAfterDamage() const;

	Rabbit(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z)
			:
			Animal(sh, x, y, z)
	{}
	Rabbit(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str)
			:
			Animal(sh, x, y, z, str, A_MEAT)
	{}
}; //class Rabbit

class Workbench : public Block, public Inventory {
	static const ushort workbench_size=10;

	void Craft();

	public:
	QString & FullName(QString & str) const {
		switch ( Sub() ) {
			case WOOD: return str="Workbench";
			case IRON: return str="Iron anvil";
			default:
				fprintf(stderr,
					"Bench::FullName: unlisted sub: %d\n",
					Sub());
				return str="Strange workbench";
		}
	}
	int Kind() const { return WORKBENCH; }
	float TrueWeight() const { return InvWeightAll()+80; }
	usage_types Use() { return OPEN; }
	Block * DropAfterDamage() const { return new Workbench(InShred(), Sub()); }
	Inventory * HasInventory() { return Inventory::HasInventory(); }

	int Sub() const { return Block::Sub(); }
	ushort Start() const { return 1; }

	int Drop(const ushort num, Inventory * const inv_to) {
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
					if ( !to_pull->Normal() )
						delete to_pull;
				}
		} else {
			if ( !inv_to->Get(ShowBlock(num)) )
				return 2;
			Pull(num);
			Craft();
		}
		return 0;
	}
	bool Get(Block * const block) {
		if ( Inventory::Get(block) ) {
			Craft();
			return true;
		}
		return false;
	}
	int GetAll(Inventory * const from) {
		const int err=Inventory::GetAll(from);
		if ( !err ) {
			Craft();
			return 0;
		} else
			return err;
	}

	void SaveAttributes(QDataStream & out) const {
		Inventory::SaveAttributes(out);
	}

	Workbench(
			Shred * const sh,
			const int sub)
			:
			Block(sub),
			Inventory(sh, workbench_size)
	{}
	Workbench(
			Shred * const sh,
			QDataStream & str,
			const int sub)
			:
			Block(str, sub),
			Inventory(sh, str, workbench_size)
	{}
}; //class Workbench

class Door : public Active {
	bool shifted;
	bool locked;
	int movable;
	int Kind() const { return DOOR; }
	QString & FullName(QString & str) const {
		return str=(locked ? "Locked door" : "Door");
	}
	int Movable() const { return movable; }
	usage_types Use() {
		locked=locked ? false : true;
		return NO;
	}
	int BeforePush(const int dir);
	void Act();
	Block * DropAfterDamage() const { return new Door(0, 0, 0, 0, Sub()); }

	void SaveAttributes(QDataStream & out) const {
		Active::SaveAttributes(out);
		out << shifted << locked;
	}

	public:
	Door(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			const int sub)
			:
			Active(sh, x, y, z, sub, max_durability, NONSTANDARD),
			shifted(false),
			locked(false),
			movable(NOT_MOVABLE)
	{}
	Door(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str,
			const int sub);
}; //class Door

#endif
