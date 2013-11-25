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
    * along with FREG. If not, see <http://www.gnu.org/licenses/>. */

/**\file screen.cpp
 * \brief This file is related to curses screen for freg. */

#include <QString>
#include <QTimer>
#include <QSettings>
#include <QDir>
#include <QMutex>
#include "screen.h"
#include "world.h"
#include "Block.h"
#include "Inventory.h"
#include "Player.h"

const char OBSCURE_BLOCK = ' ';
const int QUICK_INVENTORY_X_SHIFT = 36;

void Screen::Arrows(WINDOW * const window, const ushort x, const ushort y,
        const bool show_dir)
const {
    wcolor_set(window, WHITE_BLACK, NULL);
    if ( show_dir ) {
        mvwaddstr(window, 0, x-2, "N    N");
        mvwaddstr(window, SCREEN_SIZE+1, x-2, "S    S");
    }
    wcolor_set(window, WHITE_RED, NULL);
    mvwaddstr(window, 0, x, "@@");
    mvwaddstr(window, SCREEN_SIZE+1, x, "@@");
    HorizontalArrows(window, y, WHITE_RED, show_dir);
    (void)wmove(window, y, x);
}

void Screen::HorizontalArrows(WINDOW * const window, const ushort y,
        const short color, const bool show_dir)
const {
    wcolor_set(window, WHITE_BLACK, NULL);
    if ( show_dir ) {
        mvwaddch(window, y-1, 0, 'W');
        mvwaddch(window, y+1, 0, 'W');
        mvwaddch(window, y-1, SCREEN_SIZE*2+1, 'E');
        mvwaddch(window, y+1, SCREEN_SIZE*2+1, 'E');
    }
    wcolor_set(window, color, NULL);
    mvwaddch(window, y, 0, '@');
    mvwaddch(window, y, SCREEN_SIZE*2+1, '@');
}

void Screen::RePrint() {
    clear();
    updated = false;
    updatedPlayer = false;
}

void Screen::Update(const ushort, const ushort, const ushort) {
    updated = false;
}

void Screen::UpdateAll() {
    CleanFileToShow();
    updated = false;
}

void Screen::UpdatePlayer() {
    if ( player && ( USAGE_TYPE_READ_IN_INVENTORY==player->UsingType() ||
            player->GetCreativeMode() ) )
    {
        updated = false;
    }
    updatedPlayer = false;
}

void Screen::UpdateAround(const ushort, const ushort, const ushort,
        const ushort)
{
    updated = false;
}

void Screen::Move(const int) { updated = false; }

QString Screen::PassString(QString & str) const {
    mvwaddch(commandWin, 0, 0, ':');
    static const ushort NOTE_LENGTH = 144;
    char temp_str[NOTE_LENGTH+1];
    echo();
    wgetnstr(commandWin, temp_str, NOTE_LENGTH);
    noecho();
    werase(commandWin);
    wrefresh(commandWin);
    fprintf(notifyLog, "%lu: Command: %s\n", w->Time(), temp_str);
    return str = temp_str;
}

char Screen::CharNumber(const ushort x, const ushort y, const ushort z) const {
    if ( HEIGHT-1 == z ) { // sky
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
            fprintf(stderr, "Screen::CharNumber: (?) dir: %d\n",
                player->GetDir());
            return '*';
        }
    }
    const short z_dif = ( UP==player->GetDir() ) ?
        z - player->Z() : player->Z() - z;
    return ( !z_dif ) ?
        ' ' : ( z_dif<0 ) ?
            '-' : ( z_dif<10 ) ?
                z_dif+'0' : '+';
}

char Screen::CharNumberFront(const ushort i, const ushort j) const {
    const ushort dist =
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
    case PREDATOR: return '!';
    case WORKBENCH: return '*';
    case TELEGRAPH: return 't';
    case DOOR:        return ( STONE == sub ) ? '#' : '\'';
    case LOCKED_DOOR: return ( STONE == sub ) ? '#' : '`';
    case WEAPON: switch ( sub ) {
        default:
            fprintf(stderr, "Screen::CharName: weapon sub ?: %d\n",
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
            fprintf(stderr, "Screen::CharName: active sub ?: %d\n",
                sub);
    } // no break;
    default: switch ( sub ) {
        default: fprintf(stderr,
            "Screen::CharName: sub (?): %d\n", sub);
        case NULLSTONE:  case IRON: case CLAY:
        case MOSS_STONE: case WOOD: case GOLD:
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
        }
    }
} // char Screen::CharName(int kind, int sub)

color_pairs Screen::Color(const int kind, const int sub) const {
    switch ( kind ) { // foreground_background
    case PILE:      return WHITE_BLACK;
    case DWARF:     return WHITE_BLUE;
    case RABBIT:    return RED_WHITE;
    case PREDATOR:  return RED_BLACK;
    case TELEGRAPH: return CYAN_BLACK;
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
        case GOLD:       return WHITE_YELLOW;
        case SUN_MOON:   return ( NIGHT == w->PartOfDay() ) ?
            WHITE_WHITE : YELLOW_YELLOW;
        case SKY: case STAR: switch ( w->PartOfDay() ) {
            case NIGHT:   return WHITE_BLACK;
            case MORNING: return WHITE_BLUE;
            case NOON:    return BLACK_CYAN;
            case EVENING: return WHITE_CYAN;
            }
        default: return WHITE_BLACK;
        }
    }
} // color_pairs Screen::Color(int kind, int sub)

void Screen::ControlPlayer(const int ch) {
    CleanFileToShow();
    if ( 'Q'==ch || 3==ch || 4==ch ) { // Q, ctrl-c and ctrl-d
        emit ExitReceived();
        return;
    } // else:
    if ( ch>='a' && ch<='z' ) { // actions with inventory
        InventoryAction(ch-'a');
        return;
    } // else:
    switch ( ch ) { // interactions with world
    case KEY_UP:
        if ( player->GetDir() == NORTH ) {
            player->Move(NORTH);
        } else {
            player->SetDir(NORTH);
        }
    break;
    case KEY_DOWN:
        if ( player->GetDir() == SOUTH ) {
            player->Move(SOUTH);
        } else {
            player->SetDir(SOUTH);
        }
    break;
    case KEY_RIGHT:
        if ( player->GetDir() == EAST ) {
            player->Move(EAST);
        } else {
            player->SetDir(EAST);
        }
    break;
    case KEY_LEFT:
        if ( player->GetDir() == WEST ) {
            player->Move(WEST);
        } else {
            player->SetDir(WEST);
        }
    break;
    case KEY_END: player->Move(DOWN); break;
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

    case 'B': SetActionMode(BUILD);    break;
    case 'C': SetActionMode(CRAFT);    break;
    case 'D':
    case 'T': SetActionMode(THROW);    break;
    case 'E': SetActionMode(EAT);      break;
    case 'F': SetActionMode(TAKEOFF);  break;
    case 'I': SetActionMode(INSCRIBE); break;
    case 'O': SetActionMode(OBTAIN);   break;
    case 'U': SetActionMode(USE);      break;
    case 'W': SetActionMode(WIELD);    break;
    case KEY_HELP:
    case 'H':
        wstandend(rightWin);
        PrintFile(rightWin, "texts/help.txt");
    break;
    case 'L': RePrint(); break;
    case 'R':
        if ( !player->GetCreativeMode() ) {
            player->SetActiveHand(!player->IsRightActiveHand());
            Notify(tr("Now %1 hand is active.").
                arg(tr(player->IsRightActiveHand() ?
                    "right" : "left")));
        }
    break;

    case '-': shiftFocus = -!shiftFocus; break; // move focus down
    case '+': shiftFocus =  !shiftFocus; break; // move focus up

    case '!': player->SetCreativeMode( !player->GetCreativeMode() ); break;
    case ':':
    case '/': PassString(command); // no break
    case '.': ProcessCommand(command); break;

    default:
        Notify(tr("Don't know what such key means: %1 ('%2').").
            arg(ch).arg(char(ch)));
        Notify(tr("Press 'H' for help."));
    }
    mutex->lock();
    updated = false;
    mutex->unlock();
} // void Screen::ControlPlayer(int ch)

void Screen::ProcessCommand(QString & command) {
    if ( command.length()==1 && "."!=command ) {
        ControlPlayer(command.at(0).toAscii());
    } else if ( "warranty" == command ) {
        wstandend(rightWin);
        PrintFile(rightWin, "texts/warranty.txt");
    } else if ( "size" == command ) {
        Notify(QString("Terminal height: %1 lines, width: %2 chars.").
            arg(LINES).arg(COLS));
    } else if ( "moo" == command ) {
        Notify("^__^");
        Notify("(oo)\\_______");
        Notify("(__)\\       )\\/\\");
        Notify("    ||----w |");
        Notify("    ||     ||");
    } else {
        player->ProcessCommand(command);
    }
}

void Screen::SetActionMode(const int mode) {
    actionMode = mode;
    updatedPlayer = false;
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
        "Screen::InventoryAction: action mode ?: %d\n",
        actionMode);
    }
}

void Screen::ActionXyz(ushort & x,ushort & y, ushort & z) const {
    player->Focus(x, y, z);
    if (
            DOWN!=player->GetDir() &&
            UP  !=player->GetDir() &&
            ( AIR==w->GetBlock(x, y, z)->Sub() || AIR==w->GetBlock(
                player->X(),
                player->Y(),
                player->Z()+shiftFocus)->Sub() ))
    {
        z += shiftFocus;
    }
}

char Screen::PrintBlock(const Block * const block, WINDOW * const window)
const {
    const int kind = block->Kind();
    const int sub  = block->Sub();
    wcolor_set(window, Color(kind, sub), NULL);
    return CharName(kind, sub);
}

void Screen::Print() {
    if ( !player->IfPlayerExists() ) {
        return;
    }
    w->ReadLock();
    PrintHUD();
    mutex->lock();
    if ( updated ) {
        w->Unlock();
        mutex->unlock();
        return;
    }
    updated = true;
    mutex->unlock();
    const int dir = player->GetDir();
    switch ( player->UsingSelfType() ) { // left window
    case USAGE_TYPE_OPEN:
        if ( player->PlayerInventory() ) {
            PrintInv(leftWin, player->PlayerInventory());
            break;
        } // no break;
    default:
        PrintNormal(leftWin, (UP==dir || DOWN==dir) ?
            NORTH : dir);
    }
    if ( !fileToShow ) { // right window
        switch ( player->UsingType() ) {
        case USAGE_TYPE_READ_IN_INVENTORY:
            wstandend(rightWin);
            PrintFile(rightWin, QString(
                w->WorldName() + "/texts/" +
                player->PlayerInventory()->ShowBlock(
                    player->GetUsingInInventory())->
                        GetNote())
                    + ".txt");
            player->SetUsingTypeNo();
        break;
        case USAGE_TYPE_READ: {
            ushort x, y, z;
            ActionXyz(x, y, z);
            wstandend(rightWin);
            PrintFile(rightWin, QString(
                w->WorldName()+"/texts/"+
                w->GetBlock(x, y, z)->GetNote()+".txt"));
            player->SetUsingTypeNo();
        } break;
        case USAGE_TYPE_OPEN: {
            ushort x, y, z;
            ActionXyz(x, y, z);
            const Inventory * const inv =
                w->GetBlock(x, y, z)->HasInventory();
            if ( inv ) {
                PrintInv(rightWin, inv);
                break;
            } else {
                player->SetUsingTypeNo();
            }
        } // no break;
        default:
            if ( UP==dir || DOWN==dir ) {
                PrintNormal(rightWin, dir);
            } else {
                PrintFront(rightWin);
            }
        }
    }
    w->Unlock();
} // void Screen::Print()

void Screen::PrintHUD() {
    if ( updatedPlayer ) {
        return;
    }
    updatedPlayer = true;
    werase(hudWin);
    // quick inventory
    Inventory * const inv = player->PlayerInventory();
    if ( inv && COLS>=(SCREEN_SIZE*2+2)*2 ) {
        for (ushort i=0; i<inv->Size(); ++i) {
            wstandend(hudWin);
            const int x = QUICK_INVENTORY_X_SHIFT+i*2;
            mvwaddch(hudWin, 0, x, 'a'+i);
            const int number = inv->Number(i);
            if ( number ) {
                mvwaddch(hudWin, 1, x,
                    PrintBlock(inv->ShowBlock(i), hudWin));
                if ( number > 1 ) {
                    mvwaddch(hudWin, 2, x, number+'0');
                }
            }
        }
    }
    // action mode
    wstandend(hudWin);
    mvwaddstr(hudWin, 1, 0, "Action: ");
    switch ( actionMode ) {
    case THROW:    waddstr(hudWin, "Throw");  break;
    case OBTAIN:   waddstr(hudWin, "Obtain"); break;
    case WIELD:    waddstr(hudWin, "Wield");  break;
    case EAT:      waddstr(hudWin, "Eat");    break;
    case BUILD:    waddstr(hudWin, "Build");  break;
    case CRAFT:    waddstr(hudWin, "Craft");  break;
    case TAKEOFF:  waddstr(hudWin, "Take off"); break;
    case USE:      waddstr(hudWin, "Use in inventory"); break;
    case INSCRIBE: waddstr(hudWin, "Inscribe in inventory"); break;
    default:       waddstr(hudWin, "Unknown");
        fprintf(stderr, "Screen::Print: Unlisted actionMode: %d\n",
            actionMode);
    }
    if ( player->GetCreativeMode() ) {
        mvwaddstr(hudWin, 0, 0, "Creative Mode");
        // coordinates
        mvwprintw(hudWin, 2, 0, "xyz: %ld, %ld, %hu. XY: %ld, %ld",
            player->GlobalX(), player->GlobalY(), player->Z(),
            player->GetLatitude(), player->GetLongitude());
    } else {
        const short dur = player->HP();
        if ( -1 != dur ) { // HitPoints line
            wstandend(hudWin);
            const QString str = QString("%1").arg(dur, -10, 10,
                QChar('.'));
            mvwaddstr(hudWin, 0, 0, "HP[..........]");
            wcolor_set(hudWin, WHITE_RED, NULL);
            mvwaddstr(hudWin, 0, 3,
                qPrintable(str.left(10*dur/MAX_DURABILITY+1)));
        }
        const short breath = player->Breath();
        if ( -1!=breath && breath!=MAX_BREATH ) { // breath line
            wstandend(hudWin);
            const QString str =
                QString("%1").arg(breath, -10, 10, QChar('.'));
            mvwaddstr(hudWin, 0, 15, "BR[..........]");
            wcolor_set(hudWin, WHITE_BLUE, NULL);
            mvwaddstr(hudWin, 0, 15+3,
                qPrintable(str.left(10*breath/MAX_BREATH+1)));
        }
        const short satiation = player->SatiationPercent();
        if ( -1 != satiation ) { // satiation line
            (void)wmove(hudWin, 2, 0);
            if ( 100 < satiation ) {
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
    wrefresh(hudWin);
} // void Screen::PrintHUD()

void Screen::PrintNormal(WINDOW * const window, const int dir) const {
    const ushort k_start = ( UP!=dir ) ?
        (( DOWN==dir ) ? player->Z()-1 : player->Z()) :
        player->Z()+1;
    const short k_step = ( UP!=dir ) ? (-1) : 1;

    (void)wmove(window, 1, 1);
    const ushort start_x = ( player->X()/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    const ushort start_y = ( player->Y()/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    for (ushort j=start_y; j<SCREEN_SIZE+start_y;
            ++j, waddstr(window, "\n_"))
    for (ushort i=start_x; i<SCREEN_SIZE+start_x; ++i ) {
        ushort k = k_start;
        for ( ; INVISIBLE==w->GetBlock(i, j, k)->Transparent();
                k+=k_step);
        const Block * const block = w->GetBlock(i, j, k);
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
    wcolor_set(window, BLACK_WHITE, NULL);
    mvwaddstr(window, 0, 1, (UP==dir) ?
        ". Up ." : "x Ground x");
    Arrows(window, (player->X()-start_x)*2+1, player->Y()-start_y+1, true);
    wrefresh(window);
} // void Screen::PrintNormal(WINDOW * window, int dir)

void Screen::PrintFront(WINDOW * const window) const {
    if ( !window ) {
        return;
    } // else:
    const int dir = player->GetDir();
    short x_step, z_step,
          x_end,  z_end,
          * x, * z,
          i, j;
    const ushort pX = player->X();
    const ushort pY = player->Y();
    const ushort pZ = player->Z();
    const ushort begin_x = ( pX/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    const ushort begin_y = ( pY/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    ushort x_start, z_start, k_start;
    ushort arrow_Y, arrow_X;
    switch ( dir ) {
    default: fprintf(stderr, "Screen::PrintFront(): unlisted dir: %d\n",
        (int)dir);
    // no break;
    case NORTH:
        x = &i;
        x_step = 1;
        x_start = begin_x;
        x_end = x_start+SCREEN_SIZE;
        z = &j;
        z_step = -1;
        z_start = pY-1;
        z_end = pY-SHRED_WIDTH*2+1;
        arrow_X = (pX-begin_x)*2+1;
    break;
    case SOUTH:
        x = &i;
        x_step = -1;
        x_start = SCREEN_SIZE-1+begin_x;
        x_end = begin_x-1;
        z = &j;
        z_step = 1;
        z_start = pY+1;
        z_end = pY+SHRED_WIDTH*2-1;
        arrow_X = (SCREEN_SIZE-pX+begin_x)*2-1;
    break;
    case WEST:
        x = &j;
        x_step = -1;
        x_start = SCREEN_SIZE-1+begin_y;
        x_end = begin_y-1;
        z = &i;
        z_step = -1;
        z_start = pX-1;
        z_end = pX-SHRED_WIDTH*2+1;
        arrow_X = (SCREEN_SIZE-pY+begin_y)*2-1;
    break;
    case EAST:
        x = &j;
        x_step = 1;
        x_start = begin_y;
        x_end = SCREEN_SIZE+begin_y;
        z = &i;
        z_step = 1;
        z_start = pX+1;
        z_end = pX+SHRED_WIDTH*2-1;
        arrow_X = (pY-begin_y)*2+1;
    break;
    }
    if ( pZ+SCREEN_SIZE/2 >= HEIGHT ) {
        k_start = HEIGHT-2;
        arrow_Y = HEIGHT-pZ;
    } else if ( pZ-SCREEN_SIZE/2 < 0 ) {
        k_start = SCREEN_SIZE-1;
        arrow_Y = SCREEN_SIZE-pZ;
    } else {
        k_start = pZ+SCREEN_SIZE/2;
        arrow_Y = SCREEN_SIZE/2+1;
    }
    const int sky_colour = Color(BLOCK, SKY);
    const char sky_char = CharName(BLOCK, SKY);
    (void)wmove(window, 1, 1);
    for (short k=k_start; k>k_start-SCREEN_SIZE;
            --k, waddstr(window, "\n_"))
    {
        for (*x=x_start; *x!=x_end; *x+=x_step) {
            for (*z=z_start; *z!=z_end && w->GetBlock(i, j, k)->
                    Transparent()==INVISIBLE;
                    *z += z_step);
            const Block * const block = w->GetBlock(i, j, k);
            if ( (*z==z_end || (w->Enlightened(i, j, k)
                    && player->Visible(i, j, k)))
                        || player->GetCreativeMode() )
            {
                if ( *z != z_end ) {
                    waddch(window,
                        PrintBlock(block, window));
                    waddch(window, CharNumberFront(i, j));
                    continue;
                } else {
                    wcolor_set(window, sky_colour, NULL);
                    waddch(window, sky_char);
                }
            } else {
                wstandend(window);
                waddch(window, OBSCURE_BLOCK);
            }
            waddch(window, ' ');
        }
    }
    wstandend(window);
    box(window, 0, 0);
    wcolor_set(window, BLACK_WHITE, NULL);
    (void)wmove(window, 0, 1);
    switch ( dir ) {
    case NORTH: waddstr(window, "^ North ^"); break;
    case SOUTH: waddstr(window, "v South v"); break;
    case EAST:  waddstr(window, "> East >");  break;
    case WEST:  waddstr(window, "< West <");  break;
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
    wrefresh(window);
} // void Screen::PrintFront(WINDOW * window)

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
        const Block * const block = inv->ShowBlock(i);
        wprintw(window, "[%c]%s",
            PrintBlock(block, window),
            qPrintable(inv->InvFullName(i)) );
        if ( 1<inv->Number(i) ) {
            waddstr(window, qPrintable(inv->NumStr(i)));
        }
        const QString str = inv->GetInvNote(i);
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
    wrefresh(window);
} // void Screen::PrintInv(WINDOW * window, const Inventory * inv)

void Screen::PrintText(WINDOW * const window, QString const & str) const {
    werase(window);
    waddstr(window, qPrintable(str));
    wrefresh(window);
}

void Screen::CleanFileToShow() {
    delete fileToShow;
    fileToShow = 0;
}

bool Screen::PrintFile(WINDOW * const window, QString const & file_name) {
    CleanFileToShow();
    fileToShow = new QFile(file_name);
    if ( fileToShow->open(QIODevice::ReadOnly | QIODevice::Text) ) {
        PrintText(window, fileToShow->readAll());
        return true;
    } else {
        CleanFileToShow();
        return false;
    }
}

void Screen::Notify(const QString & str) const {
    fputs(qPrintable(QString("%1 %2\n").arg(w->TimeOfDayStr()).arg(str)),
        notifyLog);
    if ( str == DING ) {
        if ( beepOn ) {
            beep();
        }
    } else if ( str == OUCH ) {
        if ( beepOn ) {
            flash();
        }
        return;
    }
    waddstr(notifyWin, qPrintable(str));
    waddch(notifyWin, '\n');
    wrefresh(notifyWin);
}

void Screen::DeathScreen() {
    werase(rightWin);
    werase(hudWin);
    wcolor_set(leftWin, WHITE_RED, NULL);
    if ( !PrintFile(leftWin, "texts/death.txt") ) {
        waddstr(leftWin, "You die.\nWaiting for respawn...");
    }
    box(leftWin, 0, 0);
    wnoutrefresh(leftWin);
    wnoutrefresh(rightWin);
    wnoutrefresh(hudWin);
    doupdate();
    updated = true;
}

Screen::Screen(World * const wor, Player * const pl, int & error) :
        VirtScreen(wor, pl),
        leftWin(0),
        rightWin(0),
        notifyWin(0),
        commandWin(0),
        hudWin(0),
        input(new IThread(this)),
        updated(false),
        updatedPlayer(false),
        timer(new QTimer(this)),
        notifyLog(fopen("texts/messages.txt", "at")),
        fileToShow(0),
        mutex(new QMutex())
{
    setlocale(LC_ALL, "");
    #ifdef Q_OS_WIN32
        AllocConsole();
        freopen("conout$", "wt", stdout);
        freopen("conin$",  "rt", stdin);
    #else
        set_escdelay(10);
    #endif
    initscr();
    start_color();
    raw(); // send typed keys directly
    noecho(); // do not print typed symbols
    nonl();
    keypad(stdscr, TRUE); // use arrows
    if ( LINES < 41 ) {
        world->CleanAll();
        CleanAll();
        printf("Make your terminal height to be at least 41 lines.\n");
        error = HEIGHT_NOT_ENOUGH;
        return;
    }
    // all available color pairs (maybe some of them will not be used)
    const short colors[] = { // do not change colors order!
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
    const ushort preferred_width = (SCREEN_SIZE*2+2)*2;
    if ( COLS >= preferred_width ) {
        rightWin = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, COLS/2);
        leftWin  = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2,
            0, COLS/2-SCREEN_SIZE*2-2);
        hudWin = newwin(3, preferred_width,
            SCREEN_SIZE+2, COLS/2-SCREEN_SIZE*2-2);
    } else if ( COLS >= preferred_width/2 ) {
        rightWin = 0;
        leftWin  = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2,
            0, COLS/2-SCREEN_SIZE-1);
        hudWin = newwin(3, SCREEN_SIZE*2+2,
            SCREEN_SIZE+2, COLS/2-SCREEN_SIZE-1);
    } else {
        world->CleanAll();
        CleanAll();
        puts(qPrintable(
            QString("Set your terminal width at least %1 chars").
                arg(SCREEN_SIZE*2+2)));
        error = WIDTH_NOT_ENOUGH;
        return;
    }
    commandWin = newwin(1, COLS, SCREEN_SIZE+2+3, 0);
    notifyWin  = newwin(0, COLS, SCREEN_SIZE+2+4, 0);
    scrollok(notifyWin, TRUE);

    QSettings sett(QDir::currentPath()+"/freg.ini", QSettings::IniFormat);
    sett.beginGroup("screen_curses");
    shiftFocus = sett.value("focus_shift", 0).toInt();
    actionMode = sett.value("action_mode", USE).toInt();
    command    = sett.value("last_command", "hello").toString();
    beepOn     = sett.value("beep_on", true).toBool();
    sett.setValue("beep_on", beepOn);

    if ( !PrintFile(stdscr, "texts/splash.txt") ) {
        addstr("Free-Roaming Elementary Game\n");
        addstr("\nby mmaulwurff, with help of Panzerschrek\n");
    }
    printw("\nVersion %s.\n\nPress any key.", VER);
    const int ch = getch();
    qsrand(ch);
    ungetch(ch);
    erase();
    refresh();
    CleanFileToShow();
    Notify("+-------------Game started. Press 'H' for help.-------------+");
    if ( COLS < preferred_width ) {
        Notify("For better gameplay ");
        Notify(QString("set your terminal width at least %1 chars.").
            arg(preferred_width));
    }

    input->start();
    connect(timer, SIGNAL(timeout()), SLOT(Print()));
    timer->start(100);
} // Screen::Screen(World * wor, Player * pl)

void Screen::CleanAll() {
    static bool cleaned = false;
    if ( cleaned ) {
        return;
    }
    cleaned = true; // prevent double cleaning
    input->Stop();
    input->wait();
    delete input;
    delete timer;

    if ( leftWin   ) delwin(leftWin);
    if ( rightWin  ) delwin(rightWin);
    if ( notifyWin ) delwin(notifyWin);
    if ( hudWin    ) delwin(hudWin);
    endwin();
    if ( notifyLog ) {
        fclose(notifyLog);
    }
    delete fileToShow;
    delete mutex;
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

void IThread::Stop() { stopped = true; }
