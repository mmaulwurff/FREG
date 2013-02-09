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

//this file provides curses (text-based graphics interface) screen for freg.
//screen.cpp provides definitions for methods.

#ifndef SCREEN_H
#define SCREEN_H

#define NOX
#define SCREEN_SIZE 30

#include "VirtScreen.h"
#include <curses.h>

enum actions {
	USE,
	THROW,
	OBTAIN, 
	WIELD,
	INSCRIBE,
	EAT,
	BUILD,
	CRAFT,
	TAKEOFF,
};
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
	       * hudWin; //head-up display
	IThread * input;
	volatile bool updated;
	bool cleaned;
	QTimer * timer;
	FILE * notifyLog; //весь текст уведомлений (notification) дублируется в файл.
	int actionMode;
	
	char CharName(
			const ushort,
			const ushort,
			const ushort) const;
	char CharName(const int, const int) const;
	char CharNumber(
			const ushort,
			const ushort,
			const ushort) const;
	char CharNumberFront(
			const ushort,
			const ushort) const;
	void Arrows(
			WINDOW * const & window,
			const ushort x,
			const ushort y) const
	{
		wcolor_set(window, WHITE_RED, NULL);
		mvwprintw(window, 0, x, "vv");
		mvwprintw(window, SCREEN_SIZE+1, x, "^^");
		mvwprintw(window, y, 0, ">");
		mvwprintw(window, y, SCREEN_SIZE*2+1, "<");	
	}

	void PrintNormal(WINDOW * const) const;
	void PrintFront(WINDOW * const) const;
	void PrintInv(WINDOW * const, Inventory * const) const;

	color_pairs Color(
			const int kind,
			const int sub) const; //пара цветов текст_фон в зависимоти от типа (kind) и вещества (sub) блока.
	color_pairs Color(
			const ushort,
			const ushort,
			const ushort) const;

	private slots:
	void Print();

	public slots:
	void Notify(const QString &);
	void CleanAll();
	void PassString(QString &) const;
	void Update(
			const ushort,
			const ushort,
			const ushort)
	{
		updated=false;
	}
	void UpdateAll() { updated=false; };
	void UpdatePlayer() { updated=false; };
	void UpdateAround(
			const ushort,
			const ushort,
			const ushort,
			const ushort)
	{
		updated=false;
	}
	void Move(const int) { updated=false; }
	void RePrint() {
		clear();
		updated=false;
	}

	signals:
	void ExitReceived();
	void InputReceived(int, int) const;

	public:
	void ControlPlayer(const int);
	Screen(World * const, Player * const);
};

/** \class IThread screen.h
 * \brief Keyboard input thread for curses screen for freg.
 *
 * This class is thread, with IThread::run containing input loop.
 */

#include <QThread>

class IThread : public QThread {
	Q_OBJECT

	Screen * const screen;

	public:
		IThread(Screen * const);
		void Stop();

	protected:
		void run();
	
	private:
		volatile bool stopped;
};

#endif
