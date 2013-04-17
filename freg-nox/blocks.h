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
class QTextStream;
class QString;
class World;
class Shred;
class Inventory;
class Active;
class Animal;

class Block { //blocks without special physics and attributes
	void SetTransparency(quint8 transp);
	virtual float TrueWeight() const;

	protected:
	quint8 transparent;
	quint16 sub;
	bool nullWeight;
	quint8 direction;
	QString * note;
	qint16 durability;

	public:
	virtual QString & FullName(QString & str) const;
	virtual int Kind() const;
	virtual bool Catchable() const;
	virtual bool CanBeOut() const;
	virtual int Movable() const;
	///Returns false if block cannot be inscribed, otherwise true.
	virtual void Inscribe(const QString & str);
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
	virtual int DamageKind() const;
	virtual ushort DamageLevel() const;

	virtual uchar LightRadius() const;
	virtual int Temperature() const;
	virtual void ReceiveSignal(const QString &);

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

	Block(const int sb=STONE, const quint8 transp=UNDEF);
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

	float TrueWeight() const;

	Pick(const int sub);
	Pick(QDataStream & str, const int sub);
}; //class Pick

class Active : public QObject, public Block {
	Q_OBJECT

	quint8 fall_height;
	bool falling;

	protected:
	quint8 timeStep;
	///coordinates in loaded world zone
	ushort x_self, y_self, z_self;
	Shred * whereShred;

	///Returns true each second, not every turn.
	bool IsActiveTurn();
	void SendSignalAround(const QString &) const;

	signals:
	void Moved(int);
	void Destroyed();
	void Updated();
	void ReceivedText(const QString &);

	public:
	World * GetWorld() const;
	bool InBounds(ushort x, ushort y, ushort z=0) const;
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
	void ReceiveSignal(const QString &);

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
	Q_OBJECT

	protected:
	quint16 breath;
	quint16 satiation;

	public:
	QString & FullName(QString&) const=0;

	ushort Breath() const;
	ushort Satiation() const;
	virtual int Eat(Block * const)=0;

	void Act();

	void SaveAttributes(QDataStream & out) const;

	Animal * IsAnimal();

	Animal(const int sub=A_MEAT);
	Animal(QDataStream & str, const int sub);
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
	Q_OBJECT

	static const uchar onHead=0;
	static const uchar inRight=1;
	static const uchar inLeft=2;
	static const uchar onBody=3;
	static const uchar onLegs=4;

	public:
	int Kind() const;
	int Sub() const;
	QString & FullName(QString & str) const;
	float TrueWeight() const;
	ushort Start() const;
	int DamageKind() const;
	ushort DamageLevel() const;

	before_move_return BeforeMove(const int);
	void Act();
	int Eat(Block * const to_eat);

	Inventory * HasInventory();
	bool Access() const;
	int Wield(const ushort num);
	Block * DropAfterDamage() const;
	void Inscribe(const QString & str);

	void SaveAttributes(QDataStream & out) const;

	uchar LightRadius() const;

	Dwarf(const int sub=H_MEAT);
	Dwarf(QDataStream & str, const int sub);
}; //class Dwarf

class Chest : public Block, public Inventory {
	public:
	int Kind() const;
	int Sub() const;
	QString & FullName(QString & str) const;
	Inventory * HasInventory();

	usage_types Use();

	Block * DropAfterDamage() const;
	float TrueWeight() const;

	void SaveAttributes(QDataStream & out) const;

	Chest(const int s=WOOD);
	Chest(QDataStream & str, const int sub);
}; //class Chest

class Pile : public Active, public Inventory {
	Q_OBJECT

	bool ifToDestroy;

	public:
	int Kind() const;
	int Sub() const;
	QString & FullName(QString & str) const;

	Inventory * HasInventory();
	usage_types Use();
	float TrueWeight() const;

	void Act();

	before_move_return BeforeMove(const int dir);
	int Drop(const ushort n, Inventory * const inv);
	void Pull(const ushort num);

	void SaveAttributes(QDataStream & out) const;

	Pile(const int sub=DIFFERENT);
	Pile(QDataStream & str, int sub);
}; //class Pile

class Liquid : public Active {
	Q_OBJECT

	//return true if there is water near
	bool CheckWater() const;

	public:
	int Movable() const;

	int Kind() const;
	QString & FullName(QString & str) const;

	int Damage(const ushort dam, const int dam_kind);

	void Act();

	int Temperature() const;

	Liquid(const int sub=WATER);
	Liquid(QDataStream & str, const int sub);
}; //class Liquid

class Grass : public Active {
	Q_OBJECT

	public:
	QString & FullName(QString & str) const;
	void Act();
	int  Kind() const;
	bool ShouldFall() const;
	before_move_return BeforeMove(const int);

	Grass(const int sub=GREENERY);
	Grass(QDataStream & str, const int sub);
}; //class Grass

class Bush : public Active, public Inventory {
	Q_OBJECT

	static const ushort bush_size=3;

	public:
	int   Kind() const;
	int   Sub() const;
	int   Movable() const;
	float TrueWeight() const;
	bool  ShouldFall() const;
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
	QString & FullName(QString & str) const;
	int Kind() const;
	void Act();
	float TrueWeight() const;
	int Eat(Block * const to_eat);
	Block * DropAfterDamage() const;

	Rabbit(const int sub=A_MEAT);
	Rabbit(QDataStream & str, const int sub);
}; //class Rabbit

class Workbench : public Block, public Inventory {
	static const ushort workbench_size=10;

	void Craft();

	public:
	QString & FullName(QString & str) const;
	int Kind() const;
	float TrueWeight() const;
	usage_types Use();
	Block * DropAfterDamage() const;
	Inventory * HasInventory();
	int Sub() const;
	ushort Start() const;
	int Drop(const ushort num, Inventory * const inv_to);
	bool Get(Block * const block);
	int GetAll(Inventory * const from);

	void SaveAttributes(QDataStream & out) const;

	Workbench(const int sub);
	Workbench(QDataStream & str, const int sub);
}; //class Workbench

class Door : public Active {
	Q_OBJECT

	bool shifted;
	bool locked;
	int movable;

	public:
	void Act();
	int  Kind() const;
	int  Movable() const;
	int  BeforePush(const int dir);
	bool ShouldFall() const;
	QString & FullName(QString & str) const;
	usage_types Use();
	Block * DropAfterDamage() const;

	void SaveAttributes(QDataStream & out) const;

	Door(const int sub);
	Door(QDataStream & str, const int sub);
}; //class Door

class Clock : public Active {
	QTextStream * txtStream;
	short alarmTime;
	short timerTime;

	public:

	void  Act();
	int   Kind() const;
	int   Movable() const;
	bool  ShouldFall() const;
	int   BeforePush(const int);
	float TrueWeight() const;
	void  Inscribe(const QString & str);
	Block * DropAfterDamage() const;
	usage_types Use();
	QString & FullName(QString & str) const;

	Clock(const int sub);
	Clock (QDataStream & str, const int sub);
	~Clock();
}; //class Clock

#endif
