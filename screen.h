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

#ifndef SCREEN_H
#define SCREEN_H

#include "blocks.h"

class World;

class Screen {
	World * const w; //connected world
	WINDOW * leftWin,
	       * rightWin,
	       * notifyWin,
	       * soundWin;
	char CharName(const unsigned short, const unsigned short, const unsigned short) const;
	char CharName(const kinds, const subs) const;
	void Arrows(WINDOW * const window, const unsigned short x, const unsigned short y) const {
		wcolor_set(window, WHITE_RED, NULL);
		mvwprintw(window, 0, x, "vv");
		mvwprintw(window, shred_width*3+1, x, "^^");
		mvwprintw(window, y, 0, ">");
		mvwprintw(window, y, shred_width*3*2+1, "<");	
	}
	FILE * notifyLog;

	public:
	Block * blockToPrintLeft,
	      * blockToPrintRight;
	window_views viewLeft, viewRight;
	Screen(World * const wor);
	~Screen();
	color_pairs Color(const kinds, const subs) const;
	color_pairs Color(const unsigned short i, const unsigned short j, const unsigned short k) const;
	color_pairs Color(const subs sub, const kinds kind) const { return Screen::Color(kind, sub); }
	void PrintNormal(WINDOW * const) const;
	void PrintFront(WINDOW * const) const;
	void PrintInv(WINDOW * const, Inventory * const) const;
	void Print() const {
		switch (viewLeft) {
			case INVENTORY: if (NULL!=blockToPrintLeft) {
				PrintInv( leftWin, (Inventory *)blockToPrintLeft->HasInventory() );
				break;
			}
			case NORMAL: PrintNormal(leftWin); break;
			case FRONT: PrintFront(leftWin); break;
		}	
		switch (viewRight) {
			case INVENTORY: if (NULL!=blockToPrintRight) {
				PrintInv( rightWin, (Inventory *)blockToPrintRight->HasInventory() );
				break;
			}
			case NORMAL: PrintNormal(rightWin); break;
			case FRONT: PrintFront(rightWin); break;
		}	
	}
	void GetString(char * const str) const {
		echo();
		werase(notifyWin);
		box(notifyWin, 0, 0);
		mvwaddstr(notifyWin, 0, 1, "Enter inscription:");
		wmove(notifyWin, 1, 1);
		wgetnstr(notifyWin, str, note_length);
		werase(notifyWin);
		box(notifyWin, 0, 0);
		wrefresh(notifyWin);
		noecho();
	}
	void Notify(const char * const) const;
	void PrintSounds() const;
	void UpDownView(const dirs) const;
};

#endif
