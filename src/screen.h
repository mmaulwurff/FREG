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

#define NOX
#define SCREEN_SIZE 30

#include "VirtScreen.h"
#include <curses.h>

enum color_pairs { //do not change colors order! //foreground_background
        BLACK_BLACK=1,
        BLACK_RED,
        BLACK_GREEN,
        BLACK_YELLOW,
        BLACK_BLUE,
        BLACK_MAGENTA,
        BLACK_CYAN,
        BLACK_WHITE,
        //
        RED_BLACK,
        RED_RED,
        RED_GREEN,
        RED_YELLOW,
        RED_BLUE,
        RED_MAGENTA,
        RED_CYAN,
        RED_WHITE,
        //
        GREEN_BLACK,
        GREEN_RED,
        GREEN_GREEN,
        GREEN_YELLOW,
        GREEN_BLUE,
        GREEN_MAGENTA,
        GREEN_CYAN,
        GREEN_WHITE,
        //
        YELLOW_BLACK,
        YELLOW_RED,
        YELLOW_GREEN,
        YELLOW_YELLOW,
        YELLOW_BLUE,
        YELLOW_MAGENTA,
        YELLOW_CYAN,
        YELLOW_WHITE,
        //
        BLUE_BLACK,
        BLUE_RED,
        BLUE_GREEN,
        BLUE_YELLOW,
        BLUE_BLUE,
        BLUE_MAGENTA,
        BLUE_CYAN,
        BLUE_WHITE,
        //
	MAGENTA_BLACK,
        MAGENTA_RED,
        MAGENTA_GREEN,
        MAGENTA_YELLOW,
        MAGENTA_BLUE,
        MAGENTA_MAGENTA,
        MAGENTA_CYAN,
        MAGENTA_WHITE,
        //
        CYAN_BLACK,
        CYAN_RED,
        CYAN_GREEN,
        CYAN_YELLOW,
        CYAN_BLUE,
        CYAN_MAGENTA,
        CYAN_CYAN,
        CYAN_WHITE,
        //
        WHITE_BLACK,
        WHITE_RED,
        WHITE_GREEN,
        WHITE_YELLOW,
        WHITE_BLUE,
        WHITE_MAGENTA,
        WHITE_CYAN,
        WHITE_WHITE
};

class IThread;
class Block;
class Inventory;
class QTimer;

class Screen : public VirtScreen {
	Q_OBJECT

	WINDOW * leftWin,
	       * rightWin,
	       * notifyWin,
	       //* soundWin,
	       * hudWin; //head-up display
	IThread * input;
	volatile bool updated;
	bool cleaned;

	QTimer * timer;

	/*struct {
		char ch;
		unsigned short lev;
		color_pairs col;
	} soundMap[9];*/

	char CharName(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	char CharName(const int &, const int &) const;
	char CharNumber(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	char CharNumberFront(
			const unsigned short &,
			const unsigned short &) const;
	void Arrows(
			WINDOW * const & window,
			const unsigned short & x,
			const unsigned short & y) const
	{
		//стрелки, показывающие положение игрока.
		//используются, когда его самого не видно (например, вид неба)
		wcolor_set(window, WHITE_RED, NULL);
		mvwprintw(window, 0, x, "vv");
		mvwprintw(window, SCREEN_SIZE+1, x, "^^");
		mvwprintw(window, y, 0, ">");
		mvwprintw(window, y, SCREEN_SIZE*2+1, "<");	
	}
	FILE * notifyLog; //весь текст уведомлений (notification) дублируется в файл.
	unsigned short notifyLines; //для NotifyAdd() - сколько строк уже с текстом

	void PrintNormal(WINDOW * const) const;
	void PrintFront(WINDOW * const) const;
	void PrintInv(WINDOW * const, Inventory * const) const;

	color_pairs Color(
			const int & kind,
			const int & sub) const; //пара цветов текст_фон в зависимоти от типа (kind) и вещества (sub) блока.
	color_pairs Color(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;

	//void GetSound(const unsigned short, const unsigned short, const char, const kinds, const subs); //получить отдельный звук для звуковой карты
	//void PrintSounds();

	private slots:
	void Print();

	public slots:
	void Notify(QString);
	void CleanAll();
	void PassString(QString &) const;
	void Update(
			const unsigned short,
			const unsigned short,
			const unsigned short);
	void UpdateAll();
	void UpdatePlayer();
	void Flushinp() { flushinp(); }
	void RePrint() { //стереть всё с экрана и перерисовать всё с нуля (можно сделать пустой)
		wclear(leftWin);
		wclear(rightWin);
		wclear(notifyWin);
		//wclear(soundWin);
		wclear(hudWin);
		updated=false;
		//PrintSounds();
	}

	signals:
	void ExitReceived();
	void InputReceived(int, int) const;

	public:
	Screen(World * const, Player * const);
};

#endif
