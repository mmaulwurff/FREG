#ifndef SCREEN_H
#define SCREEN_H

class Screen {
	World * const w; //connected world
	WINDOW * leftWin,
	       * rightWin,
	       * notifyWin,
	       * soundWin;
	char CharName(unsigned short i, unsigned short j, unsigned short k);
	public:
	Inventory * invToPrintLeft,
	          * invToPrintRight;
	window_views viewLeft, viewRight;
	Screen(World *wor);
	~Screen();
	color_pairs Color(kinds, subs);
	color_pairs Color(unsigned short i, unsigned short j, unsigned short k);
	color_pairs Color(subs sub, kinds kind) { return Screen::Color(kind, sub); }
	void PrintNormal(WINDOW *);
	void PrintFront(WINDOW *);
	void PrintInv(WINDOW *, Inventory *);
	void Print() {
		switch (viewLeft) {
			case INVENTORY: if (NULL!=invToPrintLeft) {
				PrintInv(leftWin, invToPrintLeft);
				break;
			}
			case NORMAL: PrintNormal(leftWin); break;
			case FRONT: PrintFront(leftWin); break;
		}	
		switch (viewRight) {
			case INVENTORY: if (NULL!=invToPrintRight) {
				PrintInv(rightWin, invToPrintRight);
				break;
			}
			case NORMAL: PrintNormal(rightWin); break;
			case FRONT: PrintFront(rightWin); break;
		}	
	}
	void GetString(char * str) {
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
	void Notify(const char *);
	void PrintSounds();
	void UpDownView(dirs);
};

#endif
