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

/** \class Player Player.h
 * \brief This class contains information specific to player
 * and interface for manipulating him.
 *
 * It receives input from screen, processes it, and dispatches
 * actions to world.
 *
 * It loads player from file and saves him to it.
 *
 * Note: player block with its inventory is stored it shred
 * file with all other blocks. player_save contains only
 * coordinates where player is, his home coordinates and world
 * player belongs to.
 *
 * Also it does checks for player walking over the shred border.
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "header.h"
#include <QObject>

class QString;
class World;
class Block;
class Active;
class Animal;
class Inventory;
class Shred;

class Player : public QObject {
	Q_OBJECT

	ulong homeLongi, homeLati;
	short homeX, homeY, homeZ;
	short x, y, z; //current position
	int dir;
	World * const world;
	Active * player;
	int usingType;
	int usingSelfType;
	Shred * shred;

	bool creativeMode;

	volatile bool cleaned;

	void UpdateXYZ();

	public slots:

	/// For cleaning player-related data before exiting program.
	/**This is connected to app's aboutToQuit() signal, also it
	 * is called from destructor. There is a check for data deleting
	 * not to be called twice.
	 * Also this saves player to file player_save.
	 */
	void CleanAll();

	/// Checks if player walked over the shred border.
	/**This is connected to player's block signal Moved(int).
	 * It emits OverstepBorder(int) signal when player walks
	 * over the shred border.
	 */
	void CheckOverstep(const int);

	///This is called when player block is destroyed.
	void BlockDestroy();

	void WorldSizeReloadStart();
	void WorldSizeReloadFinish();

	void SetPlayer(
			const ushort set_x,
			const ushort set_y,
			const ushort set_z);

	signals:
	///This is emitted when a notification is needed to be displayed.
	/** It should be connected to screen::Notify(const QString &). */
	void Notify(const QString &) const;

	///This is emitted when player walks over shred border.
	/** It should be connected to World::ReloadShreds(int) signal. */
	void OverstepBorder(const int);

	///This is emitted when some player property is updated.
	/** It shoul be connected to screen::UpdatePlayer() signal. */
	void Updated();
	void GetString(QString &);

	public:
	///This returns current player block X position (coordinates in loaded zone)
	ushort X() const { return x; }

	///This returns current player block Y position (coordinates in loaded zone)
	ushort Y() const { return y; }

	///This returns current player block Z position (coordinates in loaded zone)
	ushort Z() const { return z; }

	///This returns current player direction (see enum dirs in header.h)
	int Dir() const;

	///This returns player hitpoints, also known as durability.
	short HP() const;

	///This returns player breath reserve.
	short Breath() const;

	short Satiation() const;

	///This returns player block itself.
	Active * GetP() const { return player; }

	///This returns true if block at (x, y, z) is visible to player.
	bool Visible(
		const ushort x,
		const ushort y,
		const ushort z) const;
	void Focus(
		ushort &,
		ushort &,
		ushort &) const;

	///This returns block which is now used by player.
	/** See enum usage_types in header.h. */
	Block * UsingBlock() const;

	///This returns how player is using something now.
	/** See enum usage_types in header.h. */
	int UsingType() const { return usingType; }

	//This returns how player is using himself.
	/** For example, OPEN means he is looking in his backpack. */
	int UsingSelfType() const { return usingSelfType; }

	Inventory * PlayerInventory();

	long GetLongitude() const;
	long GetLatitude() const;

	bool GetCreativeMode() const { return creativeMode; }
	void SetCreativeMode(const bool turn) { creativeMode=turn; }

	void Turn(const int dir);
	int Move(const int dir);
	void Jump();

	void Backpack();
	void Inscribe(
			const short x,
			const short y,
			const short z) const;
	void Examine(
			const short x,
			const short y,
			const short z) const;
	void Damage(
			const short x,
			const short y,
			const short z) const;
	void Use(
			const short x,
			const short y,
			const short z);

	void Use     (const ushort num);
	void Throw   (const ushort num);
	void Obtain  (const ushort num);
	void Wield   (const ushort num);
	void Inscribe(const ushort num);
	void Eat     (const ushort num);
	void Craft   (const ushort num);
	void TakeOff (const ushort num);
	void Build(
			const short x,
			const short y,
			const short z,
			const ushort num);
	void ProcessCommand(QString & command);

	private:
	Block * ValidBlock(const ushort num) const;
	int DamageKind() const;
	ushort DamageLevel() const;
	void Get(Block *);
	Block * Drop(const ushort);
	void Dir(const int dir);

	public:
	void SetNumShreds(ushort num) const;

	///Constructor creates or loads player.
	/**It reads player_save file if it exists,
	 * puts player block to the world if there is no player block,
	 * and makes necessary connections.
	 */
	Player(World * const);

	///Destructor calls Player::CleanAll().
	~Player() { CleanAll(); }
}; //Player

#endif
