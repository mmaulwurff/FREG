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

	ulong homeLongi, homeLati;
	ushort homeX, homeY, homeZ;
	ushort x, y, z; //current position
	int dir;
	World * const world;
	Active * player;
	Block * usingBlock;
	usage_types usingType;
	usage_types usingSelfType;
	Shred * shred;

	volatile bool cleaned;

	void UpdateXYZ();

	public slots:
	/// For cleaning player-related data before exiting program.
	/**
	 * This is connected to app's aboutToQuit() signal, also it
	 * is called from destructor. There is a check for data deleting
	 * not to be called twice.
	 */
	void CleanAll();
	/// Checks if player walked over the shred border.
	/**
	 * This is connected to player's block signal Moved(int).
	 * It emits OverstepBorder(int) signal when player walks
	 * over the shred border.
	 */
	void CheckOverstep(const int);
	void Act(const int, const int);
	void BlockDestroy() { player=0; }
	
	signals:
	void Notify(QString) const;
	void OverstepBorder(int);
	void Updated();

	public:
	ushort X() const { return x; }
	ushort Y() const { return y; }
	ushort Z() const { return z; }
	int Dir() const;
	short HP() const;
	short Breath() const;
	short Satiation() const;
	Active * GetP() const { return player; }
	bool Visible(
		const ushort,
		const ushort,
		const ushort) const;
	Block * UsingBlock() { return usingBlock; }
	usage_types UsingType() { return usingType; }
	usage_types UsingSelfType() { return usingSelfType; }
	Inventory * PlayerInventory();

	private:
	void Focus(
		ushort &,
		ushort &,
		ushort &) const;
	void Examine() const;
	int Move(const int dir);
	int Move();
	void Jump();
	void Dir(const int dir);
	void Build(const ushort);
	void Eat(ushort);
	void Inscribe();
	Block * Drop(const ushort);
	void Get(Block *);
	void Wield();

	public:
	Player(World * const);
	~Player() { CleanAll(); }
};
