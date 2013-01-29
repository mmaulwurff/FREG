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

//this file provides stub screen for freg (only text, only notifications)

#ifndef SCREEN_H
#define SCREEN_H

#define NOX
#define SCREEN_SIZE 30

#include "VirtScreen.h"
#include "i_thread.h"
#include <cstdio>

class Screen : public VirtScreen {
	Q_OBJECT

	IThread * input;
	bool cleaned;

	FILE * notifyLog; //весь текст уведомлений (notification) дублируется в файл.

	private slots:
	void Print() {}

	public slots:
	void Notify(const QString & str) {
		puts(str.toLocal8Bit().constData());
	}
	void CleanAll() {
		Notify("Exiting...");
	}
	void PassString(QString & str) const {
		char input[144];
		fgets(input, 144, stdin);
		str=input;
	}
	void Update(
			const ushort,
			const ushort,
			const ushort) {}
	void UpdateAll() {
		Notify("World updated.");
	}
	void UpdatePlayer() {}
	void UpdateAround(
			const ushort,
			const ushort,
			const ushort,
			const ushort) {}
	void RePrint() { //стереть всё с экрана и перерисовать всё с нуля (можно сделать пустой)
		Notify("Reprint: stub");
	}

	signals:
	void ExitReceived();
	void InputReceived(const int, const int) const;

	public:
	Screen(World * const, Player * const);
};

#endif
