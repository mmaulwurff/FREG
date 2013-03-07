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

//this file is related to curses screen for freg.

#include <QString>
#include <QTimer>
#include "screen.h"
#include "world.h"
#include "blocks.h"
#include "Player.h"

void Screen::PassString(QString & str) const {
	echo();
	werase(notifyWin);
	mvwaddstr(notifyWin, 0, 0, "Enter inscription:");
	curs_set(1);
	char temp_str[note_length+1];
	mvwgetnstr(notifyWin, 1, 0, temp_str, note_length);
	str=temp_str;
	werase(notifyWin);
	wnoutrefresh(notifyWin);
	noecho();
	curs_set(0);
}

char Screen::CharNumber(
		const ushort i,
		const ushort j,
		const ushort k) const
{
	if ( height-1==k )
		return ' ';

	if ( player->GetP()==w->GetBlock(i, j, k) )
		switch ( player->Dir() ) {
			case NORTH: return '^';
			case SOUTH: return 'v';
			case EAST:  return '>';
			case WEST:  return '<';
			case DOWN:  return 'x';
			case UP:    return '.';
			default:
				fprintf(stderr,
					"Screen::CharNumber(ushort, ushort, ushort): unlisted dir: %d\n",
					(int)player->Dir());
				return '*';
		}

	const ushort playerZ=player->Z();
	if ( UP==player->Dir() ) {
		if ( k > playerZ && k < playerZ+10 )
			return k-playerZ+'0';
	} else {
		if ( k==playerZ )
			return ' ';
		if ( k>playerZ-10 )
			return playerZ-k+'0';
	}
	return '+';
}

char Screen::CharNumberFront(
		const ushort i,
		const ushort j) const
{
	ushort ret;
	if ( NORTH==player->Dir() || SOUTH==player->Dir() ) {
		if ( (ret=abs(player->Y()-j))<10 )
			return ret+'0';
	} else
		if ( (ret=abs(player->X()-i))<10 )
			return ret+'0';
	return '+';
}

char Screen::CharName(
		const int kind,
	       	const int sub) const
{
	switch ( kind )  {
		case CHEST:
		case PILE:   return '&';
		case BUSH:   return ';';
		case DWARF:  return '@';
		case LIQUID: return '~';
		case GRASS:  return '.';
		case RABBIT: return 'r';
		case CLOCK:  return 'c';
		case PLATE:  return '-';
		case LADDER: return '^';
		case PICK:   return '\\';
		case DOOR:   return '\'';
		case WORKBENCH: return '*';
		case TELEGRAPH: return 't';
		case WEAPON: switch ( sub ) {
			case WOOD:  return '/';
			case STONE: return '.';
			case IRON:  return '/';
			default:
				fprintf(stderr,
					"Screen::CharName: unlisted sub: %d\n",
					sub);
				return '.';
		}
		default: switch ( sub ) {
			case NULLSTONE: case MOSS_STONE: case WOOD:
			case IRON:
			case STONE: return '#';
			case GLASS: return 'g';
			case SUN_MOON: case SKY:
			case AIR:   return ' ';
			case STAR:  return '.';
			case WATER: return '~';
			case SAND:
			case SOIL:  return '.';
			case GREENERY: return '%';
			case ROSE:  return ';';
			case A_MEAT: case H_MEAT:
			case HAZELNUT: return ',';
			default:
				fprintf(stderr,
					"Screen::CharName: unlisted substance: %d\n",
					sub);
				return '?';
		}
	}
}
char Screen::CharName(
		const ushort i,
		const ushort j,
		const ushort k) const
{
	return CharName( w->Kind(i, j, k),
		w->Sub(i, j, k) );
}

color_pairs Screen::Color(
		const int kind,
		const int sub) const
{
	switch (kind) { //foreground_background
		case DWARF:     return WHITE_BLUE;
		case PILE:      return WHITE_BLACK;
		case TELEGRAPH: return CYAN_BLACK;
		case RABBIT:    return RED_WHITE;
		case BUSH:      return BLACK_GREEN;
		case LIQUID: switch (sub) {
			case WATER: return CYAN_BLUE;
			default:    return RED_YELLOW;
		}
		default: switch (sub) {
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
inline color_pairs Screen::Color(
		const ushort i,
	       	const ushort j,
	       	const ushort k) const
{
	return Color( w->Kind(i, j, k),
			w->Sub(i, j, k) );
}

void Screen::ControlPlayer(const int ch) {
	if ( 'Q'==ch ) {
		emit ExitReceived();
		return;
	}
	if ( ch>='a' && ch<='z' ) {
		const int num=ch-'a';
		switch ( actionMode ) {
			case USE:   player->Use(num); break;
			case THROW: player->Throw(num); break;
			case OBTAIN: player->Obtain(num); break;
			case WIELD: player->Wield(num); break;
			case INSCRIBE: player->Inscribe(num); break;
			case EAT: player->Eat(num); break;
			case BUILD: player->Build(num); break;
			case CRAFT: player->Craft(num); break;
			case TAKEOFF: player->TakeOff(num); break;
			default:
				fprintf(stderr,
					"Screen::ControlPlayer: unlisted action mode: %d\n",
					actionMode);
		}
		return;
	}
	switch ( ch ) {
		case KEY_UP: player->Move(NORTH); break;
		case KEY_DOWN: player->Move(SOUTH); break;
		case KEY_RIGHT: player->Move(EAST); break;
		case KEY_LEFT: player->Move(WEST); break;
		case ' ': player->Jump(); break;

		case '>': player->Turn(w->TurnRight(player->Dir())); break;
		case '<': player->Turn(w->TurnLeft(player->Dir())); break;
		case KEY_NPAGE: player->Turn(DOWN); break;
		case KEY_PPAGE: player->Turn(UP); break;

		case KEY_HOME: player->Backpack(); break;
		case KEY_BACKSPACE: player->Damage(); break;
		case '\n': player->Use(); break;
		case  '?': player->Examine(); break;
		case  '~': player->Inscribe(); break;

		case 'U': actionMode=USE; break;
		case 'T': actionMode=THROW; break;
		case 'O': actionMode=OBTAIN; break;
		case 'W': actionMode=WIELD; break;
		case 'I': actionMode=INSCRIBE; break;
		case 'E': actionMode=EAT; break;
		case 'B': actionMode=BUILD; break;
		case 'C': actionMode=CRAFT; break;
		case 'F': actionMode=TAKEOFF; break;

		case '+': w->SetNumActiveShreds(w->NumActiveShreds()+2); break;
		case '-':
			if ( w->NumActiveShreds() > 1 )
				w->SetNumActiveShreds(w->NumActiveShreds()-2);
			else
				Notify(QString(
					"Active shreds number too small: %1x%2.").
						arg(w->NumActiveShreds()).
						arg(w->NumActiveShreds()));
		break;
		case '#':
			Notify("Don't make shreds number too big.");
			player->SetNumShreds(w->NumShreds()+2);
		break;
		case '_':
			player->SetNumShreds(w->NumShreds()-2);
		break;

		case 'L': RePrint(); break;
		default: Notify("Don't know what such key means."); break;
	}
	updated=false;
}

void Screen::Print() {
	w->ReadLock();

	if ( updated || !player || !player->GetP() ) {
		w->Unlock();
		return;
	}
	updated=true;

	switch ( player->UsingType() ) {
		case OPEN:
			if ( player ) {
				PrintInv(leftWin, player->UsingBlock()->HasInventory());
				break;
			}
		default: PrintNormal(leftWin);
	}
	switch ( player->UsingSelfType() ) {
		case OPEN:
			if ( player && player->GetP()->HasInventory() ) {
				PrintInv(rightWin, player->GetP()->HasInventory());
				break;
			} //no break;
		default:
			PrintFront(rightWin);
	}

	const short dur=player->HP();
	const short breath=player->Breath();
	const short satiation=player->Satiation();
	w->Unlock();

	ushort i;
	//HitPoints line
	werase(hudWin);
	wstandend(hudWin);
	wmove(hudWin, 0, 0);
	wprintw(hudWin, "HP: %3hd  [", dur);
	wcolor_set(hudWin, WHITE_RED, NULL);
	for (i=0; i<10*dur/max_durability; ++i)
		waddch(hudWin, '.');
	wstandend(hudWin);
	mvwaddstr(hudWin, 0, 20, "]\n");

	//breath line
	if ( -1!=breath ) {
		wprintw(hudWin, "BR: %3hd%% [", 100*breath/max_breath);
		wcolor_set(hudWin, WHITE_BLUE, NULL);
		for (i=0; i<10*breath/max_breath; ++i)
			waddch(hudWin, '.');
		wstandend(hudWin);
		mvwaddstr(hudWin, 1, 20, "]\n");
	}

	//action mode
	wstandend(hudWin);
	waddstr(hudWin, "Action: ");
	switch ( actionMode ) {
		case USE:      waddstr(hudWin, "Use in inventory"); break;
		case THROW:    waddstr(hudWin, "Throw"); break;
		case OBTAIN:   waddstr(hudWin, "Obtain"); break;
		case WIELD:    waddstr(hudWin, "Wield"); break;
		case INSCRIBE: waddstr(hudWin, "Inscribe in inventory"); break;
		case EAT:      waddstr(hudWin, "Eat"); break;
		case BUILD:    waddstr(hudWin, "Build"); break;
		case CRAFT:    waddstr(hudWin, "Craft"); break;
		case TAKEOFF:  waddstr(hudWin, "Take off"); break;
		default:       waddstr(hudWin, "Unknown");
			fprintf(stderr,
				"Screen::Print: Unlisted actionMode: %d\n",
				actionMode);
	}
	waddch(hudWin, '\n');

	//satiation line
	//wprintw(hudWin, "Sat: %u ", satiation);
	if ( -1!=satiation ) {
		if ( seconds_in_day<satiation ) {
			wcolor_set(hudWin, BLUE_BLACK, NULL);
			waddstr(hudWin, "Gorged\n");
		} else if ( 3*seconds_in_day/4<satiation ) {
			wcolor_set(hudWin, GREEN_BLACK, NULL);
			waddstr(hudWin, "Full\n");
		} else if (seconds_in_day/4>satiation) {
			wcolor_set(hudWin, RED_BLACK, NULL);
			waddstr(hudWin, "Hungry\n");
		}
	}

	//quick inventory
	Inventory * const inv=player->GetP()->HasInventory();
	if ( inv )
		for (i=0; i<inv->Size(); ++i) {
			wstandend(hudWin);
			const int x=36+i*2;
			mvwaddch(hudWin, 0, x, 'a'+i);
			if ( inv->Number(i) ) {
				wcolor_set(hudWin,
					Color( inv->GetInvKind(i),
						inv->GetInvSub(i) ), NULL);
				mvwaddch(hudWin, 1, x,
					CharName( inv->GetInvKind(i),
						inv->GetInvSub(i) ));
				mvwprintw(hudWin, 2, x, "%hu", inv->Number(i));
			}
		}
	wnoutrefresh(hudWin);
	doupdate();

	//fprintf(stderr, "player x: %hu, y: %hu", player->X(), player->Y());
}

void Screen::PrintNormal(WINDOW * const window) const {
	const int dir=player->Dir();
	const ushort k_start=( UP!=dir ) ?
		(( DOWN==dir ) ? player->Z()-1 : player->Z()) :
		player->Z()+1;
	const short k_step=( UP!=dir ) ? (-1) : 1;

	wmove(window, 1, 1);
	const ushort start_x=(player->X()/shred_width)*shred_width+(shred_width-SCREEN_SIZE)/2;
	const ushort start_y=(player->Y()/shred_width)*shred_width+(shred_width-SCREEN_SIZE)/2;
	const int block_side=( dir==UP ) ? DOWN : UP;
	for ( ushort j=start_x; j<SCREEN_SIZE+start_x; ++j, waddstr(window, "\n_") )
	for ( ushort i=start_y; i<SCREEN_SIZE+start_y; ++i ) {
		ushort k;
		for (k=k_start; INVISIBLE == w->Transparent(i, j, k); k+=k_step);
		if ( w->Enlightened(i, j, k, block_side) && player->Visible(i, j, k) ) {
			wcolor_set(window, Color(i, j, k), NULL);
			waddch(window, CharName(i, j, k));
			waddch(window, CharNumber(i, j, k));
		} else {
			wcolor_set(window, BLACK_BLACK, NULL);
			waddstr(window, "  ");
		}
	}

	wstandend(window);
	box(window, 0, 0);
	if ( UP==dir || DOWN==dir ) {
		const ushort start=(shred_width*w->NumShreds()-SCREEN_SIZE)/2;
		mvwaddstr(window, 0, 1, ( UP==dir ) ? "Sky View" : "Ground View");
		Arrows(window , (player->X()-start)*2+1, player->Y()-start+1);
	} else
		mvwaddstr(window, 0, 1, "Normal View");
	wnoutrefresh(window);
}

void Screen::PrintFront(WINDOW * const window) const {
	const int dir=player->Dir();
	if ( UP==dir || DOWN==dir ) {
		wstandend(window);
		werase(window);
		box(window, 0, 0);
		mvwaddstr(window, 0, 1, "No view");
		wnoutrefresh(window);
		return;
	}

	short x_step, z_step,
	      x_end, z_end,
	      * x, * z,
	      i, j, k;
	const ushort pX=player->X();
	const ushort pY=player->Y();
	const ushort pZ=player->Z();
	//const ushort start=(shred_width*w->NumShreds()-SCREEN_SIZE)/2;
	const ushort begin_x=(pX/shred_width)*shred_width+(shred_width-SCREEN_SIZE)/2;
	const ushort begin_y=(pY/shred_width)*shred_width+(shred_width-SCREEN_SIZE)/2;
	ushort x_start, z_start, k_start;
	ushort arrow_Y, arrow_X;
	switch ( dir ) {
		case NORTH:
			x=&i;
			x_step=1;
			x_start=begin_x;
			x_end=x_start+SCREEN_SIZE;
			z=&j;
			z_step=-1;
			z_start=pY-1;
			z_end=pY-shred_width-1;
			arrow_X=(pX-begin_x)*2+1;
		break;
		case SOUTH:
			x=&i;
			x_step=-1;
			x_start=SCREEN_SIZE-1+begin_x;
			x_end=begin_x-1;
			z=&j;
			z_step=1;
			z_start=pY+1;
			z_end=pY+shred_width+1;
			arrow_X=(SCREEN_SIZE-pX+begin_x)*2-1;
		break;
		case WEST:
			x=&j;
			x_step=-1;
			x_start=SCREEN_SIZE-1+begin_y;
			x_end=begin_y-1;
			z=&i;
			z_step=-1;
			z_start=pX-1;
			z_end=pX-shred_width-1;
			arrow_X=(SCREEN_SIZE-pY+begin_y)*2-1;
		break;
		case EAST:
			x=&j;
			x_step=1;
			x_start=begin_y;
			x_end=SCREEN_SIZE+begin_y;
			z=&i;
			z_step=1;
			z_start=pX+1;
			z_end=pX+shred_width+1;
			arrow_X=(pY-begin_y)*2+1;
		break;
		default:
			fprintf(stderr,
				"Screen::PrintFront(WINDOW *): unlisted dir: %d\n",
				(int)dir);
			return;
	}
	if ( pZ+SCREEN_SIZE/2>=height ) {
		k_start=height-2;
		arrow_Y=height-pZ;
	} else if ( pZ-SCREEN_SIZE/2<0 ) {
		k_start=SCREEN_SIZE-1;
		arrow_Y=SCREEN_SIZE-pZ;
	} else {
		k_start=pZ+SCREEN_SIZE/2;
		arrow_Y=SCREEN_SIZE/2+1;
	}

	const int block_side=w->Anti(dir);
	wmove(window, 1, 1);
	for (k=k_start; k_start-k<SCREEN_SIZE; --k, waddstr(window, "\n_"))
		for (*x=x_start; *x!=x_end; *x+=x_step) {
			for (*z=z_start; *z!=z_end; *z+=z_step)
				if ( w->Transparent(i, j, k) != INVISIBLE ) {
					if ( w->Enlightened(i, j, k, block_side) &&
							player->Visible(i, j, k) ) {
						wcolor_set(window, Color(i, j, k), NULL);
						waddch(window, CharName(i, j, k));
						waddch(window, CharNumberFront(i, j));
					} else {
						wcolor_set(window, BLACK_BLACK, NULL);
						waddstr(window, "  ");
					}
					break;
				}
			if ( *z==z_end ) { //far decorations
				*z-=z_step;
				wcolor_set(window,
					(player->Visible(i, j, k) ? WHITE_BLUE : BLACK_BLACK),
					NULL);
				waddstr(window, " .");
			}
		}

	wstandend(window);
	box(window, 0, 0);
	mvwaddstr(window, 0, 1, "Front View");
	Arrows(window, arrow_X, arrow_Y);
	wnoutrefresh(window);
}

void Screen::PrintInv(WINDOW * const window, Inventory * const inv) const {
	werase(window);
	wstandend(window);
	switch ( inv->Kind() ) {
		case DWARF:
			mvwaddstr(window, 2, 1, "      Head\n Right hand\n  Left hand\n       Body\n       Legs");
		break;
		case WORKBENCH: mvwaddstr(window, 2, 1, "   Product"); break;
		default: break;
	}
	mvwprintw(window, 2+inv->Size(), 40, "All weight: %6.1f kg", inv->TrueWeight());
	QString str;
	for (ushort i=0; i<inv->Size(); ++i) {
		mvwprintw(window, 2+i, 12, "%c)", 'a'+i);
		if ( inv->Number(i) ) {
			wcolor_set(window, Color(inv->GetInvKind(i), inv->GetInvSub(i)), NULL);
			wprintw(window, "[%c]%s",
					CharName( inv->GetInvKind(i),
						inv->GetInvSub(i) ),
					qPrintable(inv->InvFullName(str, i)) );
			if ( 1<inv->Number(i) )
				waddstr(window, qPrintable(inv->NumStr(str, i)));
			if ( ""!=inv->GetInvNote(str, i) )
				waddstr(window, qPrintable((" ~:"+
					(( str.size()<24 ) ? str : str.left(13)+"..."))));
			wstandend(window);
			mvwprintw(window, 2+i, 53, "%5.1f kg", 2, inv->GetInvWeight(i));
		}
	}
	wcolor_set(window, Color(inv->Kind(), inv->Sub()), NULL);
	box(window, 0, 0);
	wmove(window, 0, 1);
	if ( player->PlayerInventory()==inv )
		waddstr(window, "Your inventory");
	else
		wprintw(window, "[%c]%s",
			CharName(inv->Kind(),
				inv->Sub()),
			qPrintable(inv->FullName(str)));
	wnoutrefresh(window);
}

void Screen::Notify(const QString & str) {
	const short MAX_LINES=5;
	static QString lines[MAX_LINES]={"", "", "", "", ""};
	werase(notifyWin);
	for (ushort i=0; i<MAX_LINES-1; ++i) {
		lines[i]=lines[i+1];
		waddstr(notifyWin, qPrintable(lines[i]+'\n'));
	}
	waddstr(notifyWin, qPrintable(str));
	wnoutrefresh(notifyWin);
	updated=false;
	lines[MAX_LINES-1]=str;
	fputs(qPrintable(
			QString::number(w->Time())+
			": "+str+'\n'),
		notifyLog);
}

Screen::Screen(
		World * const wor,
		Player * const pl)
		:
		VirtScreen(wor, pl),
		updated(false),
		cleaned(false),
		actionMode(USE)
{
	set_escdelay(10); //задержка после нажатия esc. для обработки esc-последовательностей, пока не используется.
	//ifdefs are adjustments for windows console, added by Panzerschrek
	#ifdef Q_OS_WIN32
		AllocConsole();
		freopen( "conout$", "w", stdout );
		freopen( "conin$", "r", stdin );
	#endif

	initscr(); //инициировать экран

	#ifdef Q_OS_WIN32
		resize_term( (SCREEN_SIZE + 2) + (2 + 5) + (2 + 3), SCREEN_SIZE * 4 + 4 );
	#endif
	start_color();
	raw(); //коды нажатия клавиш поступают в программу без обработки (сырыми)
	noecho(); //не показывать то, что введено
	keypad(stdscr, TRUE); //использовать стрелки
	curs_set(0); //сделать курсор невидимым
	//all available color pairs (maybe some of them will not be used)
	const short colors[]={ //do not change colors order!
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
	leftWin =newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, 0);
	rightWin=newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, SCREEN_SIZE*2+2);
	hudWin=newwin(3+2, (SCREEN_SIZE*2+2)*2, SCREEN_SIZE+2, 0);
	notifyWin=newwin(0, COLS, SCREEN_SIZE+2+5, 0);

	notifyLog=fopen("messages.txt", "a");

	addstr("Press any key.");
	getch();
	Notify("Game started.");

	input=new IThread(this);
	input->start();

	timer=new QTimer(this);
	connect(timer, SIGNAL(timeout()),
		this, SLOT(Print()));
	timer->start(100);
}

void Screen::CleanAll() {
	//TODO: make own lock
	w->WriteLock();
	if ( cleaned ) {
		w->Unlock();
		return;
	}

	cleaned=true;//prevent double cleaning
	input->Stop();
	input->wait();
	delete input;
	w->Unlock();

	//удалить окна
	delwin(leftWin);
	delwin(rightWin);
	delwin(notifyWin);
	delwin(hudWin);
	//закончить оконный режим, очистить экран
	endwin();
	if ( NULL!=notifyLog )
		fclose(notifyLog);
}

IThread::IThread(Screen * const scr)
		:
	screen(scr),
	stopped(false)
{}

void IThread::run() {
	while ( !stopped ) {
		screen->ControlPlayer(getch());
		msleep(90);
		flushinp();
	}
}

void IThread::Stop() { stopped=true; }
