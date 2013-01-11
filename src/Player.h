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

#include <header.h>
#include <QString>
#include <QObject>

class World;
class Block;
class Active;
class Animal;
class Inventory;
class Shred;

class Player : public QObject {
	Q_OBJECT

	unsigned long homeLongi, homeLati;
	unsigned short homeX, homeY, homeZ;
	unsigned short x, y, z; //current position
	World * world;
	Active * player;
	Block * usingBlock;
	usage_types usingType;
	usage_types usingSelfType;
	Shred * shred;

	volatile bool cleaned;

	void UpdateXYZ();

	public slots:
	void CleanAll();
	void CheckOverstep(int);
	void Act(int, int);
	
	signals:
	void Notify(QString) const;
	void OverstepBorder(int);
	void Updated();

	public:
	Block * UsingBlock() { return usingBlock; }
	usage_types UsingType() { return usingType; }
	usage_types UsingSelfType() { return usingSelfType; }
	void Focus(
		unsigned short &,
		unsigned short &,
		unsigned short &) const;
	void Examine() const;
	int Move(const dirs dir);
	int Move();
	void Jump();
	void Dir(const dirs);
	dirs Dir() const;
	unsigned short X() const { return x; }
	unsigned short Y() const { return y; }
	unsigned short Z() const { return z; }
	Active * GetP() const { return player; }
	void Build(const unsigned short);
	void Eat(unsigned short);
	void Inscribe();
	Block * Drop(const unsigned short);
	void Get(Block *);
	void Wield();
	bool Visible(
		const unsigned short &,
		const unsigned short &,
		const unsigned short &) const;

	short HP() const;
	short Breath() const;
	short Satiation() const;
	Inventory * PlayerInventory();

	Player(World * const);
	~Player() { CleanAll(); }
};
