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

#include "header.h"
#include "screen.h"
#include "world.h"

int main() {
	World earth;
	Screen screen(&earth);
	FILE * scenario=fopen("scenario.txt", "r");
	int c='.';
	int print_flag=1; //print only if needed, needed nearly everytime
	while ( 'Q'!=c ) {
		if ( NULL!=scenario ) {
			c=fgetc(scenario);
			if ( EOF==c || '\n'==c ) {
				fclose(scenario);
				scenario=NULL;
				c=screen.Getch();
			} else
				fprintf(stderr, "Scenario used, key is: '%c'.\n", c);
		} else
			c=screen.Getch();
		switch (c) {
			case 'U': case KEY_UP:    earth.PlayerMove(NORTH); break;
			case 'D': case KEY_DOWN:  earth.PlayerMove(SOUTH); break;
			case 'R': case KEY_RIGHT: earth.PlayerMove(EAST ); break;
			case 'L': case KEY_LEFT:  earth.PlayerMove(WEST ); break;
			case '>': earth.SetPlayerDir( earth.TurnRight(earth.GetPlayerDir()) ); break;
			case '<': earth.SetPlayerDir( earth.TurnLeft(earth.GetPlayerDir()) );  break;
			case ' ': earth.PlayerJump(); break;
			case 'w': earth.SetPlayerDir(WEST);  break;
			case 'e': earth.SetPlayerDir(EAST);  break;
			case 's': earth.SetPlayerDir(SOUTH); break;
			case 'n': earth.SetPlayerDir(NORTH); break;
			case 'P': case KEY_NPAGE: earth.SetPlayerDir(DOWN);  break; //page down key
			case 'p': case KEY_PPAGE: earth.SetPlayerDir(UP);    break; //page up key
			case 'i':
				if (INVENTORY!=screen.viewRight) {
					screen.viewRight=INVENTORY;
					screen.blockToPrintRight=(Block *)(earth.GetPlayerP());
				} else
					screen.viewRight=FRONT;
			break;
			case 'N': case '\n': { //enter key
				unsigned short i, j, k;
				earth.PlayerFocus(i, j, k);
				earth.Use(i, j, k);
			} break;
			case '?': earth.Examine(); break;
			case 'd': earth.PlayerDrop(screen.Getch()-'a'); break;
			case 'g': earth.PlayerGet(screen.Getch()-'a'); break;
			case 'W': earth.PlayerWield(); break;
			case 'E': earth.PlayerEat(screen.Getch()-'a'); break;
			case 'I': earth.PlayerInscribe(); break;
			case 'B': case KEY_BACKSPACE: {
				unsigned short i, j, k;
				earth.PlayerFocus(i, j, k);
				earth.Damage(i, j, k);
			} break;
			case 'H': case KEY_HOME: earth.PlayerBuild(screen.Getch()-'a'); break;
			case 'l': screen.RePrint(); break;
			case 'Q': break;
			default: screen.Notify("What?\n");
		}

		if ( print_flag )
			screen.Print();
		usleep(90000);
		screen.Flushinp();
	}
}
