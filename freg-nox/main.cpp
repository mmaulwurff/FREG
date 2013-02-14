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

#include "screen.h" //NOX, if needed, is defined in screen.h

#ifdef NOX //no need for X server
	#include <QCoreApplication>
#else
	#include <QApplication>
#endif

#include <QString>
#include <QFile>
#include <QTextStream>
#include "header.h"
#include "world.h"
#include "Player.h"

int main(int argc, char *argv[]) {
	#ifdef NOX
		QCoreApplication freg(argc, argv);
	#else
		QApplication freg(argc, argv);
	#endif
	QString worldName, temp;
	ushort size;
	ushort sizeActive;
	QFile file("options.txt");
	if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		worldName="The_Land_Of_Doubts";
		size=3;
		sizeActive=3;
	} else {
		QTextStream in(&file);
		in >> temp >> worldName
			>> temp >> size
			>> temp >> sizeActive;
	}

	World earth(worldName, size, sizeActive);
	Player player(&earth);
	Screen screen(&earth, &player);
	earth.start();

	QObject::connect(&freg, SIGNAL(aboutToQuit()),
		&screen, SLOT(CleanAll()));
	QObject::connect(&freg, SIGNAL(aboutToQuit()),
		&player, SLOT(CleanAll()));
	QObject::connect(&freg, SIGNAL(aboutToQuit()),
		&earth, SLOT(CleanAll()));

	QObject::connect(&screen, SIGNAL(ExitReceived()),
		&freg, SLOT(quit()));

	return freg.exec();
}
