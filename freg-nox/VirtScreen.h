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

/** \file
 * \brief VirtScreen class declaration.
 */

#ifndef VIRTSCREEN_H
#define VIRTSCREEN_H

#include <QObject>

class QString;
class World;
class Player;

class VirtScreen : public QObject {
	/** \class VirtScreen VirtScreen.h
		* \brief This class provides base for all screens for freg.
		*
		* It provides interface for world-screen and player-screen
		* communications by its slots and signals.
	*/

	Q_OBJECT;
	
	enum window_views { NORMAL, FRONT, INVENTORY };

	protected:
	///world to print
	World * const w;
	///player to print (HP, inventory, etc)
	Player * const player;

	private slots:
	///Prints world. Should not be called not within screen.
	virtual void Print()=0;

	public slots:
	///This is called for a notification to be displayed.
	virtual void Notify(const QString &)=0;

	///This is called when program is stopped and from destructor.
	/**
	 * When implemented, this should contain check to prevent double cleaning.
	 */
	virtual void CleanAll();

	///This is called when string is needed to be received from input.
	/**
	 * It is connected to world in constructor.
	 */
	virtual QString & PassString(QString &) const=0;

	///This is called when block at (x, y, z) should be updated in screen.
	/**
	 * When implemented, this should work fast.
	 * It is connected to world in constructor.
	 */
	virtual void Update(const ushort x, const ushort y, const ushort z)=0;

	///This is called when all world should be updated in sceen.
	/**
	 * When implemented, this should work fast.
	 * It is connected to world in constructor.
	 */
	virtual void UpdateAll()=0;

	///This is called when loaded zone of world is moved to update world in screen properly.
	/**
	 * When implemented, this should work fast.
	 * It is connected to world in constructor.
	 */
	virtual void Move(const int)=0;

	///This is called when some player property are needed to be updated in screen.
	/**
	 * When implemented, this should work fast.
	 * It is connected to world in constructor.
	 */
	virtual void UpdatePlayer()=0;
	
	///This is called when area around x, y, z with range is needed to be updated in screen.
	/**
	 * When implemented, this should work fast.
	 * It is connected to world in constructor.
	 */
	virtual void UpdateAround(
			const ushort x, const ushort y, const ushort z,
			const ushort range)=0;

	///This is called to restore some connections.
	/**
	 * This restores connections to VirtScreen::Update and
	 * VirtScreen::UpdateAround which can be temporarily
	 * disconnected.
	 */
	void ConnectWorld();

	///This is called when current group of updates is ended.
	/**
	 * This is called from world when pack of world changing is ended.
	 * ( Can be used in screen optimization. )
	 */
	virtual void UpdatesEnd();

	signals:
	///This is emitted when input receives exit key.
	/**
	 * This is connected to application exit.
	 */
	void ExitReceived();

	public:
	///Constructor makes player and world connections.
	/**
	 * Constructor of non-virtual screen should contain this code
	 * to connect to player for sending input:
	 * connect(this, SIGNAL(InputReceived(int, int)),
	 * 	player, SLOT(Act(int, int)),
	 * 	Qt::DirectConnection);
	 */
	VirtScreen(World * const, Player * const);
	///Destructor only calls VirtScreen::CleanAll, not needed to be reimplemented.
	virtual ~VirtScreen();
}; //class VirtScreen

#endif
