#ifndef SCREEN_H
#define SCREEN_H

#include <curses.h>

class Screen {
	World * const w; //connected world
	WINDOW * leftWin,
	       * rightWin,
	       * notifyWin,
	       * soundWin;
	char CharName(unsigned short i, unsigned short j, unsigned short k);
	void PrintInv();
	public:
	special_views view;
	Screen(World *wor);
	~Screen();
	color_pairs Color(kinds, subs);
	color_pairs Color(unsigned short i, unsigned short j, unsigned short k);
	color_pairs Color(subs sub, kinds kind) { return Screen::Color(kind, sub); }
	void Print();
	void Notify(const char *);
	void PrintSounds();
	void InvOnOff() {
		if (NONE==view) {
			view=INVENTORY;
			werase(rightWin);
			wrefresh(rightWin);
		} else
			view=NONE;
	}
	void UpDownView(dirs);
};

#endif
