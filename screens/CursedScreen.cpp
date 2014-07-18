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

/**\file CursedScreen.cpp
 * \brief This file is related to curses screen for freg. */

#include <QSettings>
#include <QDir>
#include <QLocale>
#include <QMutexLocker>
#include "screens/CursedScreen.h"
#include "screens/IThread.h"
#include "Shred.h"
#include "blocks/Block.h"
#include "blocks/Inventory.h"
#include "Player.h"

const char OBSCURE_BLOCK = ' ';
const int QUICK_INVENTORY_X_SHIFT = 36;

void Screen::Arrows(WINDOW * const window, const int x, const int y,
        const bool show_dir)
const {
    wcolor_set(window, WHITE_BLACK, nullptr);
    if ( show_dir ) {
        mvwaddstr(window, 0, x-2, qPrintable(tr("N    N")));
        mvwaddstr(window, SCREEN_SIZE+1, x-2, qPrintable(tr("S    S")));
    }
    wcolor_set(window, WHITE_RED, nullptr);
    static const QString arrows_down(2, QChar(ascii ? 'v' : 0x2193));
    static const QString arrows_up  (2, QChar(ascii ? '^' : 0x2191));
    mvwaddstr(window, 0, x, qPrintable(arrows_down));
    mvwaddstr(window, SCREEN_SIZE+1, x, qPrintable(arrows_up));
    HorizontalArrows(window, y, WHITE_RED, show_dir);
    (void)wmove(window, y, x);
}

void Screen::HorizontalArrows(WINDOW * const window, const int y,
        const int color, const bool show_dir)
const {
    wcolor_set(window, WHITE_BLACK, nullptr);
    if ( show_dir ) {
        mvwaddstr(window, y-1, 0, qPrintable(tr("W")));
        mvwaddstr(window, y+1, 0, qPrintable(tr("W")));
        mvwaddstr(window, y-1, SCREEN_SIZE*2+1, qPrintable(tr("E")));
        mvwaddstr(window, y+1, SCREEN_SIZE*2+1, qPrintable(tr("E")));
    }
    wcolor_set(window, color, nullptr);
    static const QString arrow_right(QChar(ascii ? '>' : 0x2192));
    static const QString arrow_left (QChar(ascii ? '<' : 0x2190));
    mvwaddstr(window, y, 0, qPrintable(arrow_right));
    mvwaddstr(window, y, SCREEN_SIZE*2+1, qPrintable(arrow_left));
}

void Screen::RePrint() {
    clear();
    mvwaddstr(actionWin, 0, 0, qPrintable(tr("Use")));
    mvwaddstr(actionWin, 1, 0, qPrintable(tr("Throw")));
    mvwaddstr(actionWin, 2, 0, qPrintable(tr("Obtain")));
    mvwaddstr(actionWin, 3, 0, qPrintable(tr("iNscribe")));
    mvwaddstr(actionWin, 4, 0, qPrintable(tr("Build")));
    mvwaddstr(actionWin, 5, 0, qPrintable(tr("Craft")));
    refresh();
    wrefresh(actionWin);
    updated = false;
}

void Screen::Update(int, int, int) { updated = false; }
void Screen::UpdatePlayer() { updated = false; }
void Screen::UpdateAround(int, int, int, int) { updated = false; }
void Screen::Move(int) { updated = false; }

void Screen::UpdateAll() {
    CleanFileToShow();
    updated = false;
}

void Screen::PassString(QString & str) const {
    inputActive = true;
    wstandend(notifyWin);
    waddstr(notifyWin, "\n:");
    char temp_str[MAX_NOTE_LENGTH + 1];
    echo();
    wgetnstr(notifyWin, temp_str, MAX_NOTE_LENGTH);
    inputActive = false;
    noecho();
    lastNotification = str;
    fprintf(notifyLog, "%lu: Command: %s\n", w->Time(), temp_str);
    str = QString::fromUtf8(temp_str);
}

char Screen::CharNumber(const int z) const {
    if ( HEIGHT-1 == z ) { // sky
        return ' ';
    }
    const int z_dif = ( UP==player->GetDir() ) ?
        z - player->Z() : player->Z() - z;
    return ( z_dif == 0 ) ?
        ' ' : ( z_dif<0 ) ?
            '-' : ( z_dif<10 ) ?
                z_dif+'0' : '+';
}

char Screen::CharNumberFront(const int i, const int j) const {
    const int dist = (( NORTH==player->GetDir() || SOUTH==player->GetDir() ) ?
        abs(player->Y()-j) :
        abs(player->X()-i)) -1;
    return ( dist>9 ) ?
        '+' : ( dist>0 ) ?
            dist+'0' : ' ';
}

int Screen::Color(const int kind, const int sub) const {
    switch ( sub ) {
    case ACID: return COLOR_PAIR(GREEN_GREEN) | A_BOLD | A_REVERSE;
    }
    switch ( kind ) { // foreground_background
    case LIQUID: switch ( sub ) {
        case WATER:     return COLOR_PAIR(CYAN_BLUE);
        case SUB_CLOUD: return COLOR_PAIR(WHITE_WHITE);
        default:        return COLOR_PAIR(RED_YELLOW);
    } // no break);
    case FALLING: switch ( sub ) {
        case WATER: return COLOR_PAIR(CYAN_WHITE);
        case SAND:  return COLOR_PAIR(YELLOW_WHITE);
    } // no break);
    default: switch ( sub ) {
        default: return COLOR_PAIR(WHITE_BLACK);
        case STONE:      return COLOR_PAIR(BLACK_WHITE);
        case GREENERY:   return COLOR_PAIR(BLACK_GREEN);
        case WOOD:
        case HAZELNUT:
        case SOIL:       return COLOR_PAIR(BLACK_YELLOW);
        case SAND:       return COLOR_PAIR(YELLOW_WHITE);
        case COAL:       return COLOR_PAIR(BLACK_WHITE);
        case IRON:       return COLOR_PAIR(BLACK_BLACK) | A_BOLD;
        case A_MEAT:     return COLOR_PAIR(WHITE_RED);
        case H_MEAT:     return COLOR_PAIR(BLACK_RED);
        case WATER:      return COLOR_PAIR(WHITE_CYAN);
        case GLASS:      return COLOR_PAIR(BLUE_WHITE);
        case NULLSTONE:  return COLOR_PAIR(MAGENTA_BLACK) | A_BOLD;
        case MOSS_STONE: return COLOR_PAIR(GREEN_WHITE);
        case ROSE:       return COLOR_PAIR(RED_GREEN);
        case CLAY:       return COLOR_PAIR(WHITE_RED);
        case PAPER:      return COLOR_PAIR(MAGENTA_WHITE);
        case GOLD:       return COLOR_PAIR(WHITE_YELLOW);
        case BONE:       return COLOR_PAIR(MAGENTA_WHITE);
        case FIRE:       return COLOR_PAIR(RED_YELLOW) | A_BLINK;
        case EXPLOSIVE:  return COLOR_PAIR(WHITE_RED);
        case SUN_MOON:   return COLOR_PAIR(( TIME_NIGHT == w->PartOfDay() ) ?
            WHITE_WHITE : YELLOW_YELLOW);
        case SKY:
        case STAR:
            if ( w->GetEvernight() ) return COLOR_PAIR(BLACK_BLACK);
            switch ( w->PartOfDay() ) {
            case TIME_NIGHT:   return COLOR_PAIR(WHITE_BLACK) | A_BOLD;
            case TIME_MORNING: return COLOR_PAIR(WHITE_BLUE);
            case TIME_NOON:    return COLOR_PAIR(CYAN_CYAN);
            case TIME_EVENING: return COLOR_PAIR(WHITE_CYAN);
            }
        case SUB_DUST: return COLOR_PAIR(BLACK_BLACK) | A_BOLD | A_REVERSE;
        }
    case DWARF:     return COLOR_PAIR(WHITE_BLUE);
    case RABBIT:    return COLOR_PAIR(RED_WHITE);
    case PREDATOR:  return COLOR_PAIR(RED_BLACK);
    case TELEGRAPH: return COLOR_PAIR(CYAN_BLACK);
    }
} // color_pairs Screen::Color(int kind, int sub)

color_pairs Screen::ColorShred(const int type) {
    switch ( type ) { // foreground_background
    case 'c':
    case '^': return BLACK_WHITE;
    case '.': return BLACK_GREEN;
    case '~': return CYAN_BLUE;
    case '#': return MAGENTA_BLACK;
    case '%': return YELLOW_GREEN;
    case '+': return WHITE_GREEN;
    default:  return WHITE_BLACK;
    }
}

void Screen::MovePlayer(const dirs dir) {
    if ( player->GetDir() == dir ) {
        player->Move(dir);
    } else {
        player->SetDir(dir);
    }
}

void Screen::MovePlayerDiag(const dirs dir1, const dirs dir2) const {
    player->SetDir(dir1);
    static bool step_trigger = true;
    player->Move(step_trigger ? dir1 : dir2);
    step_trigger = !step_trigger;
}

int Screen::GetChar() const { return getch(); }
void Screen::FlushInput() const { flushinp(); }

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
    default:
        Notify(tr("Unknown key. Press 'H' for help."));
        #ifndef QT_NO_DEBUG
        Notify(QString("Pressed key code: %1.").arg(ch));
        #endif
        break;
    case KEY_UP:    case '8': MovePlayer(NORTH);  break;
    case KEY_DOWN:  case '2': MovePlayer(SOUTH);  break;
    case KEY_RIGHT: case '6': MovePlayer(EAST);   break;
    case KEY_LEFT:  case '4': MovePlayer(WEST);   break;
    case KEY_END:   case '5': player->Move(DOWN); break;
    case '7': MovePlayerDiag(NORTH, WEST); break;
    case '9': MovePlayerDiag(NORTH, EAST); break;
    case '1': MovePlayerDiag(SOUTH, WEST); break;
    case '3': MovePlayerDiag(SOUTH, EAST); break;
    case ' ': player->Jump(); break;

    case '>': player->SetDir(World::TurnRight(player->GetDir())); break;
    case '<': player->SetDir(World::TurnLeft (player->GetDir())); break;
    case KEY_NPAGE: player->SetDir(DOWN); break;
    case KEY_PPAGE: player->SetDir(UP);   break;

    case 'I':
    case KEY_HOME: player->Backpack(); break;
    case 8:
    case KEY_DC: // delete
    case KEY_BACKSPACE: player->Damage(); break;
    case 13:
    case '\n': player->Use();      break;
    case  '?': player->Examine();  break;
    case  '~': player->Inscribe(); break;
    case 27: /* esc */ player->StopUseAll(); break;

    case KEY_IC: player->Build(1); break; // insert
    case 'B': SetActionMode(ACTION_BUILD);    break;
    case 'C': SetActionMode(ACTION_CRAFT);    break;
    case 'D':
    case 'T': SetActionMode(ACTION_THROW);    break;
    case 'N': SetActionMode(ACTION_INSCRIBE); break;
    case 'G':
    case 'O': SetActionMode(ACTION_OBTAIN);   break;
    case 'F':
    case 'W':
    case 'E':
    case 'U': SetActionMode(ACTION_USE);      break;
    case 'S':
        if ( player->PlayerInventory() ) {
              player->PlayerInventory()->Shake();
        }
    break;
    case KEY_HELP:
    case 'H':
        DisplayFile(QString("help_%1/help.txt").arg(locale.left(2)));
    break;
    case 'R':
    case 'L': RePrint(); break;

    case '-': shiftFocus = -!shiftFocus; break; // move focus down
    case '+': shiftFocus =  !shiftFocus; break; // move focus up

    case '!': player->SetCreativeMode( not player->GetCreativeMode() ); break;
    case ':':
    case '/': PassString(previousCommand); // no break
    case '.': ProcessCommand(previousCommand); break;
    }
    updated = false;
} // void Screen::ControlPlayer(int ch)

void Screen::ProcessCommand(QString command) {
    if ( command.length()==1 && "."!=command ) {
        ControlPlayer(command.at(0).toLatin1());
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

void Screen::SetActionMode(const actions mode) {
    mvwchgat(actionWin, actionMode,      0,20, A_NORMAL, WHITE_BLACK, nullptr);
    mvwchgat(actionWin, actionMode=mode, 0,20, A_NORMAL, BLACK_WHITE, nullptr);
    wrefresh(actionWin);
    updated = false;
}

void Screen::InventoryAction(const int num) const {
    switch ( actionMode ) {
    case ACTION_USE:      player->Use     (num); break;
    case ACTION_THROW:    player->Throw   (num); break;
    case ACTION_OBTAIN:   player->Obtain  (num); break;
    case ACTION_INSCRIBE: player->Inscribe(num); break;
    case ACTION_BUILD:    player->Build   (num); break;
    case ACTION_CRAFT:    player->Craft   (num); break;
    }
}

void Screen::ActionXyz(int * x, int * y, int * z) const {
    VirtScreen::ActionXyz(x, y, z);
    if (
            DOWN != player->GetDir() &&
            UP   != player->GetDir() &&
            ( AIR==w->GetBlock(*x, *y, *z)->Sub() || AIR==w->GetBlock(
                player->X(),
                player->Y(),
                player->Z()+shiftFocus)->Sub() ))
    {
        *z += shiftFocus;
    }
}

char Screen::PrintBlock(const Block & block, WINDOW * const window) const {
    const int kind = block.Kind();
    const int sub  = block.Sub();
    wattrset(window, Color(kind, sub));
    return CharName(kind, sub);
}

void Screen::Print() {
    if ( not player->IfPlayerExists() || updated ) return;
    updated = true;
    w->Lock();
    PrintHUD();
    const dirs dir = player->GetDir();
    if ( player->UsingSelfType() != USAGE_TYPE_OPEN ) { // left window
        PrintNormal(leftWin, (UP==dir || DOWN==dir) ? NORTH : dir);
    } else if ( player->PlayerInventory() != nullptr ) {
        PrintInv(leftWin, *player->PlayerInventory());
    }
    if ( fileToShow == nullptr ) { // right window
        switch ( player->UsingType() ) {
        default:
            if ( UP==dir || DOWN==dir ) {
                PrintNormal(rightWin, dir);
            } else if ( rightWin != nullptr ) {
                PrintFront(rightWin, dir);
            }
            break;
        case USAGE_TYPE_READ_IN_INVENTORY:
            wstandend(rightWin);
            PrintFile(rightWin, QString(w->WorldName() + "/texts/"
                + player->PlayerInventory()->ShowBlock(
                    player->GetUsingInInventory())->GetNote()));
            player->SetUsingTypeNo();
            break;
        case USAGE_TYPE_READ: {
                int x, y, z;
                ActionXyz(&x, &y, &z);
                wstandend(rightWin);
                PrintFile(rightWin, QString(w->WorldName() + "/texts/"
                    + w->GetBlock(x, y, z)->GetNote()));
                player->SetUsingTypeNo();
            } break;
        case USAGE_TYPE_OPEN: {
                int x, y, z;
                ActionXyz(&x, &y, &z);
                const Inventory * const inv =
                    w->GetBlock(x, y, z)->HasInventory();
                if ( inv != nullptr ) {
                    PrintInv(rightWin, *inv);
                    break;
                } else {
                    player->SetUsingTypeNo();
                }
            } break;
        }
    }
    w->Unlock();
} // void Screen::Print()

void Screen::PrintHUD() {
    werase(hudWin);
    if ( player->GetCreativeMode() ) {
        mvwaddstr (hudWin, 0, 0,
            qPrintable(tr("Creative Mode\nxyz: %1, %2, %3. XY: %4, %5.")
            .arg(player->GlobalX()).arg(player->GlobalY()).arg(player->Z())
            .arg(player->GetLatitude()).arg(player->GetLongitude())) );
    } else {
        const int dur = player->HP();
        if ( dur > 0 ) { // HitPoints line
            PrintBar(0,
                (dur > MAX_DURABILITY/5) ?
                    COLOR_PAIR(RED_BLACK) : (COLOR_PAIR(BLACK_RED) | A_BLINK),
                ascii ? '@' : 0x2665, dur*100/MAX_DURABILITY);
        }
        const int breath = player->BreathPercent();
        if ( -100!=breath && breath!=100 ) { // breath line
            PrintBar(16, COLOR_PAIR(BLUE_BLACK), ascii ? 'o' : 0x00b0, breath);
        }
        switch ( player->SatiationPercent()/25 ) { // satiation status
        case  1:
        case  2:
        default: break;
        case  0:
            wcolor_set(hudWin, RED_BLACK, nullptr);
            mvwaddstr(hudWin, 1, 1, qPrintable(tr("Hungry")));
            break;
        case  3:
            wcolor_set(hudWin, GREEN_BLACK, nullptr);
            mvwaddstr(hudWin, 1, 1, qPrintable(tr("Full")));
            break;
        case  4:
            wcolor_set(hudWin, BLUE_BLACK, nullptr);
            mvwaddstr(hudWin, 1, 1, qPrintable(tr("Gorged")));
            break;
        }
    }
    // focused block
    int x, y, z;
    ActionXyz(&x, &y, &z);
    Block * const focused = GetWorld()->GetBlock(x, y, z);
    if ( not IsLikeAir(focused->Sub()) ) {
        PrintBar(((SCREEN_SIZE*2+2) * (IsScreenWide() ? 2 : 1)) - 15,
            Color(focused->Kind(), focused->Sub()),
            (focused->IsAnimal() == nullptr) ? '+' : '*',
            focused->GetDurability()*100/MAX_DURABILITY,
            false);
    }
    PrintQuickInventory();
    wrefresh(hudWin);
    PrintMiniMap();
} // void Screen::PrintHUD()

void Screen::PrintQuickInventory() {
    Inventory * const inv = player->PlayerInventory();
    if ( inv!=nullptr && IsScreenWide() ) {
        for (int i=inv->Size()-1; i>=0; --i) {
            wstandend(hudWin);
            const int x = QUICK_INVENTORY_X_SHIFT+i*2;
            mvwaddch(hudWin, 0, x, 'a'+i);
            switch ( inv->Number(i) ) {
            case  0: break;
            default: mvwaddch(hudWin, 2, x, inv->Number(i)+'0'); // no break;
            case  1: mvwaddch(hudWin, 1, x,
                    PrintBlock(*inv->ShowBlock(i), hudWin));
                break;
            }
        }
    }
}

void Screen::PrintMiniMap() {
    wmove(miniMapWin, 1, 0);
    const int x_center = Shred::CoordOfShred(player->X());
    const int y_center = Shred::CoordOfShred(player->Y());
    const int j_start = qMax(x_center-2, 0);
    const int j_end = qMin(x_center+2, w->NumShreds()-1);
    const int i_end = qMin(y_center+2, w->NumShreds()-1);
    for (int i=qMax(y_center-2, 0); i<=i_end; ++i, waddch(miniMapWin, '\n'))
    for (int j=j_start;             j<=j_end; ++j) {
        Shred * const shred = w->GetShredByPos(j, i);
        if ( shred == nullptr ) {
            wstandend(miniMapWin);
            waddstr(miniMapWin, "  ");
        } else {
            wcolor_set(miniMapWin,ColorShred(shred->GetTypeOfShred()),nullptr);
            wprintw(miniMapWin, " %c", shred->GetTypeOfShred());
        }
    }
    wstandend(miniMapWin);
    box(miniMapWin, 0, 0);
    wrefresh(miniMapWin);
}

void Screen::PrintNormal(WINDOW * const window, dirs dir) const {
    int k_start, k_step;
    if ( UP == dir ) {
        k_start = player->Z()+1;
        k_step  = 1;
    } else {
        k_start = player->Z() - ( DOWN==dir );
        k_step  = -1;
    }
    (void)wmove(window, 1, 1);
    const int start_x = ( player->X()/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    const int start_y = ( player->Y()/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    for (int j=start_y; j<SCREEN_SIZE+start_y; ++j, waddstr(window, "\n_"))
    for (int i=start_x; i<SCREEN_SIZE+start_x; ++i ) {
        Shred * const shred = w->GetShred(i, j);
        const int i_in = Shred::CoordInShred(i);
        const int j_in = Shred::CoordInShred(j);
        int k = k_start;
        for ( ; INVISIBLE == shred->GetBlock(i_in, j_in, k)->Transparent();
            k += k_step);
        if ( player->Visible(i, j, k) ) {
            waddch(window, PrintBlock(*shred->GetBlock(i_in,j_in,k), window));
            waddch(window, CharNumber(k));
        } else {
            wstandend(window);
            waddch(window, OBSCURE_BLOCK);
            waddch(window, ' ');
        }
    }
    if ( player->IfPlayerExists() && dir > DOWN ) {
        static const QString arrow_left (QChar(ascii ? '<' : 0x2190));
        static const QString arrow_up   (QChar(ascii ? '^' : 0x2191));
        static const QString arrow_right(QChar(ascii ? '>' : 0x2192));
        static const QString arrow_down (QChar(ascii ? 'v' : 0x2193));
        const Block * const block =
            w->GetBlock(player->X(), player->Y(), player->Z());
        wattrset(window, Color(block->Kind(), block->Sub()));
        (void)wmove(window, player->Y()-start_y+1, (player->X()-start_x)*2+2);
        switch ( player->GetDir() ) {
        case UP:    waddch(window, '.'); break;
        case DOWN:  waddch(window, 'x'); break;
        case NORTH: waddstr(window, qPrintable(arrow_up));    break;
        case SOUTH: waddstr(window, qPrintable(arrow_down));  break;
        case EAST:  waddstr(window, qPrintable(arrow_right)); break;
        case WEST:  waddstr(window, qPrintable(arrow_left));  break;
        case NOWHERE: waddch(window, '*'); break;
        }
    }
    PrintTitle(window, UP==dir ? UP : DOWN);
    Arrows(window, (player->X()-start_x)*2+1, player->Y()-start_y+1, true);
    wrefresh(window);
} // void Screen::PrintNormal(WINDOW * window, int dir)

void Screen::PrintFront(WINDOW * const window, const dirs dir) const {
    int x_step, z_step,
        x_end,  z_end;
    int * x, * z;
    int i, j;
    const int pX = player->X();
    const int pY = player->Y();
    const int pZ = player->Z();
    const int begin_x = ( pX/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    const int begin_y = ( pY/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    int x_start, z_start, k_start;
    int arrow_Y, arrow_X;
    switch ( dir ) {
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
    default:
        fprintf(stderr, "%s: unlisted dir: %d.\n", Q_FUNC_INFO, int(dir));
        return;
    }
    if ( pZ+SCREEN_SIZE/2 >= HEIGHT-1 ) { // near top of the world
        k_start = HEIGHT-2;
        arrow_Y = HEIGHT-pZ;
    } else if ( pZ-SCREEN_SIZE/2 < 0 ) { // middle of the world
        k_start = SCREEN_SIZE-1;
        arrow_Y = SCREEN_SIZE-pZ;
    } else { // near bottom of the world
        k_start = pZ+SCREEN_SIZE/2;
        arrow_Y = SCREEN_SIZE/2+1;
    }
    const int sky_colour = Color(BLOCK, SKY);
    const char sky_char = CharName(BLOCK, SKY);
    (void)wmove(window, 1, 1);
    for (int k=k_start; k>k_start-SCREEN_SIZE; --k, waddstr(window, "\n_")) {
        for (*x=x_start; *x!=x_end; *x+=x_step) {
            for (*z=z_start; *z!=z_end && w->GetBlock(i, j, k)->
                        Transparent()==INVISIBLE;
                    *z += z_step);
            if ( *z == z_end ) {
                wattrset(window, sky_colour);
                waddch(window, sky_char);
                waddch(window, ' ');
            } else if ( player->Visible(i, j, k) ) {
                const Block * const block = w->GetBlock(i, j, k);
                waddch(window, PrintBlock(*block, window));
                waddch(window, CharNumberFront(i, j));
            } else {
                wstandend(window);
                waddch(window, OBSCURE_BLOCK);
                waddch(window, ' ');
            }
        }
    }
    PrintTitle(window, dir);
    if ( shiftFocus ) {
        HorizontalArrows(window, arrow_Y-shiftFocus, WHITE_BLUE);
        for (int q=arrow_Y-shiftFocus; q<SCREEN_SIZE+1 && q>0; q-=shiftFocus) {
            mvwaddch(window, q, 0, '|');
            mvwaddch(window, q, SCREEN_SIZE*2+1, '|');
        }
    }
    Arrows(window, arrow_X, arrow_Y);
    wrefresh(window);
} // void Screen::PrintFront(WINDOW * window)

void Screen::PrintTitle(WINDOW * const window, const dirs dir) const {
    QString dir_string;
    switch ( dir ) {
    case UP:    dir_string = tr(".  Up  .");  break;
    case DOWN:  dir_string = tr("x Down x");  break;
    case NORTH: dir_string = tr("^ North ^"); break;
    case SOUTH: dir_string = tr("v South v"); break;
    case EAST:  dir_string = tr("> East >");  break;
    case WEST:  dir_string = tr("< West <");  break;
    case NOWHERE: dir_string =  "Nowhere";    break;
    }
    wstandend(window);
    box(window, 0, 0);
    wcolor_set(window, BLACK_WHITE, nullptr);
    mvwaddstr(window, 0, 1, qPrintable(dir_string));
}

void Screen::PrintInv(WINDOW * const window, const Inventory & inv) const {
    werase(window);
    wstandend(window);
    const int start = inv.Start();
    int shift = 0; // to divide inventory sections
    for (int i=0; i<inv.Size(); ++i) {
        shift += ( start == i && i != 0 );
        mvwprintw(window, 2+i+shift, 1, "%c) ", 'a'+i);
        const int number = inv.Number(i);
        if ( 0 == number ) {
            continue;
        }
        const Block * const block = inv.ShowBlock(i);
        wprintw(window, "[%c]%s",
            PrintBlock(*block, window), qPrintable(inv.InvFullName(i)) );
        if ( 1 < number ) {
            waddstr(window, qPrintable(Inventory::NumStr(number)));
        }
        if ( MAX_DURABILITY != block->GetDurability() ) {
            wprintw(window, "{%d}", block->GetDurability()*100/MAX_DURABILITY);
        }
        const QString str = block->GetNote();
        if ( not str.isEmpty() ) {
            const int x = getcurx(window);
            const int width = SCREEN_SIZE*2+2 - x - 10;
            if ( str.size() < width ) {
                wprintw(window, " ~:%s", qPrintable(str));
            } else {
                wprintw(window, " ~:%s...", qPrintable(str.left(width-6)));
            }
        }
        wstandend(window);
        mvwprintw(window, 2+i+shift, 53, "%5hu mz", inv.GetInvWeight(i));
    }
    mvwprintw(window, 2+inv.Size()+shift, 40,
        qPrintable(tr("All weight: %1 mz").
            arg(inv.Weight(), 6, 10, QChar(' '))));
    wattrset(window, Color(inv.Kind(), inv.Sub()));
    box(window, 0, 0);
    if ( start != 0 ) {
        mvwhline(window, 2+start, 1, ACS_HLINE, SCREEN_SIZE*2);
    }
    mvwprintw(window, 0, 1, "[%c]%s", CharName( inv.Kind(), inv.Sub()),
        qPrintable((player->PlayerInventory()==&inv) ?
            tr("Your inventory") : inv.FullName()) );
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

void Screen::Notify(const QString str) const {
    fprintf(notifyLog, "%s %s\n",
        qPrintable(w->TimeOfDayStr()), qPrintable(str));
    if ( inputActive ) return;
    if ( beepOn ) {
        if ( str == DING ) {
            beep();
        } else if ( str == OUCH ) {
            flash();
        }
    }
    if ( str.at(str.size()-1) == '!' ) {
        wcolor_set(notifyWin, RED_BLACK, nullptr);
    } else {
        wstandend(notifyWin);
    }

    static int notification_repeat_count = 1;
    if ( str == lastNotification ) {
        ++notification_repeat_count;
        const int y = getcury(notifyWin);
        mvwprintw(notifyWin, y-1, 0, "\n%s (%dx)",
            qPrintable(str), notification_repeat_count);
    } else {
        notification_repeat_count = 1;
        waddch(notifyWin, '\n');
        waddstr(notifyWin, qPrintable(lastNotification = str));
    }
    wrefresh(notifyWin);
}

void Screen::DeathScreen() {
    werase(rightWin);
    werase(hudWin);
    Notify(DING);
    wcolor_set(leftWin, WHITE_RED, nullptr);
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

Screen::Screen(
        World  * const wor,
        Player * const pl,
        int & error,
        bool _ascii)
    :
        VirtScreen(wor, pl),
        lastNotification(),
        input(new IThread(this)),
        notifyLog(fopen("texts/messages.txt", "at")),
        actionMode(ACTION_USE),
        fileToShow(nullptr),
        beepOn(false),
        ascii(_ascii),
        screen(newterm(nullptr, stdout, stdin))
{
    #ifndef Q_OS_WIN32
        set_escdelay(10);
    #endif
    start_color();
    raw(); // send typed keys directly
    noecho(); // do not print typed symbols
    nonl();
    keypad(stdscr, TRUE); // use arrows
    memset(windows, 0, sizeof(windows));
    if ( LINES < 41 ) {
        printf("Make your terminal height to be at least 41 lines.\n");
        error = HEIGHT_NOT_ENOUGH;
        return;
    }
    // all available color pairs (maybe some of them will not be used)
    const int colors[] = { // do not change colors order!
        COLOR_BLACK,
        COLOR_RED,
        COLOR_GREEN,
        COLOR_YELLOW,
        COLOR_BLUE,
        COLOR_MAGENTA,
        COLOR_CYAN,
        COLOR_WHITE
    };
    for (int i=BLACK_BLACK; i<=WHITE_WHITE; ++i) {
        init_pair(i, colors[(i-1)/8], colors[(i-1)%8]);
    }
    const int preferred_width = (SCREEN_SIZE*2+2)*2;
    if ( COLS >= preferred_width ) {
        const int left_border = COLS/2-SCREEN_SIZE*2-2;
        rightWin  = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, COLS/2);
        leftWin   = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, left_border);
        hudWin    = newwin(3, preferred_width, SCREEN_SIZE+2, left_border);
        notifyWin = newwin(0,preferred_width-13, SCREEN_SIZE+5,left_border+33);
        miniMapWin = newwin(7, 11, SCREEN_SIZE+5, left_border  );
        actionWin  = newwin(7, 20, SCREEN_SIZE+5, left_border+12);
    } else if ( COLS >= preferred_width/2 ) {
        const int left_border = COLS/2-SCREEN_SIZE-1;
        leftWin   = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, left_border);
        hudWin    = newwin(3, SCREEN_SIZE*2+2, SCREEN_SIZE+2, left_border);
        notifyWin = newwin(21,SCREEN_SIZE*2+2, SCREEN_SIZE+2+3,left_border+20);
        actionWin = newwin(0, 20, SCREEN_SIZE+2+3, left_border);
    } else {
        puts(qPrintable(tr("Set your terminal width at least %1 chars.").
            arg(SCREEN_SIZE*2+2)));
        error = WIDTH_NOT_ENOUGH;
        return;
    }
    scrollok(notifyWin, TRUE);

    QSettings sett(QDir::currentPath()+"/freg.ini", QSettings::IniFormat);
    sett.beginGroup("screen_curses");
    shiftFocus = sett.value("focus_shift", 0).toInt();
    previousCommand = sett.value("last_command", "hello").toString();
    beepOn     = sett.value("beep_on", false).toBool();
    sett.setValue("beep_on", beepOn);

    if ( not PrintFile(stdscr, "texts/splash.txt") ) {
        addstr("Free-Roaming Elementary Game\nby mmaulwurff\n");
    }
    addstr(qPrintable(tr("\nVersion %1.\n\nPress any key.").arg(VER)));
    qsrand(getch());
    CleanFileToShow();
    RePrint();
    SetActionMode(static_cast<actions>
        (sett.value("action_mode", ACTION_USE).toInt()));
    Print();
    Notify(tr("*--- Game started. Press 'H' for help. ---*"));
    if ( COLS < preferred_width ) {
        Notify("For better gameplay ");
        Notify(QString("set your terminal width at least %1 chars.").
            arg(preferred_width));
    }

    input->start();
    connect(wor, SIGNAL(UpdatesEnded()), SLOT(Print()), Qt::DirectConnection);
} // Screen::Screen(World * wor, Player * pl)

Screen::~Screen() {
    w->Lock();
    disconnect(w, SIGNAL(UpdatesEnded()), this, SLOT(Print()));
    w->Unlock();

    input->Stop();
    input->wait();
    delete input;

    for (ulong i=0; i<sizeof(windows)/sizeof(windows[0]); ++i) {
        delwin(windows[i]);
    }
    endwin();
    delscreen(screen);
    if ( notifyLog ) {
        fclose(notifyLog);
    }
    delete fileToShow;
    QSettings sett(QDir::currentPath()+"/freg.ini", QSettings::IniFormat);
    sett.beginGroup("screen_curses");
    sett.setValue("focus_shift", shiftFocus);
    sett.setValue("action_mode", actionMode);
    sett.setValue("last_command", previousCommand);
}

bool Screen::IsScreenWide() { return COLS >= (SCREEN_SIZE*2+2)*2; }

void Screen::PrintBar(const int x, const int attr, const int ch,
        const int percent, const bool value_position_right)
{
    wstandend(hudWin);
    mvwprintw(hudWin, 0, x,
        value_position_right ? "[..........]%hd" : "%3hd[..........]",percent);
    wattrset(hudWin, attr);
    const QString str(10, QChar(ch));
    mvwaddstr(hudWin, 0, x + (not value_position_right ? 4 : 1),
        qPrintable(str.left(percent/10)));
}
