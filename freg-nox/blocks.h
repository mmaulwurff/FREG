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

#include <QObject>
#include <QStack>
#include "header.h"

class QDataStream;
class World;
class Shred;
class Inventory;
class Active;
class Animal;

class Block { //blocks without special physics and attributes
	bool inMemoryChunk;
	void SetTransparency(quint8 transp);
	virtual float TrueWeight() const;

	protected:
	quint8 transparent;
	quint16 sub;
	bool nullWeight;
	quint8 direction;
	QString note;
	qint16 durability;

	public:
	void SetInMemoryChunk(bool in);
	bool InMemoryChunk() const;
	virtual QString & FullName(QString & str) const;
	virtual int Kind() const;
	virtual bool Catchable() const;
	virtual bool CanBeOut() const;
	virtual int Movable() const;
	virtual bool Inscribe(const QString & str);
	virtual before_move_return BeforeMove(int);
	virtual int BeforePush(int);
	virtual int Move(int);
	virtual usage_types Use();
	virtual int Damage(ushort dmg, int dmg_kind);
	virtual Block * DropAfterDamage() const;

	virtual Inventory * HasInventory();
	virtual Animal * IsAnimal();
	virtual Active * ActiveBlock();

	virtual bool Armour() const;
	virtual bool IsWeapon() const;
	virtual bool Carving() const;
	virtual int DamageKind() const;
	virtual ushort DamageLevel() const;

	virtual uchar LightRadius() const;
	virtual int Temperature() const;
	virtual bool Inscribable() const;

	protected:
	virtual void SaveAttributes(QDataStream &) const;

	public:
	void Restore();
	void NullWeight(const bool null);
	float Weight() const;
	void SetDir(const int dir);

	int GetDir() const;
	int Sub() const;
	short Durability() const;
	QString & GetNote(QString & str) const;
	int Transparent() const;

	bool operator==(const Block &) const;

	void SaveToFile(QDataStream & out) const;

	Block(
			const int sb=STONE,
			const quint8 transp=UNDEF);
	Block(
			QDataStream &,
			const int sub_,
			const quint8 transp=UNDEF);
	virtual ~Block();
}; //class Block

class Plate : public Block {
	public:
	QString & FullName(QString & str) const;
	int Kind() const;
	Block * DropAfterDamage() const;
	int BeforePush(const int);
	float TrueWeight() const;

	Plate(const int sub);
	Plate(QDataStream & str, const int sub);
}; //class Plate

class Ladder : public Block {
	public:
	QString & FullName(QString & str) const;
	int Kind() const;
	Block * DropAfterDamage() const;
	int BeforePush(const int);
	float TrueWeight() const;
	bool Catchable() const;

	Ladder(const int sub);
	Ladder(QDataStream & str, const int sub);
}; //class Ladder

class Clock : public Block {
	public:
	int Kind() const;
	QString & FullName(QString & str) const;

	Block * DropAfterDamage() const;
	usage_types Use();
	int BeforePush(const int);
	float TrueWeight() const;

	Clock(const int sub);
	Clock (QDataStream & str, const int sub);
}; //class Clock

class Weapon : public Block {
	public:
	QString & FullName(QString & str) const;
	int Kind() const;
	ushort DamageLevel() const;
	int BeforePush(const int);

	float TrueWeight() const;
	bool IsWeapon() const;
	bool CanBeOut() const;

	Weapon(const int sub);
	Weapon(QDataStream & str, const int sub);
}; //class Weapon

class Pick : public Weapon {
	public:
	int Kind() const;
	int DamageKind() const;
	ushort DamageLevel() const;
	QString & FullName(QString & str) const;

	bool Carving() const;
	float TrueWeight() const;

	Pick(const int sub);
	Pick(QDataStream & str, const int sub);
}; //class Pick

class Active : public QObject, public Block {
	Q_OBJECT;

	quint8 fall_height;
	bool falling;

	protected:
	quint8 timeStep;
	///coordinates in loaded world zone
	ushort x_self, y_self, z_self;
	Shred * whereShred;

	signals:
	void Moved(int);
	void Destroyed();
	void Updated();

	public:
	World * GetWorld() const;
	QString & FullName(QString & str) const;
	int Kind() const;

	Active * ActiveBlock();
	int Move(const int);
	void SetNotFalling();
	bool IsFalling() const;

	ushort X() const;
	ushort Y() const;
	ushort Z() const;

	///Block's  action, called every game turn.
	/**
	 * Active::Act() should be mentioned in derived classes'
	 * Act() (if reimplemented) before the end.
	 * This makes common damage from falling.
	 */
	virtual void Act();

	int Movable() const;
	virtual bool ShouldFall() const;
	before_move_return BeforeMove(const int);
	int Damage(const ushort dmg, const int dmg_kind);

	void ReloadToNorth();
	void ReloadToSouth();
	void ReloadToWest();
	void ReloadToEast();

	void SaveAttributes(QDataStream & out) const;

	void Register(
			Shred *,
			const ushort x,
			const ushort y,
			const ushort z);
	void Unregister();

	Active(const int sub, const quint8 transp=UNDEF);
	Active(
			QDataStream & str,
			const int sub,
			const quint8 transp=UNDEF);
	~Active();
}; //class Active

class Animal : public Active {
	Q_OBJECT;

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
			const int sub=A_MEAT)
			:
			Active(sub, NONSTANDARD),
			breath(MAX_BREATH),
			satiation(SECONDS_IN_DAY)
	{}
	Animal(
			QDataStream & str,
			const int sub);
}; //class Animal

class Inventory {
	static const ushort max_stack_size=9;
	const ushort size;
	QStack<Block *> * inventory;

	public:
	virtual QString & FullName(QString&) const=0;
	virtual int Kind() const=0;
	virtual int Sub() const=0;
	virtual float TrueWeight() const=0;
	virtual bool Access() const;
	virtual ushort Start() const;
	virtual Inventory * HasInventory();
	virtual int Drop(const ushort num, Inventory * const inv_to);
	virtual bool Get(Block * const block);
	virtual int GetAll(Inventory * const from);
	virtual usage_types Use();
	virtual void Pull(const ushort num);
	virtual void SaveAttributes(QDataStream & out) const;

	ushort Size() const;
	bool GetExact(Block * const block, const ushort num);
	int MiniCraft(const ushort num);
	int InscribeInv(const ushort num, const QString & str);
	QString & InvFullName(QString & str, const ushort i) const;
	QString & NumStr(QString & str, const ushort i) const;
	float GetInvWeight(const ushort i) const;
	int GetInvSub(const ushort i) const;
	int GetInvKind(const ushort i) const;
	QString & GetInvNote(QString & str, const ushort num) const;
	float InvWeightAll() const;
	quint8 Number(const ushort i) const;
	Block * ShowBlock(const ushort num) const;

	bool HasRoom();
	bool IsEmpty();

	//it is not recommended to make inventory size more than 26,
	//because it will not be convenient to deal with inventory
	//in console version.
	Inventory(const ushort sz=26);
	Inventory(QDataStream & str, const ushort size=26);
	~Inventory();
}; //class Inventory

class Dwarf : public Animal, public Inventory {
	Q_OBJECT;

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
		return str="Dwarf "+note;
	}
	float TrueWeight() const;
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
			case HAZELNUT: satiation+=SECONDS_IN_HOUR/2; break;
			case H_MEAT:   satiation+=SECONDS_IN_HOUR*2.5; break;
			case A_MEAT:   satiation+=SECONDS_IN_HOUR*2; break;
			default: return 2; //not ate
		}

		if ( SECONDS_IN_DAY < satiation )
			satiation=1.1*SECONDS_IN_DAY;

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

	Dwarf(const int sub=H_MEAT) :
			Animal(sub),
			Inventory()
	{
		Inscribe("Urist");
	}
	Dwarf(
			QDataStream & str,
			const int sub)
		:
			Animal(str, sub),
			Inventory(str)
	{}
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

	Block * DropAfterDamage() const;
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
			const int s=WOOD)
			:
			Block(s),
			Inventory()
	{}
	Chest(
			QDataStream & str,
			const int sub)
			:
			Block(str, sub),
			Inventory(str)
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

	void SaveAttributes(QDataStream & out) const {
		Active::SaveAttributes(out);
		Inventory::SaveAttributes(out);
		out << ifToDestroy;
	}

	Pile(const int sub=DIFFERENT) :
			Active(sub, NONSTANDARD),
			Inventory(),
			ifToDestroy(false)
	{}
	Pile(QDataStream & str, int sub);
}; //class Pile

class Liquid : public Active {
	Q_OBJECT;

	//return true if there is water near
	bool CheckWater() const;

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

	Liquid(const int sub=WATER) :
			Active(sub)
	{}
	Liquid(
			QDataStream & str,
			const int sub)
			:
			Active(str, sub)
	{}
}; //class Liquid

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
	void Act();

	Grass(const int sub=GREENERY) :
			Active(sub)
	{}
	Grass(
			QDataStream & str,
			const int sub)
		:
			Active(str, sub)
	{}
}; //class Grass

class Bush : public Active, public Inventory {
	Q_OBJECT;

	static const ushort bush_size=3;

	public:
	int   Kind() const;
	int   Sub() const;
	int   Movable() const;
	float TrueWeight() const;
	void  Act();

	QString & FullName(QString & str) const;
	usage_types Use();
	Inventory * HasInventory();
	Block * DropAfterDamage() const;

	void SaveAttributes(QDataStream & out) const;

	Bush(const int sub=WOOD);
	Bush(QDataStream & str, const int sub);
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
			satiation+=SECONDS_IN_HOUR*4;
			return 1;
		}
		return 0;
	}

	Block * DropAfterDamage() const;

	Rabbit(const int sub=A_MEAT) :
			Animal(sub)
	{}
	Rabbit(
			QDataStream & str,
			const int sub)
		:
			Animal(str, sub)
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
	Block * DropAfterDamage() const;
	Inventory * HasInventory() { return Inventory::HasInventory(); }

	int Sub() const { return Block::Sub(); }
	ushort Start() const { return 1; }

	int Drop(const ushort num, Inventory * const inv_to);
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

	Workbench(const int sub) :
			Block(sub),
			Inventory(workbench_size)
	{}
	Workbench(
			QDataStream & str,
			const int sub)
			:
			Block(str, sub),
			Inventory(str, workbench_size)
	{}
}; //class Workbench

class Door : public Active {
	Q_OBJECT;

	bool shifted;
	bool locked;
	int movable;

	public:
	int Kind() const { return DOOR; }
	QString & FullName(QString & str) const {
		QString sub_string;
		switch ( Sub() ) {
			case WOOD:  sub_string=" of wood";   break;
			case STONE: sub_string=" of stone";  break;
			case GLASS: sub_string=" of glass";  break;
			case IRON:  sub_string=" of iron";   break;
			default:
				sub_string=" of something";
				fprintf(stderr,
					"Door::FullName: unlisted sub: %d\n",
					Sub());
		}
		return str=QString((locked ? "Locked door" : "Door")) + sub_string;
	}
	int Movable() const { return movable; }
	usage_types Use() {
		locked=locked ? false : true;
		return NO;
	}
	int BeforePush(const int dir);
	void Act();
	Block * DropAfterDamage() const;

	void SaveAttributes(QDataStream & out) const {
		Active::SaveAttributes(out);
		out << shifted << locked;
	}

	Door(const int sub) :
			Active(sub, ( STONE==sub ) ? BLOCK_OPAQUE : NONSTANDARD),
			shifted(false),
			locked(false),
			movable(NOT_MOVABLE)
	{}
	Door(
			QDataStream & str,
			const int sub);
}; //class Door

#endif
