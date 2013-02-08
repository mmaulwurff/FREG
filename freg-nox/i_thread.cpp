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

#include "i_thread.h"
#include "curses.h"

IThread::IThread() : stopped(false) {}

void IThread::run() {
	while ( !stopped ) {
		static int c;
		switch ( c=getch() ) {
			case KEY_UP:
			case 'u': emit InputReceived(MOVE, NORTH); break;
			case KEY_DOWN:
			case 'D': emit InputReceived(MOVE, SOUTH); break;
			case KEY_RIGHT:
			case 'r': emit InputReceived(MOVE, EAST ); break;
			case KEY_LEFT:
			case 'l': emit InputReceived(MOVE, WEST ); break;
			case '>': emit InputReceived(TURN_RIGHT, HERE); break;
			case '<': emit InputReceived(TURN_LEFT, HERE); break;
			case ' ': emit InputReceived(JUMP, HERE); break;
			case KEY_NPAGE:
			case 'v': emit InputReceived(TURN, DOWN); break;
			case KEY_PPAGE:
			case 'p': emit InputReceived(TURN, UP);   break;
			case 'i': emit InputReceived(OPEN_INVENTORY, HERE); break;
			case '\n':
			case 'n': emit InputReceived(USE, HERE); break;
			case '?': emit InputReceived(EXAMINE, HERE); break;
			case 'd': emit InputReceived(DROP, HERE); break;
			case 'g': emit InputReceived(GET, HERE); break;
			case 'W': emit InputReceived(WIELD, HERE); break;
			case 'E': emit InputReceived(EAT, HERE); break;
			case 'I': emit InputReceived(INSCRIBE, HERE); break;
			case KEY_BACKSPACE:
			case 'B': emit InputReceived(DAMAGE, HERE); break;
			case KEY_HOME:
			case 'H': emit InputReceived(BUILD, HERE); break;
			case 'L': emit RePrintReceived(); break;
			case 'Q': emit ExitReceived(); break;
			default: break;
		}
		
		//fprintf(stderr, "Input received: '%c' (code %d)\n", (char)c, c);

		msleep(90);
		flushinp();
	}
}

void IThread::Stop() { stopped=true; }
