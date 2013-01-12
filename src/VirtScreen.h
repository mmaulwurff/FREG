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

#ifndef VIRTSCREEN_H
#define VIRTSCREEN_H

#include <QObject>

enum window_views { NORMAL, FRONT, INVENTORY };

class QString;
class World;
class Player;

class VirtScreen : public QObject {
	Q_OBJECT
	
	protected:
	World * const w;
	Player * const player;

	public slots:
	virtual void Print()=0;
	virtual void Notify(QString)=0;
	virtual void CleanAll() {};
	virtual void PassString(QString &) const=0;
	virtual void Update(
			const ushort,
			const ushort,
			const ushort)=0;
	virtual void UpdateAll()=0;
	virtual void UpdatePlayer()=0;
	virtual void UpdateAround(
			const ushort,
			const ushort,
			const ushort,
			const ushort level)=0;
	virtual void RePrint() {}

	signals:
	void ExitReceived();
	void InputReceived(int, int) const;

	public:
	VirtScreen(
			World * const,
		       	Player * const);
	virtual ~VirtScreen() { CleanAll(); }
};

#endif
