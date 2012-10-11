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
#include <curses.h>

class World;

class Screen {
	World * const w; //connected world
	WINDOW * leftWin,
	       * rightWin,
	       * notifyWin,
	       * soundWin,
	       * hudWin; //окно для жизни, дыхания и т.д.

	struct {
		char ch;
		unsigned short lev;
		color_pairs col;
	} soundMap[9];

	char CharName(const unsigned short, const unsigned short, const unsigned short) const; //вернуть символ, обозначающий блок
	char CharName(const kinds, const subs) const;
	void Arrows(WINDOW * const window, const unsigned short x, const unsigned short y) const {
		//стрелки, показывающие положение игрока.
		//используются, когда его самого не видно (например, вид неба)
		wcolor_set(window, WHITE_RED, NULL);
		mvwprintw(window, 0, x, "vv");
		mvwprintw(window, shred_width*3+1, x, "^^");
		mvwprintw(window, y, 0, ">");
		mvwprintw(window, y, shred_width*3*2+1, "<");	
	}
	FILE * notifyLog; //весь текст уведомлений (notification) дублируется в файл.
	unsigned short notifyLines; //для NotifyAdd() - сколько строк уже с текстом

	void PrintNormal(WINDOW * const) const;
	void PrintFront(WINDOW * const) const;
	void PrintInv(WINDOW * const, Inventory * const) const;

	color_pairs Color(const kinds, const subs) const; //пара цветов текст_фон в зависимоти от типа (kind) и вещества (sub) блока.
	color_pairs Color(const subs sub, const kinds kind) const { return Screen::Color(kind, sub); } //чтобы можно было путать порядок
	color_pairs Color(const unsigned short i, const unsigned short j, const unsigned short k) const; //в зависимости от координаты

	public:
	Block * blockToPrintLeft,
	      * blockToPrintRight; //блоки, связанные с экраном (например, блок открытого в текущий момент сундука)
	window_views viewLeft, viewRight; //тип вида: пока или NORMAL, или FRONT, или INVENTORY

	int Getch() { return getch(); }
	char * GetString(char * const str) const { //ввод строки пользователем
		echo();
		werase(notifyWin);
		mvwaddstr(notifyWin, 0, 0, "Enter inscription:");
		wmove(notifyWin, 1, 0);
		wgetnstr(notifyWin, str, note_length);
		werase(notifyWin);
		wrefresh(notifyWin);
		noecho();
		return str;
	}
	void Notify(const char * const str, const kinds kind=BLOCK, const subs sub=DIFFERENT) { //новое уведомление
		werase(notifyWin);
		notifyLines=0;
		NotifyAdd(str, kind, sub);
	}
	void NotifyAdd(const char * const, const kinds=BLOCK, const subs=DIFFERENT); //добавить строку к уведомлению
	void Print() const;

	void GetSound(const unsigned short, const unsigned short, const char, const kinds, const subs); //получить отдельный звук для звуковой карты
	void PrintSounds();

	void RePrint() { //стереть всё с экрана и перерисовать всё с нуля (можно сделать пустой)
		wclear(leftWin);
		wclear(rightWin);
		wclear(notifyWin);
		wclear(soundWin);
		wclear(hudWin);
		Print();
		PrintSounds();
	}

	Screen(World * const wor);
	~Screen();
};

#endif
