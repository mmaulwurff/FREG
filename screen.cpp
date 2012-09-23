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

#ifndef SCREEN_FUNC_H
#define SCREEN_FUNC_H

#include "screen.h"
#include "world.h"

char Screen::CharName(const kinds kind, const subs sub) const {
	switch (kind)  {
		case BLOCK: switch (sub) {
			case NULLSTONE: case MOSS_STONE: case WOOD:
			case STONE: return '#';
			case GLASS: return 'g';
			case SUN_MOON:
			case SKY:   return ' ';
			case STAR:  return '.';
			case WATER: return '~';
			case SOIL:  return '.';
			case GREENERY: return '%';
			case ROSE:  return ';';
			case A_MEAT: case H_MEAT:
			case HAZELNUT: return ',';
			case AIR:   return ' ';
			default:
				fprintf(stderr, "Screen::CharName(uns short, uns short, uns short): Block has unlisted substance: %d\n", int(sub));
				return '?';
		}
		case DWARF: return '@';
		case CHEST: return 'c';
		case BUSH: return ';';
		case PILE: return '*';
		case PICK: return '\\';
		case TELEGRAPH: return 't';
		case LIQUID: return '~';
		case GRASS: return '.';
		case RABBIT: return 'r';
		default:
			fprintf(stderr, "Screen::CharName(uns short, uns short, uns short): Unlisted kind: %d\n", int(kind));
			return '?';
	}
}
char Screen::CharName(const unsigned short i, const unsigned short j, const unsigned short k) const {
	return CharName( w->Kind(i, j, k), w->Sub(i, j, k) );
}

color_pairs Screen::Color(const kinds kind, const subs sub) const {
	switch (kind) { //foreground_background
		case DWARF:     return WHITE_BLUE;
		case PILE:      return WHITE_BLACK;
		case TELEGRAPH: return CYAN_BLACK;
		case RABBIT:    return RED_WHITE;
		case LIQUID: switch (sub) {
			case WATER: return CYAN_BLUE;
			default:    return RED_YELLOW;
		}

		default: switch (sub) {
			case A_MEAT:     return WHITE_RED;
			case H_MEAT:     return BLACK_RED;
			case WOOD: case HAZELNUT:
			case SOIL:       return BLACK_YELLOW;
			case GREENERY:   return BLACK_GREEN;
			case WATER:      return WHITE_CYAN;
			case GLASS:      return BLUE_WHITE;
			case NULLSTONE:  return WHITE_BLACK;
			case MOSS_STONE: return GREEN_WHITE;
			case IRON:       return WHITE_BLACK;
			case ROSE:       return RED_GREEN;
			case SUN_MOON:   return ( NIGHT==w->PartOfDay() ) ? WHITE_WHITE : YELLOW_YELLOW;
			case SKY: case STAR: switch ( w->PartOfDay() ) {
				case NIGHT:   return WHITE_BLACK;
				case MORNING: return WHITE_BLUE;
				case NOON:    return CYAN_CYAN;
				case EVENING: return WHITE_BLUE;
			}
			default: return BLACK_WHITE;
		}
	}
}
inline color_pairs Screen::Color(const unsigned short i, const unsigned short j, const unsigned short k) const {
	return Color( w->Kind(i, j, k), w->Sub(i, j, k) );
}

void Screen::Print() const {
	switch (viewLeft) {
		case INVENTORY: if (NULL!=blockToPrintLeft) {
			PrintInv( leftWin, (Inventory *)blockToPrintLeft->HasInventory() );
			break;
		}
		case NORMAL: PrintNormal(leftWin); break;
		case FRONT: PrintFront(leftWin); break;
	}	
	switch (viewRight) {
		case INVENTORY: if (NULL!=blockToPrintRight) {
			PrintInv( rightWin, (Inventory *)blockToPrintRight->HasInventory() );
			break;
		}
		case NORMAL: PrintNormal(rightWin); break;
		case FRONT: PrintFront(rightWin); break;
	}

	Dwarf * playerP=w->GetPlayerP();
	if (NULL==playerP) return;
	werase(hudWin);
	short dur=playerP->Durability();
	wstandend(hudWin);
	mvwprintw(hudWin, 0, 1, "HP: %hd%%", dur);
	wcolor_set(hudWin, WHITE_RED, NULL);
	wmove(hudWin, 0, 10);
	for (unsigned short i=0; i*dur/10<max_durability; ++i)
		waddch(hudWin, '.');
	wrefresh(hudWin);
}

void Screen::PrintNormal(WINDOW * const window) const {
	if (pthread_mutex_trylock(&(w->mutex)))
		return;
	unsigned short k_start;
	short k_step, k_end;
	unsigned short playerZ;
	w->GetPlayerZ(playerZ);
	if ( UP==w->GetPlayerDir() ) {
		k_start=playerZ+1;
		k_step=1;
		k_end=height;
	} else {
		k_start=( DOWN==w->GetPlayerDir() ) ? playerZ-1 : playerZ;
		k_step=-1;
		k_end=-1;
	}
	wmove(window, 1, 1);
	for ( short j=0; j<shred_width*3; ++j, waddstr(window, "\n_") )
	for ( short i=0; i<shred_width*3; ++i )
		for (short k=k_start; k!=k_end; k+=k_step) //bottom is made from undestructable stone, loop will find what to print everytime
			if (w->Transparent(i, j, k) < 2) {
				if ( w->Visible(i, j, k) && w->Enlightened(i, j, k) ) {
					wcolor_set(window, Color(i, j, k), NULL);
					wprintw( window, "%c%c", CharName(i, j, k), w->CharNumber(i, j, k) );
				} else {
					wcolor_set(window, BLACK_BLACK, NULL);
					wprintw(window, "  ");
				}
				break;
			}
	wstandend(window);
	box(window, 0, 0);
	if ( UP==w->GetPlayerDir() || DOWN==w->GetPlayerDir() ) {
		mvwaddstr(window, 0, 1, ( UP==w->GetPlayerDir() ) ? "Sky View" : "Ground View");
		unsigned short arrow_X, arrow_Y;
		w->GetPlayerCoords(arrow_X, arrow_Y);
		Arrows(window , arrow_X*2+1, arrow_Y+1);
	} else
		mvwaddstr(window, 0, 1, "Normal View");
	wrefresh(window);
	pthread_mutex_unlock(&(w->mutex));
}

void Screen::PrintFront(WINDOW * const window) const {
	if ( UP==w->GetPlayerDir() || DOWN==w->GetPlayerDir() ) {
		wstandend(window);
		werase(window);
		box(window, 0, 0);
		mvwaddstr(window, 0, 1, "No view");
		wrefresh(window);
		return;
	} else if (pthread_mutex_trylock(&(w->mutex)))
		return;
	short x_step, z_step,
	      x_end, z_end,
	      * x, * z,
	      i, j, k;
	unsigned short pX, pY, pZ;
	unsigned short x_start, z_start,
		       k_start,
	               arrow_Y, arrow_X;
	w->GetPlayerCoords(pX, pY, pZ);
	switch ( w->GetPlayerDir() ) {
		case NORTH:
			x=&i;
			x_step=1;
			x_start=0;
			x_end=shred_width*3;
			z=&j;
			z_step=-1;
			z_start=pY-1;
			z_end=-1;
			arrow_X=pX*2+1;
		break;
		case SOUTH:
			x=&i;
			x_step=-1;
			x_start=shred_width*3-1;
			x_end=-1;
			z=&j;
			z_step=1;
			z_start=pY+1;
			z_end=shred_width*3;
			arrow_X=(shred_width*3-pX)*2-1;
		break;
		case WEST:
			x=&j;
			x_step=-1;
			x_start=shred_width*3-1;
			x_end=-1;
			z=&i;
			z_step=-1;
			z_start=pX-1;
			z_end=-1;
			arrow_X=(shred_width*3-pY)*2-1;
		break;
		case EAST:
			x=&j;
			x_step=1;
			x_start=0;
			x_end=shred_width*3;
			z=&i;
			z_step=1;
			z_start=pX+1;
			z_end=shred_width*3;
			arrow_X=pY*2+1;
		break;
		default:
			fprintf(stderr, "Screen::PrintFront(WINDOW *): unlisted dir: %d\n", (int)w->GetPlayerDir());
			return;
	}
	if (pZ+shred_width*1.5>=height) {
		k_start=height-2;
		arrow_Y=height-pZ;
	} else if (pZ-shred_width*1.5<0) {
		k_start=shred_width*3-1;
		arrow_Y=shred_width*3-pZ;
	} else {
		k_start=pZ+shred_width*1.5;
		arrow_Y=shred_width*1.5+1;
	}
	wmove(window, 1, 1);
	for (k=k_start; k_start-k<shred_width*3; --k, waddstr(window, "\n_"))
		for (*x=x_start; *x!=x_end; *x+=x_step) {
			for (*z=z_start; *z!=z_end; *z+=z_step)
				if (w->Transparent(i, j, k) < 2) {
					if ( w->Visible(i, j, k) && w->Enlightened(i, j, k) ) {
						wcolor_set(window, Color(i, j, k), NULL);
						wprintw( window, "%c%c", CharName(i, j, k), w->CharNumberFront(i, j) );
					} else {
						wcolor_set(window, BLACK_BLACK, NULL);
						wprintw(window, "  ");
					}
					break;
				}
			if (*z==z_end) { //print background decorations
				*z-=z_step;
				if (w->Visible(i, j, k)) {
				       	wcolor_set(window, WHITE_BLUE, NULL);
					wprintw(window, " .");
				} else {
					wcolor_set(window, BLACK_BLACK, NULL);
					waddstr(window, "  ");
				}
			}
		}
	wstandend(window);
	box(window, 0, 0);
	mvwaddstr(window, 0, 1, "Front View");
	Arrows(window, arrow_X, arrow_Y);
	wrefresh(window);
	pthread_mutex_unlock(&(w->mutex));
}

void Screen::PrintInv(WINDOW * const window, Inventory * const inv) const {
	if (pthread_mutex_trylock(&(w->mutex)))
		return;
	werase(window);
	wstandend(window);
	mvwaddstr(window, 1, 53, "Weight");
	if ( DWARF==inv->Kind() ) {
		mvwaddstr(window, 2, 2, "On head:");
		mvwaddstr(window, 3, 2, "On body:");
		mvwaddstr(window, 4, 2, "On feet:");
		mvwaddstr(window, 5, 2, "Right hand:");
		mvwaddstr(window, 6, 2, "Left hand:");
	}
	unsigned short i;
	double sum_weight=0, temp_weight;
	char str[full_name_length],
	     num_str[6];
	for (i=0; i<inventory_size; ++i) {
		mvwprintw(window, 2+i, 14, "%c)", 'a'+i);
		if ( inv->Number(i) ) {
			wcolor_set(window, Color(inv->GetInvKind(i), inv->GetInvSub(i)), NULL);
			mvwprintw( window, 2+i, 17, "[%c]%s", CharName( inv->GetInvKind(i), inv->GetInvSub(i) ), inv->InvFullName(str, i) );
			waddstr(window, inv->NumStr(num_str, i));
			wstandend(window);
			if ('\0'!=str[0]) { //summ
				mvwprintw(window, 2+i, 50, "%2.1f kg", temp_weight=inv->GetInvWeight(i));
				sum_weight+=temp_weight;
			}
		}
	}
	mvwprintw(window, 2+i, 43, "Sum:%6.1f kg", sum_weight);
	if (w->GetPlayerP()==inv) {
		box(window, 0, 0);
		mvwaddstr(window, 0, 1, "Your inventory");
	} else {
		wcolor_set(window, Color(inv->Kind(), inv->Sub()), NULL);
		box(window, 0, 0);
		char str[full_name_length];
		mvwprintw(window, 0, 1, "[%c]%s", CharName(inv->Kind(), inv->Sub()), inv->FullName(str));
	}
	wrefresh(window);
	pthread_mutex_unlock(&(w->mutex));
}

void Screen::PrintSounds() const {
	if (pthread_mutex_trylock(&(w->mutex)))
		return;

	for (unsigned short i=0; i<3; ++i)
	for (unsigned short j=0; j<3; ++j) {
		wcolor_set(soundWin, w->soundMap[i*3+j].col, NULL);
		mvwprintw(soundWin, i+1, j*2+1, "%c%hd", w->soundMap[i*3+j].ch, w->soundMap[i*3+j].lev);
	}
	wstandend(soundWin);
	box(soundWin, 0, 0);
	mvwaddstr(soundWin, 0, 1, "Sounds");
	wrefresh(soundWin);
	pthread_mutex_unlock(&(w->mutex));
}

void Screen::Notify(const char * const str, color_pairs color) const {
	werase(notifyWin);
	wcolor_set(notifyWin, color, NULL);
	mvwaddstr(notifyWin, 0, 0, str);
	wrefresh(notifyWin);
	fprintf(notifyLog, "%s\n", str);
}

Screen::Screen(World * const wor) :
       		w(wor), blockToPrintLeft(NULL), blockToPrintRight(NULL), viewLeft(NORMAL), viewRight(FRONT) {
	set_escdelay(10);
	initscr();
	start_color();
	raw();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	//all available color pairs (maybe some of them will not be used)
	short colors[]={ //do not change colors order!
		COLOR_BLACK,
		COLOR_RED,
		COLOR_GREEN,
		COLOR_YELLOW,
		COLOR_BLUE,
		COLOR_MAGENTA,
		COLOR_CYAN,
		COLOR_WHITE
	};
	for (short i=BLACK_BLACK; i<=WHITE_WHITE; ++i)
		init_pair(i, colors[(i-1)/8], colors[(i-1)%8]);
	leftWin  =newwin(shred_width*3+2, shred_width*2*3+2, 0, 0);
	rightWin =newwin(shred_width*3+2, shred_width*2*3+2, 0, shred_width*2*3+2);
	hudWin=newwin(3+2, (shred_width*2*3+2)*2-8, shred_width*3+2, 8);
	notifyWin=newwin(0, COLS, shred_width*3+2+5, 0);
	soundWin =newwin(3+2, 3*2+2, shred_width*3+2, 0);
	w->scr=this;
	notifyLog=fopen("messages.txt", "a");
}

Screen::~Screen() {
	pthread_mutex_lock(&(w->mutex));
	delwin(leftWin);
	delwin(rightWin);
	delwin(notifyWin);
	delwin(hudWin);
	delwin(soundWin);
	endwin();
	w->scr=NULL;
	if (NULL!=notifyLog)
		fclose(notifyLog);
	pthread_mutex_unlock(&(w->mutex));
}

#endif
