    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2014 Alexander 'mmaulwurff' Kromm
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

#include <QTimer>
#include <QSettings>
#include <QDir>
#include <QMutex>
#include <QLocale>
#include "screen.h"
#include "world.h"
#include "blocks/Block.h"
#include "blocks/Inventory.h"
#include "Player.h"

const char OBSCURE_BLOCK = ' ';
const int QUICK_INVENTORY_X_SHIFT = 36;

void Screen::Arrows(WINDOW * const window, const ushort x, const ushort y,
        const bool show_dir)
const {
    wcolor_set(window, WHITE_BLACK, NULL);
    if ( show_dir ) {
        mvwaddstr(window, 0, x-2, qPrintable(tr("N    N")));
        mvwaddstr(window, SCREEN_SIZE+1, x-2, qPrintable(tr("S    S")));
    }
    wcolor_set(window, WHITE_RED, NULL);
    static const QString arrows_down(2, QChar(0x2193));
    static const QString arrows_up  (2, QChar(0x2191));
    mvwaddstr(window, 0, x, qPrintable(arrows_down));
    mvwaddstr(window, SCREEN_SIZE+1, x, qPrintable(arrows_up));
    HorizontalArrows(window, y, WHITE_RED, show_dir);
    (void)wmove(window, y, x);
}

void Screen::HorizontalArrows(WINDOW * const window, const ushort y,
        const short color, const bool show_dir)
const {
    wcolor_set(window, WHITE_BLACK, NULL);
    if ( show_dir ) {
        mvwaddstr(window, y-1, 0, qPrintable(tr("W")));
        mvwaddstr(window, y+1, 0, qPrintable(tr("W")));
        mvwaddstr(window, y-1, SCREEN_SIZE*2+1, qPrintable(tr("E")));
        mvwaddstr(window, y+1, SCREEN_SIZE*2+1, qPrintable(tr("E")));
    }
    wcolor_set(window, color, NULL);
    static const QString arrow_right(QChar(0x2192));
    static const QString arrow_left (QChar(0x2190));
    mvwaddstr(window, y, 0, qPrintable(arrow_right));
    mvwaddstr(window, y, SCREEN_SIZE*2+1, qPrintable(arrow_left));
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

void Screen::PassString(QString & str) const {
    mvwaddch(commandWin, 0, 0, ':');
    static const ushort NOTE_LENGTH = 144;
    char temp_str[NOTE_LENGTH+1];
    echo();
    wgetnstr(commandWin, temp_str, NOTE_LENGTH);
    noecho();
    werase(commandWin);
    wrefresh(commandWin);
    fprintf(notifyLog, "%lu: Command: %s\n", w->Time(), temp_str);
    str = QString::fromUtf8(temp_str);
}

char Screen::CharNumber(const ushort z) const {
    if ( HEIGHT-1 == z ) { // sky
        return ' ';
    }
    const short z_dif = ( UP==player->GetDir() ) ?
        z - player->Z() : player->Z() - z;
    return ( z_dif == 0 ) ?
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
    case GRASS:  return ( FIRE == sub ) ? 'f' : '.';
    case RABBIT: return 'r';
    case CLOCK:  return 'c';
    case PLATE:  return '_';
    case LADDER: return '^';
    case PICK:   return '\\';
    case SHOVEL: return '|';
    case HAMMER: return 'T';
    case AXE:    return '/';
    case CHEST:
    case PILE:   return '&';
    case BELL:   return 'b';
    case BUCKET: return 'u';
    case PREDATOR: return '!';
    case WORKBENCH: return '*';
    case TELEGRAPH: return 't';
    case DOOR:        return ( STONE == sub ) ? '#' : '\'';
    case LOCKED_DOOR: return ( STONE == sub ) ? '#' : '`';
    case WEAPON: switch ( sub ) {
        default: fprintf(stderr, "Screen::CharName: weapon sub ?: %d\n", sub);
        // no break;
        case STONE: return '.';
        case IRON: case BONE:
        case WOOD:  return '/';
    } break;
    case ACTIVE: switch ( sub ) {
        case SAND:  return '.';
        case WATER: return '*';
        case STONE: return ':';
        default: fprintf(stderr, "Screen::CharName: active sub ?: %d\n", sub);
    } // no break;
    default: switch ( sub ) {
        default: fprintf(stderr, "Screen::CharName: sub (?): %d\n", sub);
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
    } // no break;
    case ACTIVE: switch ( sub ) {
        case WATER: return CYAN_WHITE;
        case SAND:  return YELLOW_WHITE;
    } // no break;
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
        case BONE:       return MAGENTA_WHITE;
        case FIRE:       return RED_YELLOW;
        case SUN_MOON:   return ( NIGHT == w->PartOfDay() ) ?
            WHITE_WHITE : YELLOW_YELLOW;
        case SKY: case STAR: switch ( w->PartOfDay() ) {
            case NIGHT:   return WHITE_BLACK;
            case MORNING: return WHITE_BLUE;
            case NOON:    return CYAN_CYAN;
            case EVENING: return WHITE_CYAN;
            }
        default: return WHITE_BLACK;
        }
    }
} // color_pairs Screen::Color(int kind, int sub)

void Screen::ControlPlayer(const int ch) {
    CleanFileToShow();
    // Q, ctrl-c, ctrl-d, ctrl-q, ctrl-x
    // TODO: ctrl-z (to background) support
    if ( 'Q'==ch || 3==ch || 4==ch || 17==ch || 24==ch ) {
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

    case 'B': SetActionMode(ACTION_BUILD);    break;
    case 'C': SetActionMode(ACTION_CRAFT);    break;
    case 'D':
    case 'T': SetActionMode(ACTION_THROW);    break;
    case 'E': SetActionMode(ACTION_EAT);      break;
    case 'F': SetActionMode(ACTION_TAKEOFF);  break;
    case 'I': SetActionMode(ACTION_INSCRIBE); break;
    case 'G':
    case 'O': SetActionMode(ACTION_OBTAIN);   break;
    case 'U': SetActionMode(ACTION_USE);      break;
    case 'W': SetActionMode(ACTION_WIELD);    break;
    case KEY_HELP:
    case 'H':
        DisplayFile(QString("help_%1/help.txt")
            .arg(locale.left(2)));
    break;
    case 'L': RePrint(); break;
    case 'R': // switch active hand
        if ( not player->GetCreativeMode() ) {
            player->SetActiveHand(not player->IsRightActiveHand());
            Notify(tr("Now %1 hand is active.").
                arg(player->IsRightActiveHand() ?
                    tr("right") : tr("left")));
        }
    break;

    case '-': shiftFocus = -!shiftFocus; break; // move focus down
    case '+': shiftFocus =  !shiftFocus; break; // move focus up

    case '!': player->SetCreativeMode( not player->GetCreativeMode() ); break;
    case ':':
    case '/': PassString(command); // no break
    case '.': ProcessCommand(command); break;

    default: Notify(tr("Unknown key. Press 'H' for help."));
    }
    mutex->lock();
    updated = false;
    mutex->unlock();
} // void Screen::ControlPlayer(int ch)

void Screen::ProcessCommand(QString command) {
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
    case ACTION_USE:
        if ( player->Use(num) == USAGE_TYPE_POUR ) {
            ushort x, y, z;
            ActionXyz(x, y, z);
            player->Pour(x, y, z, num);
        }
    break;
    case ACTION_WIELD:    player->Wield(num);    break;
    case ACTION_INSCRIBE: player->Inscribe(num); break;
    case ACTION_EAT:      player->Eat(num);      break;
    case ACTION_CRAFT:    player->Craft(num);    break;
    case ACTION_TAKEOFF:  player->TakeOff(num);  break;
    case ACTION_OBTAIN: {
        ushort x, y, z;
        ActionXyz(x, y, z);
        player->Obtain(x, y, z, num);
    } break;
    case ACTION_THROW: {
        ushort x, y, z;
        ActionXyz(x, y, z);
        player->Throw(x, y, z, num);
    } break;
    case ACTION_BUILD: {
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
            DOWN != player->GetDir() &&
            UP   != player->GetDir() &&
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
    if ( not player->IfPlayerExists() ) {
        return;
    }
    w->ReadLock();
    mutex->lock();
    PrintHUD();
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
    if ( not fileToShow ) { // right window
        switch ( player->UsingType() ) {
        case USAGE_TYPE_READ_IN_INVENTORY:
            wstandend(rightWin);
            PrintFile(rightWin, QString(w->WorldName() + "/texts/"
                + player->PlayerInventory()->ShowBlock(
                    player->GetUsingInInventory())->GetNote()));
            player->SetUsingTypeNo();
        break;
        case USAGE_TYPE_READ: {
            ushort x, y, z;
            ActionXyz(x, y, z);
            wstandend(rightWin);
            PrintFile(rightWin, QString(w->WorldName() + "/texts/"
                + w->GetBlock(x, y, z)->GetNote()));
            player->SetUsingTypeNo();
        } break;
        case USAGE_TYPE_OPEN: {
            ushort x, y, z;
            ActionXyz(x, y, z);
            const Inventory * const inv = w->GetBlock(x, y, z)->HasInventory();
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
    if ( updatedPlayer ) return;
    int y_save, x_save;
    getyx(rightWin, y_save, x_save);

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
                mvwaddch(hudWin, 1, x, PrintBlock(inv->ShowBlock(i), hudWin));
                if ( number > 1 ) {
                    mvwaddch(hudWin, 2, x, number+'0');
                }
            }
        }
    }
    // action mode
    wstandend(hudWin);
    QString actionString(tr("Action: "));
    switch ( actionMode ) {
    case ACTION_THROW:    actionString.append(tr("Throw"));  break;
    case ACTION_OBTAIN:   actionString.append(tr("Obtain")); break;
    case ACTION_WIELD:    actionString.append(tr("Wield"));  break;
    case ACTION_EAT:      actionString.append(tr("Eat"));    break;
    case ACTION_BUILD:    actionString.append(tr("Build"));  break;
    case ACTION_CRAFT:    actionString.append(tr("Craft"));  break;
    case ACTION_TAKEOFF:  actionString.append(tr("Take off")); break;
    case ACTION_USE:      actionString.append(tr("Use in inventory")); break;
    case ACTION_INSCRIBE:
        actionString.append(tr("Inscribe in inventory"));
    break;
    default:
        actionString.append(tr("Unknown"));
        fprintf(stderr, "Screen::Print: Unlisted actionMode: %d\n",
            actionMode);
    }
    mvwaddstr(hudWin, 1, 0, qPrintable(actionString));
    if ( player->GetCreativeMode() ) {
        mvwaddstr(hudWin, 0, 0, qPrintable(tr("Creative Mode")));
        // coordinates
        mvwprintw(hudWin, 2, 0, "xyz: %ld, %ld, %hu. XY: %ld, %ld",
            player->GlobalX(), player->GlobalY(), player->Z(),
            player->GetLatitude(), player->GetLongitude());
    } else {
        const short dur = player->HP();
        if ( dur > 0 ) { // HitPoints line
            wstandend(hudWin);
            mvwprintw(hudWin, 0, 0, "[..........]%hd", dur);
            wcolor_set(hudWin, WHITE_RED, NULL);
            const QString str(10, QChar(0x2665));
            mvwaddstr(hudWin, 0, 1,
                qPrintable(str.left(10*dur/MAX_DURABILITY+1)));
        }
        const short breath = player->Breath();
        if ( -1!=breath && breath!=MAX_BREATH ) { // breath line
            wstandend(hudWin);
            const QString str(10, QChar(0x00b0));
            mvwprintw(hudWin, 0, 16, "[..........]%hd", breath);
            wcolor_set(hudWin, WHITE_BLUE, NULL);
            mvwaddstr(hudWin, 0, 14+3,
                qPrintable(str.left(10*breath/MAX_BREATH)));
        }
        const short satiation = player->SatiationPercent();
        if ( -1 != satiation ) { // satiation line
            if ( 100 < satiation ) {
                wcolor_set(hudWin, BLUE_BLACK, NULL);
                mvwaddstr(hudWin, 2, 0, qPrintable(tr("Gorged")));
            } else if ( 75 < satiation ) {
                wcolor_set(hudWin, GREEN_BLACK, NULL);
                mvwaddstr(hudWin, 2, 0, qPrintable(tr("Full")));
            } else if ( 25 > satiation ) {
                wcolor_set(hudWin, RED_BLACK, NULL);
                mvwaddstr(hudWin, 2, 0, qPrintable(tr("Hungry")));
            }
        }
    }
    (void)wmove(rightWin, y_save, x_save);
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
    for (ushort j=start_y; j<SCREEN_SIZE+start_y; ++j, waddstr(window, "\n_"))
    for (ushort i=start_x; i<SCREEN_SIZE+start_x; ++i ) {
        ushort k = k_start;
        for ( ; INVISIBLE==w->GetBlock(i, j, k)->Transparent(); k+=k_step);
        const Block * const block = w->GetBlock(i, j, k);
        if ( (w->Enlightened(i, j, k) && player->Visible(i, j, k)) ||
                player->GetCreativeMode() )
        {
            waddch(window, PrintBlock(block, window));
            waddch(window, CharNumber(k));
        } else {
            wstandend(window);
            waddch(window, OBSCURE_BLOCK);
            waddch(window, ' ');
        }
    }
    wstandend(window);
    mvwaddch(window, player->Y(), player->X()*2+3, '!');
    box(window, 0, 0);
    if ( player->IfPlayerExists() && dir > DOWN ) {
        const Block * const block = w->GetBlock(player->X(), player->Y(),
            player->Z());
        static const QString arrow_left (QChar(0x2190));
        static const QString arrow_up   (QChar(0x2191));
        static const QString arrow_right(QChar(0x2192));
        static const QString arrow_down (QChar(0x2193));
        wcolor_set(window, Color(block->Kind(), block->Sub()), NULL);
        (void)wmove(window, player->Y()-start_y+1, (player->X()-start_x)*2+2);
        switch ( player->GetDir() ) {
        case NORTH: waddstr(window, qPrintable(arrow_up));    break;
        case SOUTH: waddstr(window, qPrintable(arrow_down));  break;
        case EAST:  waddstr(window, qPrintable(arrow_right)); break;
        case WEST:  waddstr(window, qPrintable(arrow_left));  break;
        case UP:    waddch(window, '.'); break;
        case DOWN:  waddch(window, 'x'); break;
        default:
            fprintf(stderr, "Screen::PrintNormal: (?) dir: %d\n",
                player->GetDir());
            waddch(window, '*');
        }
    }
    wcolor_set(window, BLACK_WHITE, NULL);
    mvwaddstr(window, 0, 1, qPrintable((UP==dir) ?
        tr("(. Up .") : tr("x Down x")));
    Arrows(window, (player->X()-start_x)*2+1, player->Y()-start_y+1, true);

    wrefresh(window);
} // void Screen::PrintNormal(WINDOW * window, int dir)

void Screen::PrintFront(WINDOW * const window) const {
    if ( window == nullptr ) return;
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
    for (short k=k_start; k>k_start-SCREEN_SIZE; --k, waddstr(window, "\n_")) {
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
                    waddch(window, PrintBlock(block, window));
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
    QString dir_string;
    switch ( dir ) {
    case NORTH: dir_string = tr("^ North ^"); break;
    case SOUTH: dir_string = tr("v South v"); break;
    case EAST:  dir_string = tr("> East >");  break;
    case WEST:  dir_string = tr("< West <");  break;
    }
    waddstr(window, qPrintable(dir_string));
    if ( shiftFocus ) {
        HorizontalArrows(window, arrow_Y-shiftFocus, WHITE_BLUE);
        for (int i=arrow_Y-shiftFocus; i<SCREEN_SIZE+1 && i>0; i-=shiftFocus) {
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
        mvwaddstr(window, 2, 1, qPrintable(tr("      Head")));
        mvwaddstr(window, 3, 1, qPrintable(tr("Right hand")));
        mvwaddstr(window, 4, 1, qPrintable(tr(" Left hand")));
        mvwaddstr(window, 5, 1, qPrintable(tr("      Body")));
        mvwaddstr(window, 6, 1, qPrintable(tr("      Legs")));
    break;
    case WORKBENCH: mvwaddstr(window, 2, 4, qPrintable(tr("Product"))); break;
    }
    mvwprintw(window, 2+inv->Size(), 40,
        qPrintable(tr("All weight: %1 mz").arg(inv->Weight())));
    const int start = inv->Start();
    int shift = 0; // to divide inventory sections
    for (ushort i=0; i<inv->Size(); ++i) {
        if ( start == i && i != 0) {
            ++shift;
            mvwhline(window, 2+i, 0, ACS_HLINE, SCREEN_SIZE*2+2);
        }
        mvwprintw(window, 2+i+shift, 12, "%c)", 'a'+i);
        if ( not inv->Number(i) ) {
            continue;
        }
        const Block * const block = inv->ShowBlock(i);
        wprintw(window, "[%c]%s",
            PrintBlock(block, window),
            qPrintable(inv->InvFullName(i)) );
        if ( 1 < inv->Number(i) ) {
            waddstr(window, qPrintable(inv->NumStr(i)));
        }
        const QString str = inv->GetInvNote(i);
        if ( not str.isEmpty() ) {
            if ( str.size() < 24 ) {
                wprintw(window, " ~:%s", qPrintable(str));
            } else {
                wprintw(window, " ~:%s...", qPrintable(str.left(13)));
            }
        }
        wstandend(window);
        mvwprintw(window, 2+i+shift, 53, "%5hu mz", inv->GetInvWeight(i));
    }
    wcolor_set(window, Color(inv->Kind(), inv->Sub()), NULL);
    box(window, 0, 0);
    mvwprintw(window, 0, 1, "[%c]%s",
        CharName(inv->Kind(), inv->Sub()), qPrintable(
        ( player->PlayerInventory()==inv ) ?
            tr("Your inventory") : inv->FullName()));
    wrefresh(window);
} // void Screen::PrintInv(WINDOW * window, const Inventory * inv)

void Screen::CleanFileToShow() {
    delete fileToShow;
    fileToShow = 0;
}

bool Screen::PrintFile(WINDOW * const window, QString const & file_name) {
    CleanFileToShow();
    fileToShow = new QFile(file_name);
    if ( fileToShow->open(QIODevice::ReadOnly | QIODevice::Text) ) {
        werase(window);
        waddstr(window, qPrintable(
            QString::fromLocal8Bit(fileToShow->readAll().constData())) );
        wrefresh(window);
        return true;
    } else {
        CleanFileToShow();
        return false;
    }
}

void Screen::DisplayFile(QString path) {
    wstandend(rightWin);
    Notify( PrintFile(rightWin, path) ?
        tr("File path: %1/%2").arg(QDir::currentPath()).arg(path) :
        tr("There is no such help file: %1/%2.")
            .arg(QDir::currentPath()).arg(path) );
}

void Screen::Notify(const QString str) {
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
    if ( ++notificationRepeatCount && str == lastNotification ) {
        int x, y;
        getyx(notifyWin, y, x);
        (void)wmove(notifyWin, y-1, 0);
        wclrtoeol(notifyWin);
        wprintw(notifyWin, "%s (%dx)\n",
            qPrintable(str), notificationRepeatCount);
    } else {
        notificationRepeatCount = 1;
        wprintw(notifyWin, "%s\n", qPrintable(str));
        lastNotification = str;
    }
    wrefresh(notifyWin);
}

void Screen::DeathScreen() {
    werase(rightWin);
    werase(hudWin);
    Notify(DING);
    wcolor_set(leftWin, WHITE_RED, NULL);
    if ( not PrintFile(leftWin, "texts/death.txt") ) {
        waddstr(leftWin, qPrintable(tr("You die.\nWaiting for respawn...")));
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
        notificationRepeatCount(1),
        fileToShow(0),
        mutex(new QMutex())
{
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
        const ushort left_border = COLS/2-SCREEN_SIZE*2-2;
        rightWin = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, COLS/2);
        leftWin  = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, left_border);
        hudWin   = newwin(3, preferred_width, SCREEN_SIZE+2, left_border);
        commandWin = newwin(1, preferred_width, SCREEN_SIZE+2+3, left_border);
        notifyWin  = newwin(0, preferred_width, SCREEN_SIZE+2+4, left_border);
    } else if ( COLS >= preferred_width/2 ) {
        const ushort left_border = COLS/2-SCREEN_SIZE-1;
        rightWin = 0;
        leftWin  = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, left_border);
        hudWin   = newwin(3, SCREEN_SIZE*2+2, SCREEN_SIZE+2, left_border);
        commandWin = newwin(1, SCREEN_SIZE*2+2, SCREEN_SIZE+2+3, left_border);
        notifyWin  = newwin(0, SCREEN_SIZE*2+2, SCREEN_SIZE+2+4, left_border);
    } else {
        world->CleanAll();
        CleanAll();
        puts(qPrintable(
            tr("Set your terminal width at least %1 chars.").
                arg(SCREEN_SIZE*2+2)));
        error = WIDTH_NOT_ENOUGH;
        return;
    }
    scrollok(notifyWin, TRUE);

    QSettings sett(QDir::currentPath()+"/freg.ini", QSettings::IniFormat);
    sett.beginGroup("screen_curses");
    shiftFocus = sett.value("focus_shift", 0).toInt();
    actionMode = sett.value("action_mode", ACTION_USE).toInt();
    command    = sett.value("last_command", "hello").toString();
    beepOn     = sett.value("beep_on", true).toBool();
    sett.setValue("beep_on", beepOn);

    if ( not PrintFile(stdscr, "texts/splash.txt") ) {
        addstr("Free-Roaming Elementary Game\n");
        addstr("\nby mmaulwurff, with help of Panzerschrek\n");
    }
    printw(qPrintable(tr("\nVersion %1.\n\nPress any key.").arg(VER)));
    qsrand(getch());
    erase();
    refresh();
    CleanFileToShow();
    Notify(tr("*--- Game started. Press 'H' for help. ---*"));
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
    while ( not stopped ) {
        screen->ControlPlayer(getch());
        msleep(90);
        flushinp();
    }
}

void IThread::Stop() { stopped = true; }
