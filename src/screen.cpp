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

char Screen::CharName(const kinds kind, const subs sub) const { //вернуть символ, обозначающий блок
	switch (kind)  {
		//блоки, для символа которых тип важнее вещества
		case CHEST:
		case PILE:   return '&';
		case BUSH:   return ';';
		case PICK:   return '\\';
		case DWARF:  return '@';
		case LIQUID: return '~';
		case GRASS:  return '.';
		case RABBIT: return 'r';
		case TELEGRAPH: return 't';
		default: switch (sub) {
			//блоки, для символа которых вещество важнее типа
			case NULLSTONE: case MOSS_STONE: case WOOD:
			case STONE: return '#';
			case GLASS: return 'g';
			case SUN_MOON:
			case SKY:   return ' ';
			case STAR:  return '.';
			case WATER: return '~';
			case SAND:
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
	}
}
char Screen::CharName(const unsigned short i, const unsigned short j, const unsigned short k) const {
	return CharName( w->Kind(i, j, k), w->Sub(i, j, k) );
}

color_pairs Screen::Color(const kinds kind, const subs sub) const { //пара цветов текст_фон в зависимоти от типа (kind) и вещества (sub) блока.
	switch (kind) { //foreground_background
		//блоки, для цвета которых тип важнее вещества
		case DWARF:     return WHITE_BLUE;
		case PILE:      return WHITE_BLACK;
		case TELEGRAPH: return CYAN_BLACK;
		case RABBIT:    return RED_WHITE;
		case BUSH:      return BLACK_GREEN;
		case LIQUID: switch (sub) {
			case WATER: return CYAN_BLUE;
			default:    return RED_YELLOW; //всё расплавленное
		}
		default: switch (sub) {
			//блоки, для цвета которых вещество важнее типа
			case STONE:      return BLACK_WHITE;
			case SAND:       return YELLOW_WHITE;
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
			default: return WHITE_BLACK;
		}
	}
}
inline color_pairs Screen::Color(const unsigned short i, const unsigned short j, const unsigned short k) const {
	return Color( w->Kind(i, j, k), w->Sub(i, j, k) );
}

void Screen::Print() const {/*
	if ( w->mutex_trylock() ) //если другой процесс использует мир.
		return;
	//мьютекс (знак) заблокирован, другой процесс (нить) не сможет использовать мир до разблокировки.

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
	
	Dwarf * playerP=w->GetPlayerP(); //если игрока нет, и жизнь рисовать незачем.
	if (NULL==playerP)
		return;

	werase(hudWin); 

	//строка с жизнью
	wstandend(hudWin); //вернуть окну стандартный цвет
	short dur=playerP->Durability(); //прочность есть у всех блоков. для игрока прочность - жизнь
	wprintw(hudWin, " HP: %3hd%% <", dur);
	wcolor_set(hudWin, WHITE_RED, NULL);
	for (unsigned short i=0; i<10*dur/max_durability; ++i)
		waddch(hudWin, '.');
	wstandend(hudWin);
	mvwaddstr(hudWin, 0, 21, ">\n"); //передвинуть на y=0, x=21 и вывести строку

	//строка с дыханием
	short breath=playerP->Breath();
	wprintw(hudWin, " BR: %3hd%% <", breath);
	wcolor_set(hudWin, WHITE_BLUE, NULL);
	for (unsigned short i=0; i<10*breath/max_breath; ++i)
		waddch(hudWin, '.');
	wstandend(hudWin);
	mvwaddstr(hudWin, 1, 21, ">\n");
	
	//строка с сытостью/голодом
	short satiation=playerP->Satiation();
	 //полная насыщённость численно равна количеству секунд в дне * количество шагов в секунде
	if ( seconds_in_day*time_steps_in_sec<satiation ) { //если насыщенность больше - объелся и т.д.
		wcolor_set(hudWin, BLUE_BLACK, NULL);
		mvwaddstr(hudWin, 2, 1, "Gorged\n");
	} else if ( 3*seconds_in_day*time_steps_in_sec/4<satiation ) {
		wcolor_set(hudWin, GREEN_BLACK, NULL);
		mvwaddstr(hudWin, 2, 1, "Full\n");
	} else if (seconds_in_day*time_steps_in_sec/4>satiation) {
		wcolor_set(hudWin, RED_BLACK, NULL);
		mvwaddstr(hudWin, 2, 1, "Hungry\n");
	}

	wrefresh(hudWin); //обновить окно, в curses нужно, чтобы вывести символы из временного буфера непосредственно на экран
	w->mutex_unlock(); //мьютекс разблокирован
*/}

void Screen::PrintNormal() const {/*
	unsigned short k_start;
	short k_step;
	unsigned short playerZ;
	w->GetPlayerZ(playerZ); //получить z-координату игрока
	if ( UP==w->GetPlayerDir() ) { //подготовка: откуда начинать отрисовку и куда идти: в направлении роста или уменьшения z
		k_start=playerZ+1;
		k_step=1;
	} else {
		//если игрок смотрит в сторону, то рисовать всё с уровнем игрока, если смотрит в пол, то только пол и ниже
		k_start=( DOWN==w->GetPlayerDir() ) ? playerZ-1 : playerZ;
		k_step=-1;
	}
	wmove(window, 1, 1); //передвинуть курсор (невидимый) в 1,1. в клетке 0,0 начинается рамка.
	for ( short j=0; j<shred_width*3; ++j, waddstr(window, "\n_") )
	for ( short i=0; i<shred_width*3; ++i )
		for (short k=k_start; ; k+=k_step) //верх и низ лоскута (чанка) - непрозрачные нуль-камень и небесная твердь. за границы массива на выйдет.
			if (w->TransparentNotSafe(i, j, k) < 2) { //проверка прозрачности. NotSafe - т.к. гарантированно внутри массива
				if ( w->Enlightened(i, j, k) && w->Visible(i, j, k) ) { //освещено ли, не загорожено ли
					wcolor_set(window, Color(i, j, k), NULL); //цвет
					waddch(window, CharName(i, j, k)); //символ
					waddch(window, w->CharNumber(i, j, k)); //цифра (или +, если далеко)
				} else {
					wcolor_set(window, BLACK_BLACK, NULL);
					waddstr(window, "  ");
				}
				break;
			}
	wstandend(window); //вернуть окну стандартный цвет
	box(window, 0, 0); //рамка
	if ( UP==w->GetPlayerDir() || DOWN==w->GetPlayerDir() ) { //заголовок окна
		mvwaddstr(window, 0, 1, ( UP==w->GetPlayerDir() ) ? "Sky View" : "Ground View");
		unsigned short arrow_X, arrow_Y;
		w->GetPlayerCoords(arrow_X, arrow_Y);
		Arrows(window , arrow_X*2+1, arrow_Y+1);
	} else
		mvwaddstr(window, 0, 1, "Normal View");
	wrefresh(window); //вывод на экран
*/}

void Screen::PrintInv(Inventory * const inv) const {/*
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
*/}

void Screen::GetSound(const unsigned short n, const unsigned short dist, const char sound, const kinds kind, const subs sub) {
	if ( w->mutex_trylock() )
		return;

	soundMap[n].ch=(' '!=soundMap[n].ch) ? '&' : sound;

	short temp=shred_width-dist;
	if (temp<0)
		temp=0;
	soundMap[n].lev+=(temp*10)/shred_width;
	if (soundMap[n].lev>9)
		soundMap[n].lev=9;
	if (soundMap[n].lev>0) {
		soundMap[n].col=Color(kind, sub); 
		soundMap[n].lev+=1;
	}

	w->mutex_unlock();
}
void Screen::PrintSounds() {/*
	if ( w->mutex_trylock() )
		return;

	werase(soundWin);
	for (unsigned short i=0; i<3; ++i)
	for (unsigned short j=0; j<3; ++j)
		if ( ' '!=soundMap[i*3+j].ch ) {
			wcolor_set(soundWin, soundMap[i*3+j].col, NULL);
			mvwprintw(soundWin, i+1, j*2+1, "%c%hd", soundMap[i*3+j].ch, soundMap[i*3+j].lev);
			soundMap[i*3+j].ch=' ';
			soundMap[i*3+j].lev=0;
			soundMap[i*3+j].col=WHITE_BLACK;
		}
	wstandend(soundWin);
	box(soundWin, 0, 0);
	mvwaddstr(soundWin, 0, 1, "Sounds");
	wrefresh(soundWin);
	w->mutex_unlock();
*/}

void Screen::NotifyAdd(const char * const str, const kinds kind, const subs sub) {/*
	if (!str[0])
		return;
	wcolor_set(notifyWin, Color(kind, sub), NULL);
	mvwaddstr(notifyWin, notifyLines++, 0, str);
	wrefresh(notifyWin);
	fprintf(notifyLog, "%s\n", str);
*/}

void Screen::keyPressEvent(QKeyEvent *event) {
	switch (event->key()) {
		case 'U': /*case KEY_UP:*/    w->PlayerMove(NORTH); break;
		case 'D': /*case KEY_DOWN:*/  w->PlayerMove(SOUTH); break;
		case 'R': /*case KEY_RIGHT:*/ w->PlayerMove(EAST ); break;
		case 'L': /*case KEY_LEFT:*/  w->PlayerMove(WEST ); break;
		case '>': w->SetPlayerDir( w->TurnRight(w->GetPlayerDir()) ); break;
		case '<': w->SetPlayerDir( w->TurnLeft(w->GetPlayerDir()) );  break;
		case ' ': w->PlayerJump(); break;
		case 'w': w->SetPlayerDir(WEST);  break;
		case 'e': w->SetPlayerDir(EAST);  break;
		case 's': w->SetPlayerDir(SOUTH); break;
		case 'n': w->SetPlayerDir(NORTH); break;
		case 'P': /*case KEY_NPAGE:*/ w->SetPlayerDir(DOWN);  break; //page down key
		case 'p': /*case KEY_PPAGE:*/ w->SetPlayerDir(UP);    break; //page up key
		case 'i':
			if (INVENTORY!=viewRight) {
				viewRight=INVENTORY;
				blockToPrintRight=(Block *)(w->GetPlayerP());
			} else
				viewRight=FRONT;
		break;
		case 'N': case '\n': { //enter key
			unsigned short i, j, k;
			w->PlayerFocus(i, j, k);
			w->Use(i, j, k);
		} break;
		case '?': w->Examine(); break;
		//case 'd': w->PlayerDrop(Getch()-'a'); break;
		//case 'g': w->PlayerGet(Getch()-'a'); break;
		case 'W': w->PlayerWield(); break;
		//case 'E': w->PlayerEat(Getch()-'a'); break;
		case 'I': w->PlayerInscribe(); break;
		case 'B': /*case KEY_BACKSPACE:*/ {
			unsigned short i, j, k;
			w->PlayerFocus(i, j, k);
			w->Damage(i, j, k);
		} break;
		//case 'H': /*case KEY_HOME:*/ w->PlayerBuild(screen.Getch()-'a'); break;
		case 'l': RePrint(); break;
		case 'Q': break;
		default: Notify("What?\n");
	}

	//if ( print_flag )
		Print();
	//usleep(90000);
	Flushinp();
}

Screen::Screen(World * const wor) :
       		w(wor), notifyLines(0),
		blockToPrintLeft(NULL),
		blockToPrintRight(NULL),
		viewLeft(NORMAL),
		viewRight(FRONT) {
	/*set_escdelay(10); //задержка после нажатия esc. для обработки esc-последовательностей, пока не используется.
	initscr(); //инициировать экран
	start_color(); //цетной режим
	raw(); //коды нажатия клавиш поступают в программу без обработки (сырыми)
	noecho(); //не показывать то, что введено
	keypad(stdscr, TRUE); //использовать стрелки
	curs_set(0); //сделать курсор невидимым
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
	//ввести все цвета
	for (short i=BLACK_BLACK; i<=WHITE_WHITE; ++i)
		init_pair(i, colors[(i-1)/8], colors[(i-1)%8]);
	//задать положения и размеры окон
	leftWin  =newwin(shred_width*3+2, shred_width*2*3+2, 0, 0);
	rightWin =newwin(shred_width*3+2, shred_width*2*3+2, 0, shred_width*2*3+2);
	hudWin=newwin(3+2, (shred_width*2*3+2)*2-8, shred_width*3+2, 8); //окно для жизни, дыхания и т.д.
	notifyWin=newwin(0, COLS, shred_width*3+2+5, 0);
	soundWin =newwin(3+2, 3*2+2, shred_width*3+2, 0);
	*/
	for (unsigned short i=0; i<9; ++i) {
		soundMap[i].ch=' ';
		soundMap[i].lev=0;
		soundMap[i].col=WHITE_BLACK;
	}

	w->scr=this; //привязать экран к миру
	notifyLog=fopen("messages.txt", "a");

	setWindowTitle(tr("freg"));
	resize(550, 370);
}

Screen::~Screen() {
	w->mutex_lock();
	//удалить окна
	/*delwin(leftWin);
	delwin(rightWin);
	delwin(notifyWin);
	delwin(hudWin);
	delwin(soundWin);
	//закончить оконный режим, очистить экран
	endwin();*/
	w->scr=NULL; //отвязаться от мира
	if (NULL!=notifyLog)
		fclose(notifyLog);
	w->mutex_unlock();
}

#endif
