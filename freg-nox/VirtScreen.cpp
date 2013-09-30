	/* freg, Free-Roaming Elementary Game with open and interactive world
	*  Copyright (C) 2012-2013 Alexander 'mmaulwurff' Kromm
	*  mmaulwurff@gmail.com
	*
	* This file is part of FREG.
	*
	* FREG is free software: you can redistribute it and/or modify
	* it under the terms of the GNU General Public License as published by
	* the Free Software Foundation, either version 3 of the License, or
	* (at your option) any later version.
	*
	* FREG is distributed in the hope that it will be useful,
	* but WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	* GNU General Public License for more details.
	*
	* You should have received a copy of the GNU General Public License
	* along with FREG. If not, see <http://www.gnu.org/licenses/>. */

/**\file VirtScreen.cpp
 * \brief This file provides definitions of code common to all freg screens. */

#include "VirtScreen.h"
#include "Player.h"
#include "world.h"

void VirtScreen::ConnectWorld() {
	connect(w, SIGNAL(Updated(ushort, ushort, ushort)),
		SLOT(Update(ushort, ushort, ushort)),
		Qt::DirectConnection);
	connect(w, SIGNAL(UpdatedAround(ushort, ushort, ushort, ushort)),
		SLOT(UpdateAround(ushort, ushort, ushort, ushort)),
		Qt::DirectConnection);
}

void VirtScreen::UpdatesEnd() {}
void VirtScreen::DeathScreen() {}

VirtScreen::VirtScreen(World * const world_, Player * const player_) :
		w(world_),
		player(player_)
{
	connect(w, SIGNAL(Notify(const QString &)),
		SLOT(Notify(const QString &)));
	connect(player, SIGNAL(Notify(const QString &)),
		SLOT(Notify(const QString &)));

	connect(w, SIGNAL(GetString(QString &)),
		SLOT(PassString(QString &)), Qt::DirectConnection);
	connect(player, SIGNAL(GetString(QString &)),
		SLOT(PassString(QString &)), Qt::DirectConnection);

	connect(player, SIGNAL(Updated()), SLOT(UpdatePlayer()),
		Qt::DirectConnection);
	connect(w, SIGNAL(ReConnect()), SLOT(ConnectWorld()),
		Qt::DirectConnection);
	connect(w, SIGNAL(UpdatedAll()), SLOT(UpdateAll()),
		Qt::DirectConnection);
	connect(w, SIGNAL(Moved(const int)), SLOT(Move(const int)),
		Qt::DirectConnection);
	ConnectWorld();
	connect(w, SIGNAL(UpdatesEnded()), SLOT(UpdatesEnd()),
		Qt::DirectConnection);

	connect(player, SIGNAL(Destroyed()), SLOT(DeathScreen()),
		Qt::DirectConnection );
}

void VirtScreen::CleanAll() {}
VirtScreen::~VirtScreen() { CleanAll(); }
