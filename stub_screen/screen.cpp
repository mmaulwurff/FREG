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

//this file is related to text (stub) screen for freg.

#include "screen.h"
#include "world.h"
#include "i_thread.h"
#include "Player.h"

Screen::Screen(
		World * const world,
		Player * const pl)
		:
		VirtScreen(world, pl)
{
	input=new IThread();
	connect(input, SIGNAL(RePrintReceived()),
		this, SLOT(RePrint()),
		Qt::DirectConnection);
	connect(input, SIGNAL(ExitReceived()),
		this, SIGNAL(ExitReceived()));
	connect(input, SIGNAL(InputReceived(int, int)),
		this, SIGNAL(InputReceived(int, int)),
		Qt::DirectConnection);
	connect(this, SIGNAL(InputReceived(const int, const int)),
		player, SLOT(Act(const int, const int)),
		Qt::DirectConnection);
	input->start();
	Notify("Game loaded.\n");
}
