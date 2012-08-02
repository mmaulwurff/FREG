#ifndef SCREEN_FUNC_H
#define SCREEN_FUNC_H

char Screen::CharName(unsigned short i, unsigned short j, unsigned short k) {
	switch ( w->Kind(i, j, k) ) {
		case BLOCK: switch (w->Sub(i, j, k)) {
			case NULLSTONE:
			case STONE: return '#';
			case SOIL:  return 's';
			case GLASS: return 'g';
			case SUN_MOON:
			case SKY:   return ' ';
			case STAR:  return '.';
			default: return '?';
		}
		case DWARF: return '@';
		default: return '?';
	}
}

void Screen::Print() {
	if (pthread_mutex_trylock(&(w->mutex)))
		return;
	//left window
	unsigned short k_start;
	short k_step, k_end;
	if ( UP==w->GetPlayerDir() ) {
		k_start=w->playerZ+1;
		k_step=1;
		k_end=height;
	} else {
		k_start=w->playerZ;
		k_step=-1;
		k_end=-1;
	}
	wmove(leftWin, 1, 1);
	short i, j, k;
	for ( j=0; j<shred_width*3; ++j, waddstr(leftWin, "\n_") )
	for ( i=0; i<shred_width*3; ++i )
		for (k=k_start; k!=k_end; k+=k_step) //bottom is made from undestructable stone, loop will find what to print everytime
			if (w->Transparent(i, j, k) < 2) {
				if ( w->Visible(i, j, k) ) {
					wcolor_set(leftWin, Color(i, j, k), NULL);
					wprintw( leftWin, "%c%c", CharName(i, j, k), w->CharNumber(i, j, k) );
				} else {
					wcolor_set(leftWin, BLACK_BLACK, NULL);
					wprintw(leftWin, "  ");
				}
				break;
			}
	wstandend(leftWin);
	box(leftWin, 0, 0);
	mvwaddstr(leftWin, 0, 1, ( UP==w->GetPlayerDir() ) ? "Sky View" : "Normal View");
	//mvwprintw( leftWin, shred_width*3+1, 1, "%d", int(w->time));//w->TimeOfDay() );
	wrefresh(leftWin);
	//right window
	if (INVENTORY==view) {
		pthread_mutex_unlock(&(w->mutex));
		PrintInv();
		return;
	} else if ( UP==w->GetPlayerDir() || DOWN==w->GetPlayerDir() ) {
		pthread_mutex_unlock(&(w->mutex));
		return;
	}
	short pX, pY, pZ,
	      x_step, z_step,
	      x_end, z_end,
	      * x, * z;
	unsigned short x_start, z_start,
	               arrow_Y, arrow_X;
	//	       pX, pY, pZ;
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
	wmove(rightWin, 1, 1);
	for (k=k_start; k_start-k<shred_width*3; --k, waddstr(rightWin, "\n_"))
		for (*x=x_start; *x!=x_end; *x+=x_step) {
			for (*z=z_start; *z!=z_end; *z+=z_step)
				if (w->Transparent(i, j, k) < 2) {
					if ( w->Visible(i, j, k) ) {
						wcolor_set(rightWin, Color(i, j, k), NULL);
						wprintw( rightWin, "%c%c", CharName(i, j, k), w->CharNumberFront(i, j) );
					} else {
						wcolor_set(rightWin, BLACK_BLACK, NULL);
						wprintw(rightWin, "  ");
					}
					break;
				}
			if (*z==z_end) { //print background decorations
				*z-=z_step;
				if (w->Visible(i, j, k)) {
				       	wcolor_set(rightWin, WHITE_BLUE, NULL);
					wprintw(rightWin, " .");
				} else {
					wcolor_set(rightWin, BLACK_BLACK, NULL);
					waddstr(rightWin, "  ");
				}
			}
		}
	wstandend(rightWin);
	box(rightWin, 0, 0);
	mvwaddstr(rightWin, 0, 1, "Front View");
	mvwprintw(rightWin, 0,               arrow_X,           "vv");
	mvwprintw(rightWin, shred_width*3+1, arrow_X,           "^^");
	mvwprintw(rightWin, arrow_Y,         0,                 ">");
	mvwprintw(rightWin, arrow_Y,         shred_width*3*2+1, "<");
	wrefresh(rightWin);
	pthread_mutex_unlock(&(w->mutex));
}

void Screen::PrintInv() {
	Dwarf * player=w->GetPlayerP();
	unsigned short i;
	double sum_weight=0, temp_weight;
	char str[full_name_length],
	     num_str[6];
	mvwaddstr(rightWin, 1, 50, "Weight");
	mvwaddstr(rightWin, 2, 4, "On head:");
	mvwaddstr(rightWin, 3, 4, "On body:");
	mvwaddstr(rightWin, 4, 4, "On feet:");
	mvwaddstr(rightWin, 5, 4, "In right hand:");
	mvwaddstr(rightWin, 6, 4, "In left hand:");
	for (i=0; i<inventory_size; ++i) {
		player->InvFullName(str, i);
		player->NumStr(num_str, i);
		mvwprintw(rightWin, 2+i, 20, "%c) %s", 'a'+i, num_str);
		wcolor_set(rightWin, Color(player->GetInvKind(i), player->GetInvSub(i)), NULL);
		wprintw(rightWin, "%s", str);
		wstandend(rightWin);
		if ('\0'!=str[0]) {
			mvwprintw(rightWin, 2+i, 50, "%2.1f kg", temp_weight=player->GetInvWeight(i));
			sum_weight+=temp_weight;
		}
	}
	mvwprintw(rightWin, 2+i, 43, "Sum:%6.1f kg", sum_weight);
	box(rightWin, 0, 0);
	mvwprintw(rightWin, 0, 1, "Inventory");
	wrefresh(rightWin);
}

void Screen::PrintSounds() {
	unsigned short temp;
	for (unsigned short i=0; i<3; ++i)
	for (unsigned short j=0; j<3; ++j) {
		wcolor_set(soundWin, w->soundMap[i*3+j].col, NULL);
		mvwprintw(soundWin, i+1, j*2+1, "%c%hd", w->soundMap[i*3+j].ch, w->soundMap[i*3+j].lev);
	}
	wstandend(soundWin);
	box(soundWin, 0, 0);
	mvwaddstr(soundWin, 0, 1, "Sounds");
	wrefresh(soundWin);
}

void Screen::Notify(const char * str) {
	werase(notifyWin);
	mvwprintw(notifyWin, 1, 1, "%s", str);
	box(notifyWin, 0, 0);
	wrefresh(notifyWin);
}

color_pairs Screen::Color(kinds kind, subs sub) {
	switch (kind) {
		case DWARF: return WHITE_BLUE;
		case BLOCK: switch (sub) {
			case GLASS:     return BLUE_WHITE;
			case NULLSTONE: return WHITE_BLACK;
			case SUN_MOON:  return ( NIGHT==w->PartOfDay() ) ? WHITE_WHITE : YELLOW_YELLOW;
			case SKY: case STAR: switch ( w->PartOfDay() ) {
				case NIGHT:   return WHITE_BLACK;
				case MORNING: return WHITE_BLUE;
				case NOON:    return CYAN_CYAN;
				case EVENING: return WHITE_BLUE;
			}
			default: return BLACK_WHITE;
		}
		default: return BLACK_WHITE;
	}
}
inline color_pairs Screen::Color(unsigned short i, unsigned short j, unsigned short k) { return Color( w->Kind(i, j, k), w->Sub(i, j, k) ); }

inline void Screen::UpDownView(dirs dir) {
	w->SetPlayerDir(dir);
	werase(rightWin);
	wrefresh(rightWin);
}

Screen::Screen(World *wor) : w(wor), view(NONE) {
	set_escdelay(10);
	initscr();
	start_color();
	raw();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	//all available color pairs (maybe some of them will not be used)
	short i, colors[]={ //do not change colors order!
		COLOR_BLACK,
		COLOR_RED,
		COLOR_GREEN,
		COLOR_YELLOW,
		COLOR_BLUE,
		COLOR_MAGENTA,
		COLOR_CYAN,
		COLOR_WHITE
	};
	for (i=BLACK_BLACK; i<=WHITE_WHITE; ++i)
		init_pair(i, colors[(i-1)/8], colors[(i-1)%8]);
	leftWin  =newwin(shred_width*3+2, shred_width*2*3+2, 0, 0);
	rightWin =newwin(shred_width*3+2, shred_width*2*3+2, 0, shred_width*2*3+2);
	notifyWin=newwin(3+2, (shred_width*2*3+2)*2-8, shred_width*3+2, 8);
	soundWin =newwin(3+2, 3*2+2, shred_width*3+2, 0);
	w->scr=this;
}

Screen::~Screen() {
	delwin(leftWin);
	delwin(rightWin);
	delwin(notifyWin);
	delwin(soundWin);
	endwin();
	w->scr=NULL;
}

#endif
