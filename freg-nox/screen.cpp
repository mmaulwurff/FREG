	/* freg, Free-Roaming Elementary Game with open and interactive world
	*  Copyright (C) 2012-2013 Alexander 'mmaulwurff' Kromm
	*  mmaulwurff@gmail.com
	*
	* This file is part of FREG.
	*
	* FREG is free software: you can redistribute it and/or modify
	* it under the terms of the GNU General Public License as published by
	* the Free Software Foundation, either version 3 of the License, or
	* (at your option) any later version.
	*
	* FREG is distributed in the hope that it will be useful,
	* but WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	* GNU General Public License for more details.
	*
	* You should have received a copy of the GNU General Public License
	* along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

/**\file screen.cpp
 * This file is related to curses screen for freg.
 */

#include <QString>
#include <QTimer>
#include <QSettings>
#include <QDir>
#include "screen.h"
#include "world.h"
#include "blocks.h"
#include "Player.h"

const char OBSCURE_BLOCK=' ';
const int QUICK_INVENTORY_X_SHIFT=36;

void Screen::Arrows(WINDOW * const window, const ushort x, const ushort y)
const {
	wcolor_set(window, WHITE_RED, NULL);
	mvwaddstr(window, 0, x, "vv");
	mvwaddstr(window, SCREEN_SIZE+1, x, "^^");
	HorizontalArrows(window, y);
}

void Screen::HorizontalArrows(WINDOW * const window,
		const ushort y,
		const short color)
const {
	wcolor_set(window, color, NULL);
	mvwaddch(window, y, 0, '>');
	mvwaddch(window, y, SCREEN_SIZE*2+1, '<');
}

void Screen::RePrint() {
	clear();
	updated=false;
	updatedPlayer=false;
}

void Screen::Update(const ushort, const ushort, const ushort) {
	updated=false;
}

void Screen::UpdateAll() {
	CleanFileToShow();
	updated=false;
}

void Screen::UpdatePlayer() {
	if ( player && ( USAGE_TYPE_READ_IN_INVENTORY==player->UsingType() ||
			player->GetCreativeMode() ) )
	{
		updated=false;
	}
	updatedPlayer=false;
}

void Screen::UpdateAround(const ushort, const ushort, const ushort,
		const ushort)
{
	updated=false;
}

void Screen::Move(const int) { updated=false; }

QString Screen::PassString(QString & str) const {
	mvwaddch(commandWin, 0, 0, ':');
	static const ushort NOTE_LENGTH=144;
	char temp_str[NOTE_LENGTH+1];
	echo();
	wgetnstr(commandWin, temp_str, NOTE_LENGTH);
	noecho();
	werase(commandWin);
	wrefresh(commandWin);
	fprintf(notifyLog, "%lu: Command: %s\n", w->Time(), temp_str);
	return str=temp_str;
}

char Screen::CharNumber(const ushort x, const ushort y, const ushort z) const {
	if ( HEIGHT-1==z ) { // sky
		return ' ';
	}
	if ( player->X()==x && player->Y()==y && player->Z()==z ) {
		switch ( player->GetDir() ) {
		case NORTH: return '^';
		case SOUTH: return 'v';
		case EAST:  return '>';
		case WEST:  return '<';
		case UP:    return '.';
		case DOWN:  return 'x';
		default:
			fprintf(stderr,
				"Screen::CharNumber: (?) dir: %d\n",
				player->GetDir());
			return '*';
		}
	}
	const short z_dif=( UP==player->GetDir() ) ?
		z - player->Z() :
		player->Z() - z;
	return ( !z_dif ) ?
		' ' :
		( z_dif<0 ) ?
			'-' :
			( z_dif<10 ) ?
				z_dif+'0' :
				'+';
}

char Screen::CharNumberFront(const ushort i, const ushort j) const {
	const ushort dist=
		(( NORTH==player->GetDir() || SOUTH==player->GetDir() ) ?
		abs(player->Y()-j) :
		abs(player->X()-i)) -1;
	return ( dist>9 ) ?
		'+' : ( dist>0 ) ?
			dist+'0' : ' ';

}

char Screen::CharName(const int kind, const int sub) const {
	switch ( kind )  {
	case BUSH:   return ';';
	case CREATOR:
	case DWARF:  return '@';
	case LIQUID: return '~';
	case GRASS:  return '.';
	case RABBIT: return 'r';
	case CLOCK:  return 'c';
	case PLATE:  return '_';
	case LADDER: return '^';
	case PICK:   return '\\';
	case CHEST:
	case PILE:   return '&';
	case BELL:   return 'b';
	case WORKBENCH: return '*';
	case TELEGRAPH: return 't';
	case DOOR:        return ( STONE==sub ) ? '#' : '\'';
	case LOCKED_DOOR: return ( STONE==sub ) ? '#' : '`';
	case WEAPON: switch ( sub ) {
		default:
			fprintf(stderr,
				"Screen::CharName: weapon sub ?: %d\n",
				sub);
		// no break;
		case STONE: return '.';
		case IRON:
		case WOOD:  return '/';
	} break;
	case ACTIVE: switch ( sub ) {
		case SAND:  return '.';
		case WATER: return '*';
		default:
			fprintf(stderr,
				"Screen::CharName: active sub ?: %d\n",
				sub);
	} // no break;
	default: switch ( sub ) {
		case NULLSTONE: case MOSS_STONE: case WOOD:
		case IRON: case CLAY:
		case STONE: return '#';
		case GLASS: return 'g';
		case AIR:   return ' ';
		case STAR:  return '.';
		case WATER: return '~';
		case SAND:  return '#';
		case SOIL:  return '.';
		case ROSE:  return ';';
		case PAPER: return '~';
		case A_MEAT: case H_MEAT:
		case HAZELNUT: return ',';
		case SKY:
		case SUN_MOON: return ' ';
		case GREENERY: return '%';
		default:
			fprintf(stderr,
				"Screen::CharName: unlisted sub: %d\n",
				sub);
			return '?';
		}
	}
} // Screen::CharName

color_pairs Screen::Color(const int kind, const int sub) const {
	switch ( kind ) { // foreground_background
	case DWARF:     return WHITE_BLUE;
	case TELEGRAPH: return CYAN_BLACK;
	case RABBIT:    return RED_WHITE;
	case BUSH:      return BLACK_GREEN;
	case PILE:      return WHITE_BLACK;
	case LIQUID: switch ( sub ) {
		case WATER: return CYAN_BLUE;
		default:    return RED_YELLOW;
	}
	case ACTIVE: switch ( sub ) {
		case WATER: return CYAN_WHITE;
		case SAND:  return YELLOW_WHITE;
		default:    return WHITE_BLACK;
	}
	default: switch ( sub ) {
		case WOOD: case HAZELNUT:
		case SOIL:       return BLACK_YELLOW;
		case GREENERY:   return BLACK_GREEN;
		case STONE:      return BLACK_WHITE;
		case SAND:       return YELLOW_WHITE;
		case A_MEAT:     return WHITE_RED;
		case H_MEAT:     return BLACK_RED;
		case WATER:      return WHITE_CYAN;
		case GLASS:      return BLUE_WHITE;
		case NULLSTONE:  return WHITE_BLACK;
		case MOSS_STONE: return GREEN_WHITE;
		case IRON:       return WHITE_BLACK;
		case ROSE:       return RED_GREEN;
		case CLAY:       return WHITE_RED;
		case PAPER:      return MAGENTA_WHITE;
		case SUN_MOON:   return ( NIGHT==w->PartOfDay() ) ?
			WHITE_WHITE : YELLOW_YELLOW;
		case SKY: case STAR: switch ( w->PartOfDay() ) {
			case NIGHT:   return WHITE_BLACK;
			case MORNING: return WHITE_BLUE;
			case NOON:    return WHITE_CYAN;
			case EVENING: return WHITE_BLUE;
		}
		default: return WHITE_BLACK;
		}
	}
} // Screen::Color

void Screen::ControlPlayer(const int ch) {
	CleanFileToShow();
	if ( 'Q'==ch ) {
		emit ExitReceived();
		return;
	}
	if ( ch>='a' && ch<='z' ) { // actions with inventory
		InventoryAction(ch-'a');
		return;
	}
	switch ( ch ) { // interactions with world
	case KEY_UP:
		if ( player->GetDir()==NORTH ) {
			player->Move(NORTH);
		} else {
			player->SetDir(NORTH);
		}
	break;
	case KEY_DOWN:
		if ( player->GetDir()==SOUTH ) {
			player->Move(SOUTH);
		} else {
			player->SetDir(SOUTH);
		}
	break;
	case KEY_RIGHT:
		if ( player->GetDir()==EAST ) {
			player->Move(EAST);
		} else {
			player->SetDir(EAST);
		}
	break;
	case KEY_LEFT:
		if ( player->GetDir()==WEST ) {
			player->Move(WEST);
		} else {
			player->SetDir(WEST);
		}
	break;
	case KEY_END:   player->Move(DOWN); break;
	case ' ': player->Jump(); break;
	case '=': player->Move(); break;

	case '>': player->SetDir(World::TurnRight(player->GetDir())); break;
	case '<': player->SetDir(World::TurnLeft (player->GetDir())); break;
	case KEY_NPAGE: player->SetDir(DOWN); break;
	case KEY_PPAGE: player->SetDir(UP);   break;

	case KEY_HOME: player->Backpack(); break;
	case 8:
	case KEY_BACKSPACE: { // damage
		ushort x, y, z;
		ActionXyz(x, y, z);
		player->Damage(x, y, z);
	} break;
	case 13:
	case '\n': { // use
		ushort x, y, z;
		ActionXyz(x, y, z);
		player->Use(x, y, z);
	} break;
	case  '?': { // examine
		ushort x, y, z;
		ActionXyz(x, y, z);
		player->Examine(x, y, z);
	} break;
	case  '~': { // inscribe
		ushort x, y, z;
		ActionXyz(x, y, z);
		player->Inscribe(x, y, z);
	} break;
	case 27: /* esc */ player->StopUseAll(); break;

	case 'U': SetActionMode(USE);      break;
	case 'D':
	case 'T': SetActionMode(THROW);    break;
	case 'O': SetActionMode(OBTAIN);   break;
	case 'W': SetActionMode(WIELD);    break;
	case 'I': SetActionMode(INSCRIBE); break;
	case 'E': SetActionMode(EAT);      break;
	case 'B': SetActionMode(BUILD);    break;
	case 'C': SetActionMode(CRAFT);    break;
	case 'F': SetActionMode(TAKEOFF);  break;

	case KEY_HELP:
	case 'H':
		wstandend(rightWin);
		PrintFile(rightWin, "help.txt");
	break;

	case ';': {
		Inventory * const inv=player->PlayerInventory();
		if ( inv ) {
			player->MoveInsideInventory(
				inv->Start(), inv->Size()-1);
		}
	} break;
	case '-': shiftFocus = -!shiftFocus; break; // move focus down
	case '+': shiftFocus =  !shiftFocus; break; // move focus up

	case '!': player->SetCreativeMode( !player->GetCreativeMode() ); break;
	case '/': PassString(command); // no break
	case '.': player->ProcessCommand(command); break;

	case 'L': RePrint(); break;

	case KEY_MOUSE: MouseAction(); break;
	case 'R':
		if ( !player->GetCreativeMode() ) {
			player->SetActiveHand(!player->IsRightActiveHand());
			Notify(tr("Now %1 hand is active.").
				arg(tr(player->IsRightActiveHand() ?
					"right" : "left")));
		}
	break;
	default:
		Notify(tr("Don't know what such key means: %1 ('%2').").
			arg(ch).
			arg(char(ch)));
		Notify(tr("Press 'H' for help."));
	}
	updated=false;
} // Screen::ControlPlayer

void Screen::SetActionMode(const int mode) {
	actionMode=mode;
	updatedPlayer=false;
}

void Screen::MouseAction() {
	#ifdef Q_OS_LINUX // mouse events are in ncurses, but not in pdcurses
	MEVENT mouse_event;
	getmouse(&mouse_event);
	if ( !wenclose(hudWin, mouse_event.y, mouse_event.x) ) {
		return;
	}
	wmouse_trafo(hudWin, &mouse_event.y, &mouse_event.x, FALSE);
	if ( mouse_event.x>=QUICK_INVENTORY_X_SHIFT
			&& mouse_event.x < QUICK_INVENTORY_X_SHIFT+INV_SIZE*2)
	{
		Notify(QString("In place '%1':").
			arg(char((mouse_event.x-QUICK_INVENTORY_X_SHIFT)/2+
				'a')));
		InventoryAction((mouse_event.x -QUICK_INVENTORY_X_SHIFT)/2);
	}
	#endif
}

void Screen::InventoryAction(const ushort num) const {
	switch ( actionMode ) {
	case USE:      player->Use(num);      break;
	case WIELD:    player->Wield(num);    break;
	case INSCRIBE: player->Inscribe(num); break;
	case EAT:      player->Eat(num);      break;
	case CRAFT:    player->Craft(num);    break;
	case TAKEOFF:  player->TakeOff(num);  break;
	case OBTAIN: {
		ushort x, y, z;
		ActionXyz(x, y, z);
		player->Obtain(x, y, z, num);
	} break;
	case THROW: {
		ushort x, y, z;
		ActionXyz(x, y, z);
		player->Throw(x, y, z, num);
	} break;
	case BUILD: {
		ushort x, y, z;
		ActionXyz(x, y, z);
		player->Build(x, y, z, num);
	} break;
	default: fprintf(stderr,
		"Screen::InventoryActiov: action mode ?: %d\n",
		actionMode);
	}
}

void Screen::ActionXyz(ushort & x,ushort & y, ushort & z) const {
	player->Focus(x, y, z);
	if (
			DOWN!=player->GetDir() &&
			UP  !=player->GetDir() &&
			( AIR==w->Sub(x, y, z) || AIR==w->Sub(
				player->X(),
				player->Y(),
				player->Z()+shiftFocus) ))
	{
		z+=shiftFocus;
	}
}

char Screen::PrintBlock(const Block * const block, WINDOW * const window)
const {
	const int kind=block->Kind();
	const int sub =block->Sub();
	wcolor_set(window, Color(kind, sub), NULL);
	return CharName(kind, sub);
}

void Screen::Print() {
	if ( !player ) {
		return;
	}
	w->ReadLock();
	PrintHUD();
	if ( updated ) {
		w->Unlock();
		return;
	}
	updated=true;
	if ( !fileToShow ) { // right window
		switch ( player->UsingType() ) {
		case USAGE_TYPE_READ_IN_INVENTORY:
			wstandend(rightWin);
			PrintFile(rightWin, QString(
				w->WorldName()+"/texts/"+
				player->PlayerInventory()->ShowBlock(
					player->GetUsingInInventory())->
						GetNote()));
			player->SetUsingTypeNo();
		break;
		case USAGE_TYPE_READ: {
			ushort x, y, z;
			ActionXyz(x, y, z);
			wstandend(rightWin);
			PrintFile(rightWin, QString(
				w->WorldName()+"/texts/"+
				w->GetBlock(x, y, z)->GetNote()));
			player->SetUsingTypeNo();
		} break;
		case USAGE_TYPE_OPEN: {
			ushort x, y, z;
			ActionXyz(x, y, z);
			const Inventory * const inv=
				w->GetBlock(x, y, z)->HasInventory();
			if ( inv ) {
				PrintInv(rightWin, inv);
				break;
			} else {
				player->SetUsingTypeNo();
			}
		} // no break;
		default:
			if ( UP==player->GetDir() || DOWN==player->GetDir() ) {
				PrintNormal(rightWin, player->GetDir());
			} else {
				PrintFront(rightWin);
			}
		}
	}
	switch ( player->UsingSelfType() ) { // left window
	case USAGE_TYPE_OPEN:
		if ( player->PlayerInventory() ) {
			PrintInv(leftWin, player->PlayerInventory());
			break;
		} // no break;
	default:
		PrintNormal(leftWin,
				( UP==player->GetDir() ||
				DOWN==player->GetDir() ) ?
			NORTH : player->GetDir());
	}
	w->Unlock();
	doupdate();
} // Screen::Print

void Screen::PrintHUD() {
	if ( updatedPlayer ) {
		return;
	} else {
		updatedPlayer=true;
	}
	werase(hudWin);
	// quick inventory
	Inventory * const inv=player->PlayerInventory();
	if ( inv ) {
		for (ushort i=0; i<inv->Size(); ++i) {
			wstandend(hudWin);
			const int x=QUICK_INVENTORY_X_SHIFT+i*2;
			mvwaddch(hudWin, 1, x, 'a'+i);
			const int number=inv->Number(i);
			if ( number ) {
				mvwaddch(hudWin, 2, x,
					PrintBlock(inv->ShowBlock(i), hudWin));
				if ( number > 1 ) {
					mvwprintw(hudWin, 3, x, "%hu", number);
				}
			}
		}
	}
	// action mode
	wstandend(hudWin);
	mvwaddstr(hudWin, 1, 0, "Action: ");
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
	if ( player->GetCreativeMode() ) {
		wstandend(hudWin);
		mvwaddstr(hudWin, 0, 0, "Creative Mode");
		// coordinates
		mvwprintw(hudWin, 2, 0, "xyz: %ld, %ld, %hu. XY: %ld, %ld",
			player->GlobalX(), player->GlobalY(), player->Z(),
			player->GetLatitude(), player->GetLongitude());
		wcolor_set(hudWin, BLACK_WHITE, NULL);
		(void)wmove(hudWin, 0, SCREEN_SIZE*2-8);
		switch ( player->GetDir() ) {
		case NORTH: waddstr(hudWin, "^ North ^"); break;
		case SOUTH: waddstr(hudWin, "v South v"); break;
		case EAST:  waddstr(hudWin, ">   East>"); break;
		case WEST:  waddstr(hudWin, "<West   <"); break;
		case DOWN:  waddstr(hudWin, "x DOWN  x"); break;
		case UP:    waddstr(hudWin, ".   UP  ."); break;
		}
	} else {
		const short dur=player->HP();
		if ( -1!=dur ) { // HitPoints line
			wstandend(hudWin);
			const QString str=QString("%1").arg(dur, -10, 10,
				QChar('.'));
			mvwaddstr(hudWin, 0, 0, "HP[..........]");
			wcolor_set(hudWin, WHITE_RED, NULL);
			mvwaddstr(hudWin, 0, 3,
				qPrintable(str.left(10*dur/MAX_DURABILITY+1)));
		}
		const short breath=player->Breath();
		if ( -1!=breath ) { // breath line
			wstandend(hudWin);
			const QString str=
				QString("%1").arg(breath, -10, 10, QChar('.'));
			mvwaddstr(hudWin, 0, 15, "BR[..........]");
			wcolor_set(hudWin, WHITE_BLUE, NULL);
			mvwaddstr(hudWin, 0, 15+3,
				qPrintable(str.left(10*breath/MAX_BREATH+1)));
		}
		const short satiation=player->SatiationPercent();
		if ( -1!=satiation ) { // satiation line
			(void)wmove(hudWin, 2, 0);
			if ( 100<satiation ) {
				wcolor_set(hudWin, BLUE_BLACK, NULL);
				waddstr(hudWin, "Gorged");
			} else if ( 75<satiation ) {
				wcolor_set(hudWin, GREEN_BLACK, NULL);
				waddstr(hudWin, "Full");
			} else if (25>satiation) {
				wcolor_set(hudWin, RED_BLACK, NULL);
				waddstr(hudWin, "Hungry");
			}
		}
	}
	wnoutrefresh(hudWin);
} // Screen::PrintHUD

void Screen::PrintNormal(WINDOW * const window, const int dir) const {
	const ushort k_start=( UP!=dir ) ?
		(( DOWN==dir ) ? player->Z()-1 : player->Z()) :
		player->Z()+1;
	const short k_step=( UP!=dir ) ? (-1) : 1;

	(void)wmove(window, 1, 1);
	const ushort start_x=( player->X()/SHRED_WIDTH )*SHRED_WIDTH +
		( SHRED_WIDTH-SCREEN_SIZE )/2;
	const ushort start_y=( player->Y()/SHRED_WIDTH )*SHRED_WIDTH +
		( SHRED_WIDTH-SCREEN_SIZE )/2;
	for (ushort j=start_y; j<SCREEN_SIZE+start_y;
			++j, waddstr(window, "\n_"))
	for (ushort i=start_x; i<SCREEN_SIZE+start_x; ++i ) {
		ushort k=k_start;
		const Block * block;
		for ( ; INVISIBLE==(block=w->GetBlock(i, j, k))->Transparent();
				k+=k_step);
		if ( (w->Enlightened(i, j, k) && player->Visible(i, j, k)) ||
				player->GetCreativeMode() )
		{
			waddch(window, PrintBlock(block, window));
			waddch(window, CharNumber(i, j, k));
		} else {
			wstandend(window);
			waddch(window, OBSCURE_BLOCK);
			waddch(window, ' ');
		}
	}
	wstandend(window);
	box(window, 0, 0);
	if ( UP==dir || DOWN==dir ) {
		mvwaddstr(window, 0, 1, ( UP==dir ) ?
			"Up view" : "Ground view");
		Arrows(window,
			(player->X()-start_x)*2+1, player->Y()-start_y+1);
	} else {
		mvwaddstr(window, 0, 1, "Down view");
		if ( player->GetCreativeMode() ) {
			Arrows(window,
				(player->X()-start_x)*2+1,
				player->Y()-start_y+1);
		}
	}
	wnoutrefresh(window);
} // Screen::PrintNormal

void Screen::PrintFront(WINDOW * const window) const {
	const int dir=player->GetDir();
	short x_step, z_step,
	      x_end, z_end,
	      * x, * z,
	      i, j;
	const ushort pX=player->X();
	const ushort pY=player->Y();
	const ushort pZ=player->Z();
	const ushort begin_x = ( pX/SHRED_WIDTH )*SHRED_WIDTH +
		( SHRED_WIDTH-SCREEN_SIZE )/2;
	const ushort begin_y = ( pY/SHRED_WIDTH )*SHRED_WIDTH +
		( SHRED_WIDTH-SCREEN_SIZE )/2;
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
		z_end=pY-SHRED_WIDTH-1;
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
		z_end=pY+SHRED_WIDTH+1;
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
		z_end=pX-SHRED_WIDTH-1;
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
		z_end=pX+SHRED_WIDTH+1;
		arrow_X=(pY-begin_y)*2+1;
	break;
	default:
		fprintf(stderr,
			"Screen::PrintFront(): unlisted dir: %d\n",
			(int)dir);
		return;
	}
	if ( pZ+SCREEN_SIZE/2 >= HEIGHT ) {
		k_start=HEIGHT-2;
		arrow_Y=HEIGHT-pZ;
	} else if ( pZ-SCREEN_SIZE/2 < 0 ) {
		k_start=SCREEN_SIZE-1;
		arrow_Y=SCREEN_SIZE-pZ;
	} else {
		k_start=pZ+SCREEN_SIZE/2;
		arrow_Y=SCREEN_SIZE/2+1;
	}
	(void)wmove(window, 1, 1);
	for (short k=k_start; k>k_start-SCREEN_SIZE;
			--k, waddstr(window, "\n_"))
	{
		for (*x=x_start; *x!=x_end; *x+=x_step) {
			const Block * block;
			for (*z=z_start; *z!=z_end &&
				(block=w->GetBlock(i, j, k))->
					Transparent()==INVISIBLE; *z+=z_step);
			if ( (*z==z_end || (w->Enlightened(i, j, k) &&
					player->Visible(i, j, k))) ||
						player->GetCreativeMode() )
			{
				if ( *z!=z_end ) {
					waddch(window,
						PrintBlock(block, window));
					waddch(window, CharNumberFront(i, j));
				} else {
					wcolor_set(window, Color(BLOCK, SKY),
						NULL);
					waddch(window, CharName(BLOCK, SKY));
					waddch(window, ' ');
				}
			} else {
				wstandend(window);
				waddch(window, OBSCURE_BLOCK);
				waddch(window, ' ');
			}
		}
	}
	wstandend(window);
	box(window, 0, 0);
	switch ( dir ) {
	case NORTH: mvwaddstr(window, 0, 1, "North view"); break;
	case SOUTH: mvwaddstr(window, 0, 1, "South view"); break;
	case EAST:  mvwaddstr(window, 0, 1, "East view");  break;
	case WEST:  mvwaddstr(window, 0, 1, "West view");  break;
	}
	if ( shiftFocus ) {
		HorizontalArrows(window, arrow_Y-shiftFocus, WHITE_BLUE);
		for (ushort i=arrow_Y-shiftFocus; i<SCREEN_SIZE+1 && i>0;
				i-=shiftFocus)
		{
			mvwaddch(window, i, 0, '|');
			mvwaddch(window, i, SCREEN_SIZE*2+1, '|');
		}
	}
	Arrows(window, arrow_X, arrow_Y);
	wnoutrefresh(window);
} // Screen::PrintFront

void Screen::PrintInv(WINDOW * const window, const Inventory * const inv)
const {
	werase(window);
	wstandend(window);
	switch ( inv->Kind() ) {
	case DWARF:
		mvwaddstr(window, 2, 7, "Head\n Right hand\n  Left hand\n");
		waddstr(window, "       Body\n       Legs");
	break;
	case WORKBENCH: mvwaddstr(window, 2, 4, "Product"); break;
	}
	mvwprintw(window, 2+inv->Size(), 40, "All weight: %6hu mz",
		inv->Weight());
	for (ushort i=0; i<inv->Size(); ++i) {
		mvwprintw(window, 2+i, 12, "%c)", 'a'+i);
		if ( !inv->Number(i) ) {
			continue;
		}
		const Block * const block=inv->ShowBlock(i);
		wprintw(window, "[%c]%s",
			PrintBlock(block, window),
			qPrintable(inv->InvFullName(i)) );
		if ( 1<inv->Number(i) ) {
			waddstr(window, qPrintable(inv->NumStr(i)));
		}
		const QString str=inv->GetInvNote(i);
		if ( !str.isEmpty() ) {
			waddstr(window, " ~:");
			if ( str.size() < 24 ) {
				waddstr(window, qPrintable(str));
			} else {
				waddstr(window, qPrintable(str.left(13)));
				waddstr(window, "...");
			}
		}
		wstandend(window);
		mvwprintw(window, 2+i, 53, "%5hu mz", inv->GetInvWeight(i));
	}
	wcolor_set(window, Color(inv->Kind(), inv->Sub()), NULL);
	box(window, 0, 0);
	mvwprintw(window, 0, 1, "[%c]%s",
		CharName(inv->Kind(), inv->Sub()),
		( player->PlayerInventory()==inv ) ?
			"Your inventory" : qPrintable(inv->FullName()));
	wnoutrefresh(window);
} // Screen::PrintInv

void Screen::PrintText(WINDOW * const window, QString const & str) const {
	werase(window);
	waddstr(window, qPrintable(str));
	wrefresh(window);
}

void Screen::CleanFileToShow() {
	delete fileToShow;
	fileToShow=0;
}

bool Screen::PrintFile(WINDOW * const window, QString const & file_name) {
	CleanFileToShow();
	fileToShow=new QFile(file_name);
	if ( fileToShow->open(QIODevice::ReadOnly | QIODevice::Text) ) {
		PrintText(window, fileToShow->readAll());
		return true;
	} else {
		CleanFileToShow();
		return false;
	}
}

void Screen::Notify(const QString & str) const {
	waddstr(notifyWin, qPrintable(str));
	waddch(notifyWin, '\n');
	if ( str==SOUND_STRINGS[0] ) {
		if ( beepOn ) {
			beep();
		}
	} else if ( str==SOUND_STRINGS[1] ) {
		if ( beepOn ) {
			flash();
		}
	}
	wrefresh(notifyWin);
	fputs(qPrintable(QString("%1: %2\n").arg(w->Time()).arg(str)),
		notifyLog);
}

void Screen::DeathScreen() {
	werase(rightWin);
	werase(hudWin);
	(void)wmove(leftWin, 1, 1);
	wcolor_set(leftWin, WHITE_RED, NULL);
	if ( !PrintFile(leftWin, "death.txt") ) {
		waddstr(leftWin, "You die.\nWaiting for respawn...");
	}
	box(leftWin, 0, 0);
	wnoutrefresh(leftWin);
	wnoutrefresh(rightWin);
	wnoutrefresh(hudWin);
	doupdate();
	updated=true;
}

Screen::Screen(World * const wor, Player * const pl) :
		VirtScreen(wor, pl),
		input(new IThread(this)),
		updated(false),
		updatedPlayer(false),
		cleaned(false),
		timer(new QTimer(this)),
		notifyLog(fopen("messages.txt", "at")),
		fileToShow(0)
{
	setlocale(LC_ALL, "");
	#ifdef Q_OS_WIN32 // by Panzerschrek
		AllocConsole();
		freopen( "conout$", "w", stdout );
		freopen( "conin$", "r", stdin );
	#else
		set_escdelay(10);
	#endif
	resize_term( SCREEN_SIZE+2 + 3 + 1 + 5, (SCREEN_SIZE*2 + 2)*2 );
	initscr();
	start_color();
	raw(); // send typed keys directly
	noecho(); // do not print typed symbols
	nonl();
	keypad(stdscr, TRUE); // use arrows
	curs_set(0); // invisible cursor
	// all available color pairs (maybe some of them will not be used)
	const short colors[]={ // do not change colors order!
		COLOR_BLACK,
		COLOR_RED,
		COLOR_GREEN,
		COLOR_YELLOW,
		COLOR_BLUE,
		COLOR_MAGENTA,
		COLOR_CYAN,
		COLOR_WHITE
	};
	for (short i=BLACK_BLACK; i<=WHITE_WHITE; ++i) {
		init_pair(i, colors[(i-1)/8], colors[(i-1)%8]);
	}
	rightWin=newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, COLS/2);
	leftWin =newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0,
		COLS/2-SCREEN_SIZE*2-2);
	hudWin=newwin(4, (SCREEN_SIZE*2+2)*2, SCREEN_SIZE+2,
		COLS/2-SCREEN_SIZE*2-2);
	commandWin=newwin(1, COLS, SCREEN_SIZE+2+4, 0);
	notifyWin =newwin(0, COLS, SCREEN_SIZE+2+5, 0);
	scrollok(notifyWin, TRUE);

	mousemask(BUTTON1_PRESSED, NULL);
	mouseinterval(0);

	QSettings sett(QDir::currentPath()+"/freg.ini", QSettings::IniFormat);
	sett.beginGroup("screen_curses");
	shiftFocus=sett.value("focus_shift", 0).toInt();
	actionMode=sett.value("action_mode", USE).toInt();
	command   =sett.value("last_command", "hello").toString();
	beepOn    =sett.value("beep_on", true).toBool();
	sett.setValue("beep_on", beepOn);

	if ( !PrintFile(stdscr, "splash.txt") ) {
		addstr("Free-Roaming Elementary Game\n");
		addstr("\nby mmaulwurff, with help of Panzerschrek\n");
	}
	printw("\nVersion %s.\n\nPress any key.", VER);
	const int ch=getch();
	qsrand(ch);
	ungetch(ch);
	erase();
	refresh();
	CleanFileToShow();
	Notify("Game started. Press 'H' for help.");

	input->start();
	connect(timer, SIGNAL(timeout()), this, SLOT(Print()));
	timer->start(100);
} // Screen::Screen

void Screen::CleanAll() {
	// TODO: make own lock
	w->WriteLock();
	if ( cleaned ) {
		w->Unlock();
		return;
	}

	cleaned=true; // prevent double cleaning
	input->Stop();
	input->wait();
	delete input;
	delete timer;
	w->Unlock();

	delwin(leftWin);
	delwin(rightWin);
	delwin(notifyWin);
	delwin(hudWin);
	endwin();
	if ( notifyLog ) {
		fclose(notifyLog);
	}
	delete fileToShow;
	QSettings sett(QDir::currentPath()+"/freg.ini", QSettings::IniFormat);
	sett.beginGroup("screen_curses");
	sett.setValue("focus_shift", shiftFocus);
	sett.setValue("action_mode", actionMode);
	sett.setValue("last_command", command);
}

Screen::~Screen() { CleanAll(); }

IThread::IThread(Screen * const scr) :
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
