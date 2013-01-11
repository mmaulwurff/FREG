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

#include <QString>
#include <QTimer>
#include "screen.h"
#include "world.h"
#include "blocks.h"
#include "i_thread.h"
#include "Player.h"

void Screen::PassString(QString & str) const {
	echo();
	werase(notifyWin);
	mvwaddstr(notifyWin, 0, 0, "Enter inscription:");
	char temp_str[note_length+1];
	mvwgetnstr(notifyWin, 1, 0, temp_str, note_length);
	str=temp_str;
	werase(notifyWin);
	wrefresh(notifyWin);
	noecho();
}

char Screen::CharNumber(
		const unsigned short & i,
		const unsigned short & j,
		const unsigned short & k) const
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
					"Screen::ChanNumber(int &, int &, int &): unlisted dir: %d\n",
					(int)player->Dir());
				return '*';
		}

	const unsigned short playerZ=player->Z();
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
		const unsigned short & i,
		const unsigned short & j) const
{
	unsigned short ret;
	if ( NORTH==player->Dir() || SOUTH==player->Dir() ) {
		if ( (ret=abs(player->Y()-j))<10 )
			return ret+'0';
	} else
		if ( (ret=abs(player->X()-i))<10 )
			return ret+'0';
	return '+';
}

char Screen::CharName(
		const int & kind,
	       	const int & sub) const
{ //вернуть символ, обозначающий блок
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
char Screen::CharName(
		const unsigned short & i,
		const unsigned short & j,
		const unsigned short & k) const
{
	return CharName( w->Kind(i, j, k),
		w->Sub(i, j, k) );
}

color_pairs Screen::Color(
		const int & kind,
		const int & sub) const
{ //пара цветов текст_фон в зависимоти от типа (kind) и вещества (sub) блока.
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
inline color_pairs Screen::Color(
		const unsigned short & i,
	       	const unsigned short & j,
	       	const unsigned short & k) const
{
	return Color( w->Kind(i, j, k),
			w->Sub(i, j, k) );
}

void Screen::Print() {
	w->ReadLock();
	//if ( !w->TryReadLock() )
	//	return;

	if ( updated ) {
		w->Unlock();
		return;
	}
	updated=true;

	//do not print player stats if there is no player
	//TODO PrintNormal and PrintFront with parameters (for printing w/o player)
	/*if ( !player ) {
		wrefresh(hudWin);
		PrintNormal(leftWin);
		PrintFront(rightWin);
		w->mutex_unlock();
		return;
	}*/

	if ( player->UsingBlock() )
		switch ( player->UsingType() ) {
			case OPEN:
				PrintInv(leftWin, player->UsingBlock()->HasInventory());
			break;
			default: PrintNormal(leftWin);
		}
	else
		PrintNormal(leftWin);

	switch ( player->UsingSelfType() ) {
		case OPEN:
			PrintInv(rightWin, player->GetP()->HasInventory());
			break;
		default: PrintFront(rightWin);
	}

	short dur=player->HP();
	short breath=player->Breath();
	short satiation=player->Satiation();
	w->Unlock();

	werase(hudWin); 
	unsigned short i;
	//HitPoints line
	wstandend(hudWin);
	wprintw(hudWin, " HP: %3hd%% <", dur);
	wcolor_set(hudWin, WHITE_RED, NULL);
	for (i=0; i<10*dur/max_durability; ++i)
		waddch(hudWin, '.');
	wstandend(hudWin);
	mvwaddstr(hudWin, 0, 21, ">\n");

	//breath line
	if ( -1!=breath ) {
		wprintw(hudWin, " BR: %3hd%% <", breath);
		wcolor_set(hudWin, WHITE_BLUE, NULL);
		for (i=0; i<10*breath/max_breath; ++i)
			waddch(hudWin, '.');
		wstandend(hudWin);
		mvwaddstr(hudWin, 1, 21, ">\n");
	}
	
	//satiation line
	if ( -1!=satiation ) {
		if ( seconds_in_day*time_steps_in_sec<satiation ) {
			wcolor_set(hudWin, BLUE_BLACK, NULL);
			mvwaddstr(hudWin, 2, 1, "Gorged\n");
		} else if ( 3*seconds_in_day*time_steps_in_sec/4<satiation ) {
			wcolor_set(hudWin, GREEN_BLACK, NULL);
			mvwaddstr(hudWin, 2, 1, "Full\n");
		} else if (seconds_in_day*time_steps_in_sec/4>satiation) {
			wcolor_set(hudWin, RED_BLACK, NULL);
			mvwaddstr(hudWin, 2, 1, "Hungry\n");
		}
	}

	wrefresh(hudWin);
	wrefresh(notifyWin);

	//fprintf(stderr, "player x: %hu, y: %hu", player->X(), player->Y());
}

void Screen::PrintNormal(WINDOW * const window) const {
	unsigned short k_start;
	short k_step;
	unsigned short playerZ=player->Z();
	
	const dirs dir=player->Dir();
	if ( UP==dir ) { //подготовка: откуда начинать отрисовку и куда идти: в направлении роста или уменьшения z
		k_start=playerZ+1;
		k_step=1;
	} else {
		//если игрок смотрит в сторону, то рисовать всё с уровнем игрока, если смотрит в пол, то только пол и ниже
		k_start=( DOWN==dir ) ? playerZ-1 : playerZ;
		k_step=-1;
	}
	wmove(window, 1, 1); //передвинуть курсор (невидимый) в 1,1. в клетке 0,0 начинается рамка.
	static const unsigned short start=(shred_width*w->NumShreds()-SCREEN_SIZE)/2;
	unsigned short i, j, k;
	for ( j=start; j<SCREEN_SIZE+start; ++j, waddstr(window, "\n_") )
	for ( i=start; i<SCREEN_SIZE+start; ++i )
		for (k=k_start; ; k+=k_step) //верх и низ лоскута (чанка) - непрозрачные нуль-камень и небесная твердь. за границы массива на выйдет.
			if ( w->Transparent(i, j, k) < 2 ) {
				if ( w->Enlightened(i, j, k) && player->Visible(i, j, k) ) {
					wcolor_set(window, Color(i, j, k), NULL);
					waddch(window, CharName(i, j, k));
					waddch(window, CharNumber(i, j, k));
				} else {
					wcolor_set(window, BLACK_BLACK, NULL);
					waddstr(window, "  ");
				}
				break;
			}
	
	wstandend(window); //вернуть окну стандартный цвет
	box(window, 0, 0); //рамка
	if ( UP==dir || DOWN==dir ) {
		mvwaddstr(window, 0, 1, ( UP==dir ) ? "Sky View" : "Ground View");
		Arrows(window , player->X()*2+1, player->Y()+1);
	} else
		mvwaddstr(window, 0, 1, "Normal View");
	mvwprintw(window, SCREEN_SIZE+1, 1, "Time: %ld", w->Time());
	wrefresh(window); //вывод на экран
}

void Screen::PrintFront(WINDOW * const window) const {
	dirs dir=player->Dir();
	if ( UP==dir || DOWN==dir ) {
		wstandend(window);
		werase(window);
		box(window, 0, 0);
		mvwaddstr(window, 0, 1, "No view");
		wrefresh(window);
		return;
	}

	//подготовка сложнее, чем в PrintNormal: не только откуда и куда отрисовывать, но и какие переменные менять по ходу дела (указатели) *x и *z
	short x_step, z_step,
	      x_end, z_end,
	      * x, * z,
	      i, j, k;
	unsigned short pX=player->X();
	unsigned short pY=player->Y();
	unsigned short pZ=player->Z();
	unsigned short x_start, z_start,
		       k_start,
	               arrow_Y, arrow_X;
	switch ( dir ) {
		case NORTH:
			x=&i;
			x_step=1;
			x_start=0;
			x_end=SCREEN_SIZE;
			z=&j;
			z_step=-1;
			z_start=pY-1;
			z_end=-1;
			arrow_X=pX*2+1;
		break;
		case SOUTH:
			x=&i;
			x_step=-1;
			x_start=SCREEN_SIZE-1;
			x_end=-1;
			z=&j;
			z_step=1;
			z_start=pY+1;
			z_end=SCREEN_SIZE;
			arrow_X=(SCREEN_SIZE-pX)*2-1;
		break;
		case WEST:
			x=&j;
			x_step=-1;
			x_start=SCREEN_SIZE-1;
			x_end=-1;
			z=&i;
			z_step=-1;
			z_start=pX-1;
			z_end=-1;
			arrow_X=(SCREEN_SIZE-pY)*2-1;
		break;
		case EAST:
			x=&j;
			x_step=1;
			x_start=0;
			x_end=SCREEN_SIZE;
			z=&i;
			z_step=1;
			z_start=pX+1;
			z_end=SCREEN_SIZE;
			arrow_X=pY*2+1;
		break;
		default:
			fprintf(stderr, "Screen::PrintFront(WINDOW *): unlisted dir: %d\n", (int)dir);
			return;
	}
	if (pZ+SCREEN_SIZE/2>=height) {
		k_start=height-2;
		arrow_Y=height-pZ;
	} else if (pZ-SCREEN_SIZE/2<0) {
		k_start=SCREEN_SIZE-1;
		arrow_Y=SCREEN_SIZE-pZ;
	} else {
		k_start=pZ+SCREEN_SIZE/2;
		arrow_Y=SCREEN_SIZE/2+1;
	}
	/*
	wmove(window, 1, 1);
	for (k=k_start; k_start-k<SCREEN_SIZE; --k, waddstr(window, "\n_"))
		for (*x=x_start; *x!=x_end; *x+=x_step) {
			for (*z=z_start; *z!=z_end; *z+=z_step)
				if (w->Transparent(i, j, k) < 2) {
					if ( w->Enlightened(i, j, k) && w->Visible(i, j, k) ) {
						wcolor_set(window, Color(i, j, k), NULL);
						waddch(window, CharName(i, j, k));
						waddch(window, w->CharNumberFront(i, j));
					} else {
						wcolor_set(window, BLACK_BLACK, NULL);
						waddstr(window, "  ");
					}
					break;
				}
			if (*z==z_end) { //рисовать декорации дальнего вида (белые точки на синем)
				*z-=z_step;
				if (w->Visible(i, j, k)) {
				       	wcolor_set(window, WHITE_BLUE, NULL);
					waddstr(window, " .");
				} else {
					wcolor_set(window, BLACK_BLACK, NULL);
					waddstr(window, "  ");
				}
			}
		}
	*/
	wstandend(window);
	box(window, 0, 0);
	mvwaddstr(window, 0, 1, "Front View");
	Arrows(window, arrow_X, arrow_Y);
	wrefresh(window);
}

void Screen::PrintInv(WINDOW * const window, Inventory * const inv) const {
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
	QString str;
	char num_str[6];
	for (i=0; i<inventory_size; ++i) {
		mvwprintw(window, 2+i, 14, "%c)", 'a'+i);
		if ( inv->Number(i) ) {
			wcolor_set(window, Color(inv->GetInvKind(i), inv->GetInvSub(i)), NULL);
			mvwprintw( window, 2+i, 17, "[%c]%s",
					CharName( inv->GetInvKind(i),
					inv->GetInvSub(i) ),
					inv->InvFullName(str, i).toAscii().constData() );
			waddstr(window, inv->NumStr(num_str, i));
			wstandend(window);
			if ( ""!=str ) { //summ
				mvwprintw(window, 2+i, 50, "%2.1f kg", temp_weight=inv->GetInvWeight(i));
				sum_weight+=temp_weight;
			}
		}
	}
	mvwprintw(window, 2+i, 43, "Sum:%6.1f kg", sum_weight);
	if ( player->PlayerInventory()==inv ) {
		box(window, 0, 0);
		mvwaddstr(window, 0, 1, "Your inventory");
	} else {
		wcolor_set(window, Color(inv->Kind(), inv->Sub()), NULL);
		box(window, 0, 0);
		mvwprintw(window, 0, 1, "[%c]%s",
			CharName(inv->Kind(),
			inv->Sub()),
			inv->FullName(str).toAscii().constData());
	}
	wrefresh(window);
}

/*void Screen::GetSound(const unsigned short n, const unsigned short dist, const char sound, const kinds kind, const subs sub) {
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
}*/
/*void Screen::PrintSounds() {
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
}*/

void Screen::Notify(const QString str) {
	//TODO do not reprint after each notify
	static QString lines[5]={"", "", "", "", ""};
	const short MAX_LINES=5;
	wclear(notifyWin);
	for (unsigned short i=0; i<MAX_LINES-1; ++i) {
		lines[i]=lines[i+1];
		mvwaddstr(notifyWin, i, 0, lines[i].toAscii().constData());
	}
	mvwaddstr(notifyWin, MAX_LINES-1, 0,
		str.toAscii().constData());
	lines[MAX_LINES-1]=str;
	fputs(str.toAscii().constData(), notifyLog);
	updated=false;	
}

void Screen::Update(
		const unsigned short,
		const unsigned short,
		const unsigned short)
{
	updated=false;	
}
void Screen::UpdateAll() {
	updated=false;	
}
void Screen::UpdatePlayer() {
	updated=false;
}

Screen::Screen(
		World * const wor,
		Player * const pl)
		:
		VirtScreen(wor, pl),
		updated(false),
		cleaned(false),
		notifyLines(0)
{
	set_escdelay(10); //задержка после нажатия esc. для обработки esc-последовательностей, пока не используется.
	initscr(); //инициировать экран
	start_color();
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
	leftWin =newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, 0);
	rightWin=newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, SCREEN_SIZE*2+2);
	hudWin=newwin(3+2, (SCREEN_SIZE*2+2)*2-8, SCREEN_SIZE+2, 8); //окно для жизни, дыхания и т.д.
	notifyWin=newwin(0, COLS, SCREEN_SIZE+2+5, 0);
	//soundWin=newwin(3+2, 3*2+2, SCREEN_SIZE+2, 0);
	
	/*for (unsigned short i=0; i<9; ++i) {
		soundMap[i].ch=' ';
		soundMap[i].lev=0;
		soundMap[i].col=WHITE_BLACK;
	}*/

	input=new IThread();
	connect(input, SIGNAL(RePrintReceived()),
		this, SLOT(RePrint()),
		Qt::DirectConnection);

	connect(input, SIGNAL(ExitReceived()),
		this, SIGNAL(ExitReceived()));

	connect(input, SIGNAL(InputReceived(int, int)),
		this, SIGNAL(InputReceived(int, int)),
		Qt::DirectConnection);
	
	connect(this, SIGNAL(InputReceived(int, int)),
		player, SLOT(Act(int, int)),
		Qt::DirectConnection);

	input->start();

	timer=new QTimer(this);
	connect(timer, SIGNAL(timeout()),
		this, SLOT(Print()));
	timer->start(100);

	notifyLog=fopen("messages.txt", "a");
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
	//delwin(soundWin);
	//закончить оконный режим, очистить экран
	endwin();
	if ( NULL!=notifyLog )
		fclose(notifyLog);
}
