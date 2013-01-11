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

#include "VirtScreen.h"
#include "Player.h"
#include "world.h"

VirtScreen::VirtScreen(
		World * const world_,
		Player * const player_)
		:
		w(world_),
		player(player_)
{
	connect(w, SIGNAL(Notify(QString)),
		this, SLOT(Notify(QString)));
	connect(player, SIGNAL(Notify(QString)),
		this, SLOT(Notify(QString)));

	connect(w, SIGNAL(GetString(QString &)),
		this, SLOT(PassString(QString &)),
		Qt::DirectConnection);

	connect(player, SIGNAL(Updated()),
		this, SLOT(UpdatePlayer()),
		Qt::DirectConnection);
	connect(w, SIGNAL(Updated(
			const unsigned short,
			const unsigned short,
			const unsigned short)),
		this, SLOT(Update(
			const unsigned short,
			const unsigned short,
			const unsigned short)),
		Qt::DirectConnection);
	connect(w, SIGNAL(UpdatedAll()),
		this, SLOT(UpdateAll()),
		Qt::DirectConnection);
}
