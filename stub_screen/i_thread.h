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

//this file provides declaration of inputh thread for text (stub) screen for freg.

#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include "header.h"

class IThread : public QThread { //thread for keyboard input
	Q_OBJECT
	
	public:
		IThread();
		void Stop();

	protected:
		void run();
	
	signals:
		void InputReceived(int, int);
		void ExitReceived();
		void RePrintReceived() const;

	private:
		volatile bool stopped;
};

#endif
