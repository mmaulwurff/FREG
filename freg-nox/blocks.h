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
#include <QDataStream>
#include <QStack>

class World;
class Shred;
class Inventory;
class Active;
class Animal;

const uchar onHead=0;
const uchar inRight=1;
const uchar inLeft=2;
const uchar onBody=3;
const uchar onLegs=4;

class Block { //blocks without special physics and attributes
	bool normal;
	
	bool Inscribable() const {
		return !( sub==AIR ||
			sub==NULLSTONE ||
			sub==A_MEAT ||
			sub==GREENERY ||
			sub==H_MEAT);
	}
	
	public:
	void SetNormal(const bool n) { normal=n; }
	bool Normal() const { return normal; }

	protected:
	quint16 sub;
	float weight;
	float shown_weight;
	quint8 direction;
	QString note;
	qint16 durability;

	public:
	virtual QString & FullName(QString&) const;

	void SetWeight(const float m) { shown_weight=m; }
	void SetWeight() { shown_weight=weight; }
	virtual float Weight() const { return shown_weight; }
	int GetDir() const { return direction; }
	void SetDir(const int dir) { direction=dir; }
	int Sub() const { return sub; }
	virtual bool Inscribe(const QString & str) {
		if ( Inscribable() ) {
			note=str;
			return true;
		}
		return false;
	}
	virtual QString & GetNote(QString & str) const { return str=note; }

	virtual int Kind() const { return BLOCK; }
	virtual bool CanBeIn() const { return true; }
	virtual bool CanBeOut() const {
		switch (sub) {
			case HAZELNUT: return false;
			default: return true;
		}
	}
	virtual int Movable() const {
	       return ( AIR==Sub() ) ?
		       ENVIRONMENT :
		       NOT_MOVABLE;
	}
	int Transparent() const {
		//0 - normal block,
		//1 - block is visible, but light can pass through it,
		//2 - invisible block
		switch (sub) {
			case AIR: return 2;
			case WATER: case GREENERY:
			case GLASS: return 1;
			default: return 0;
		}
	}
	virtual before_move_return BeforeMove(const int) { return NOTHING; }
	virtual int Move(const int) { return 0; }
	virtual int Move() { return 0; }
	virtual char MakeSound() const { return ' '; }
	virtual ushort Noise() const { return 1; }
	virtual usage_types Use() { return NO; }
	virtual int Damage(const ushort, const int dmg_kind);
	virtual short MaxDurability() const {
		switch ( Sub() ) {
			case GREENERY: return 1;
			case GLASS: return 2;
			default: return max_durability;
		}
	}
	void Restore() { durability=MaxDurability(); }
	short Durability() const { return durability; }
	virtual Block * DropAfterDamage() const {
		return ( BLOCK==Kind() && GLASS!=sub ) ?
			new Block(sub) : 0;
	}
	virtual Inventory * HasInventory() { return 0; }
	virtual Animal * IsAnimal() { return 0; }
	virtual Active * ActiveBlock() { return 0; }
	virtual Block * Drop(const ushort) { return 0; }

	virtual bool Armour() const { return false; }
	virtual bool Weapon() const { return false; }
	virtual bool Carving() const { return false; }
	virtual int DamageKind() const { return CRUSH; }
	virtual ushort DamageLevel() const { return 1; }

	virtual void Unregister() {}
	virtual int Eat(Block * const) { return 0; }

	virtual uchar LightRadius() const { return 0; }

	virtual int Temperature() const { 
		switch (sub) {
			case WATER: return -100;
			default: return 0;
		}
	}
	bool operator==(const Block &) const;

	void SaveToFile(QDataStream & out) const {
		out << (quint16)Kind() << sub << normal;
		
		if ( normal )
			return;

		out << weight
			<< direction
			<< durability
			<< note;
		SaveAttributes(out);
	}
	virtual void SaveAttributes(QDataStream &) const {}

	Block(
			const int=STONE,
			const short=max_durability,
			const float=0);
	Block(QDataStream &, const int sub_);
	virtual ~Block() {}
};

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

	Clock(
			World * const w,
			const int sub)
			:
			Block(sub, 2, 0.1),
			world (w)
	{}
	Clock (
			QDataStream & str,
			World * const w,
			const int sub)
			:
			Block(str, sub),
			world(w)
	{}
};

class Weapons : public Block {
	public:
	int Kind() const=0;
	int DamageKind() const=0;
	ushort DamageLevel() const=0;
	bool Weapon() const { return true; }
	bool CanBeOut() const { return false; }

	Weapons(const int sub, const short dur=max_durability)
			:
			Block(sub, dur) {}
	Weapons(QDataStream & str, const int sub)
			:
			Block(str, sub) {}
};

class Pick : public Weapons {
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
	float Weight() const { return 10; }

	Pick(
			const int sub,
			const short durability=max_durability)
			:
			Weapons(sub, durability)
	{}
	Pick(
			QDataStream & str,
			const int sub)
			:
			Weapons(str, sub)
	{}
};

class Active : public QObject, public Block {
	Q_OBJECT
	
	protected:
	ushort x_self, y_self, z_self;
	Shred * whereShred;

	signals:
	void Moved(int);
	void Destroyed();

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
	int Move() { return Move(direction); }

	ushort X() const { return x_self; }
	ushort Y() const { return y_self; }
	ushort Z() const { return z_self; }
	
	//!returns if block should be destroyed
	virtual bool Act() { return false; }

	char MakeSound() const { return ' '; }

	int Movable() const { return MOVABLE; }
	virtual bool ShouldFall() const { return true; }

	void ReloadToNorth() { y_self+=shred_width; }
	void ReloadToSouth() { y_self-=shred_width; }
	void ReloadToWest()  { x_self+=shred_width; }
	void ReloadToEast()  { x_self-=shred_width; }

	void Register(Shred *,
			const ushort,
			const ushort,
			const ushort);
	void Unregister();

	Active(
			const int sub,
			const short dur=max_durability)
			:
			Block(sub, dur),
	       		whereShred(NULL)
	{}
	Active(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			int sub,
			const short dur=max_durability)
			:
			Block(sub, dur)
	{
		Register(sh, x, y, z);
	}
	Active(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str,
			const int sub)
			:
			Block(str, sub)
	{
		Register(sh, x, y, z);
	}
	virtual ~Active();
};

class Animal : public Active {
	Q_OBJECT
	
	protected:
	quint16 breath;
	quint16 satiation;

	public:
	QString & FullName(QString&) const=0;

	ushort Breath() const { return breath; }
	ushort Satiation() const { return satiation; }
	int Eat(Block * const)=0;

	bool Act();

	void SaveAttributes(QDataStream & out) const {
		Block::SaveAttributes(out);
		out << breath << satiation;
	}

	Animal * IsAnimal() { return this; }

	Animal(Shred * const sh,
			const ushort i,
			const ushort j,
			const ushort k,
			int sub=A_MEAT,
			const short dur=max_durability)
			:
			Active(sh, i, j, k, sub, dur),
			breath(max_breath),
			satiation(seconds_in_day*time_steps_in_sec) {}
	Animal(Shred * const sh,
			const ushort i,
			const ushort j,
			const ushort k,
			QDataStream & str,
			const int sub)
			:
			Active(sh, i, j, k, str, sub)
	{
		str >> breath >> satiation;
	}
};

class Inventory {
	QStack<Block *> inventory[inventory_size];
	protected:
	Shred * inShred;

	public:
	QString & InvFullName(QString & str, const ushort i) const {
		return str=( inventory[i].isEmpty() ) ? "" :
			inventory[i].top()->FullName(str);
	}
	char * NumStr(char * const str, const ushort i) const {
		const ushort n=Number(i);
		if ( 1>=n )
			str[0]='\0';
		else
			sprintf(str, " (%hdx)", n);
		return str;
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
		for (ushort i=0; i<inventory_size; ++i)
			sum+=GetInvWeight(i);
		return sum;
	}
	quint8 Number(const ushort i) const {
		return inventory[i].size();
	}
	Block * ShowBlock(const ushort num) const {
		if ( num>inventory_size || inventory[num].isEmpty() )
			return 0;
		return inventory[num].top();
	}

	virtual QString & FullName(QString&) const=0;
	virtual int Kind() const=0;
	virtual int Sub() const=0;
	virtual bool Access() const { return true; }

	virtual ushort InventoryStart() const { return 0; } 
	virtual Inventory * HasInventory() { return this; }
	usage_types Use() { return OPEN; }

	virtual Block * Drop(const ushort n) {
		if ( inventory_size<=n || !Number(n) )
			return 0;

		return inventory[n].pop();
	}
	bool Get(Block * const block) {
		if ( !block )
			return true;

		for (ushort i=InventoryStart(); i<inventory_size; ++i)
			if ( GetExact(block, i) )
				return true;
		return false;
	}
	bool GetExact(Block * const block, const ushort num) {
		if ( inventory[num].isEmpty() ||
				( *block==*inventory[num].top() &&
				Number(num)<max_stack_size ) )
		{
			inventory[num].push(block);
			return true;
		}
		return false;
	}
	void GetAll(Inventory * const from) {
		if ( !from || !from->Access() )
			return;

		for (ushort i=0; i<inventory_size; ++i)
			while ( from->Number(i) ) {
				Block * const temp=from->Drop(i);
				if ( !Get(temp) ) {
					from->Get(temp);
					return;
				}
			}
	}
	virtual void SaveAttributes(QDataStream & out) const {
		for (ushort i=0; i<inventory_size; ++i) {
			out << Number(i);
			for (ushort j=0; j<Number(i); ++j)
				inventory[i].top()->SaveToFile(out);
		}
	}

	Inventory(Shred * const sh)
			:
			inShred(sh)
	{}
	Inventory(
			Shred * const,
			QDataStream & str);
	~Inventory() {
		for (ushort i=0; i<inventory_size; ++i)
			while ( !inventory[i].isEmpty() ) {
				Block * const block=inventory[i].pop();
				if ( !block->Normal() )
					delete block;
			}
	}
};

class Dwarf : public Animal, public Inventory {
	Q_OBJECT
	
	quint16 noise;

	public:
	ushort Noise() const { return noise; }
	bool CarvingWeapon() const {
		return ( (ShowBlock(inRight) && ShowBlock(inRight)->Carving()) ||
		         (ShowBlock(inLeft)  && ShowBlock(inLeft )->Carving()) );
	}

	int Kind() const { return DWARF; }
	int Sub() const { return Block::Sub(); }
	QString & FullName(QString & str) const {
		return str="Dwarf"+note;
	}
	char MakeSound() const { return (rand()%10) ? ' ' : 's'; }
	bool CanBeIn() const { return false; }
	float Weight() const { return InvWeightAll()+100; }
	ushort InventoryStart() const { return 5; } 

	before_move_return BeforeMove(const int);
	bool Act();

	int Eat(Block * const to_eat) {
		if ( !to_eat )
			return 2;

		switch ( to_eat->Sub() ) {
			case HAZELNUT: satiation+=seconds_in_hour*time_steps_in_sec; break;
			case H_MEAT:   satiation+=seconds_in_hour*time_steps_in_sec*2.5; break;
			case A_MEAT:   satiation+=seconds_in_hour*time_steps_in_sec*2; break;
			default: return 0; //not ate
		}
		
		if ( seconds_in_day*time_steps_in_sec < satiation )
			satiation=1.1*seconds_in_day*time_steps_in_sec;

		return 1; //ate
	}

	Inventory * HasInventory() { return Inventory::HasInventory(); }
	bool Access() const { return false; }
	Block * Drop(const ushort n) { return Inventory::Drop(n); }
	int Wield(const ushort num) {
		Block *  const block=Drop(num);
		if  ( !block )
			return 1;

		if ( block->Weapon() ) {
			if ( !ShowBlock(inRight) ) {
				GetExact(block, inRight);
				return 0;
			}
			if ( !ShowBlock(inLeft) ) {
				GetExact(block, inLeft);
				return 0;
			}
		}
		//TODO: clothes, armour
		Get(block);
		return 2;
	}
	Block * DropAfterDamage() const;

	void SaveAttributes(QDataStream & out) const {
		Animal::SaveAttributes(out);
		Inventory::SaveAttributes(out);
		out << noise;
	}

	uchar LightRadius() const { return 3; }

	Dwarf(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z)
			:
			Animal(sh, x, y, z, H_MEAT, 100),
			Inventory(sh),
			noise(1)
	{
		Get(new Clock(GetWorld(), IRON));
		Inscribe("Urist");
	}
	Dwarf(
			Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str,
			const int sub)
			:
			Animal(sh, x, y, z, str, sub),
			Inventory(sh, str)
	{
		str >> noise;
	}
};

class Chest : public Block, public Inventory {
	public:
	int Kind() const { return CHEST; }
	int Sub() const { return Block::Sub(); }
	QString & FullName(QString & str) const {
		switch (sub) {
			case WOOD: return str="Wooden Chest";
			default:
				fprintf(stderr,
					"Chest::FullName(QString&): Chest has unknown substance: %d\n",
					sub);
				return str="Chest";
		}
	}
	Inventory * HasInventory() { return Inventory::HasInventory(); }
	Block * Drop(const ushort n) { return Inventory::Drop(n); }
	float Weight() const { return InvWeightAll()+300; }

	usage_types Use() { return Inventory::Use(); }

	Block * DropAfterDamage() const { return new Chest(inShred, sub); }
	
	void SaveAttributes(QDataStream & out) const {
		Block::SaveAttributes(out);
		Inventory::SaveAttributes(out);
	}

	Chest(
			Shred * const sh,
			const int s=WOOD,
			const short dur=max_durability)
			:
			Block(s, dur),
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
};

class Pile : public Active, public Inventory {
	Q_OBJECT

	bool ifToDestroy;

	public:
	int Kind() const { return PILE; }
	int Sub() const { return Block::Sub(); }
	QString & FullName(QString & str) const { return str="Pile"; }

	Inventory * HasInventory() { return Inventory::HasInventory(); }
	usage_types Use() { return Inventory::Use(); }
	float Weight() const { return InvWeightAll(); }

	bool Act();
	
	Block * Drop(const ushort n) {
		Block * const temp=Inventory::Drop(n);
		ifToDestroy=true;
		for (ushort i=0; i<inventory_size; ++i)
			if ( Number(i) ) {
				ifToDestroy=false;
				break;
			}
		return temp;
	}

	before_move_return BeforeMove(const int dir);
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
			Active(sh, x, y, z, DIFFERENT),
			Inventory(sh),
			ifToDestroy(false)
	{
		Get(block);
	}
	Pile(Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str,
			const int sub)
			:
			Active(sh, x, y, z, str, sub),
			Inventory(sh, str)
	{
		str >> ifToDestroy;
	}
};

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
			const damage_kinds dam_kind)
	{
		return ( HEAT==dam_kind ) ?
			durability-=dam :
			durability;
	}

	bool Act();

	int Temperature() const {
		if (WATER==sub) return 0;
		else return 1000;
	}

	Liquid(Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			const int sub=WATER)
			:
			Active(sh, x, y, z, sub) {}
	Liquid(Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str,
			const int sub)
			:
			Active(sh, x, y, z, str, sub)
		{}
};

class Grass : public Active {
	Q_OBJECT
	
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

	bool ShouldFall() const { return false; }

	before_move_return BeforeMove(const int) { return DESTROY; }
	bool Act();

	Grass() : Active(GREENERY, 1) {}
	Grass(Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z)
			:
			Active(sh, x, y, z, GREENERY, 1) {}
	Grass(Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str,
			const int sub)
			:
			Active(sh, x, y, z, str, sub)
		{}
};

class Bush : public Active, public Inventory {
	Q_OBJECT
	
	public:
	QString & FullName(QString & str) const { return str="Bush"; }
	int Kind() const { return BUSH; }
	int Sub() const { return Block::Sub(); }

	usage_types Use() { return Inventory::Use(); }
	Inventory * HasInventory() { return Inventory::HasInventory(); }
	int Movable() const { return NOT_MOVABLE; }
	float Weight() const { return InvWeightAll()+Block::Weight(); }

	bool Act();

	Block * DropAfterDamage() const;

	void SaveAttributes(QDataStream & out) const {
		Active::SaveAttributes(out);
		Inventory::SaveAttributes(out);
	}

	Bush(Shred * const sh) :
			Active(sh, 0, 0, 0, WOOD),
	       		Inventory(sh) {}
	Bush(Shred * const sh,
			QDataStream & str,
			const int sub)
			:
			Active(sh, 0, 0, 0, str, sub),
			Inventory(sh, str) {}
};

class Rabbit : public Animal {
	Q_OBJECT
	
	float Attractive(int kind) const;
	
	public:
	QString & FullName(QString & str) const { return str="Rabbit"; }
	int Kind() const { return RABBIT; }

	bool Act();
	float Weight() const { return 2; }

	int Eat(Block * const to_eat) {
		if ( NULL==to_eat )
			return 2;

		if ( GREENERY==to_eat->Sub() ) {
			satiation+=seconds_in_hour*time_steps_in_sec*4;
			return 1;
		}
		return 0;
	}

	Block * DropAfterDamage() const;

	Rabbit(Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z)
			:
			Animal(sh, x, y, z)
		{}
	Rabbit(Shred * const sh,
			const ushort x,
			const ushort y,
			const ushort z,
			QDataStream & str,
			const int sub)
			:
			Animal(sh, x, y, z, str, sub)
		{}
};

#endif
