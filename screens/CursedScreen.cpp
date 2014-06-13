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

#include <QSettings>
#include <QDir>
#include <QMutex>
#include <QLocale>
#include "screens/CursedScreen.h"
#include "screens/IThread.h"
#include "world.h"
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
    SetUpdated(false);
}

void Screen::Update(int, int, int) { SetUpdated(false); }
void Screen::UpdatePlayer() { SetUpdated(false); }
void Screen::UpdateAround(int, int, int, int) { SetUpdated(false); }
void Screen::Move(int) { SetUpdated(false); }

void Screen::UpdateAll() {
    CleanFileToShow();
    SetUpdated(false);
}

void Screen::PassString(QString & str) const {
    waddch(notifyWin, ':');
    char temp_str[MAX_NOTE_LENGTH+1];
    echo();
    wgetnstr(notifyWin, temp_str, MAX_NOTE_LENGTH);
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

color_pairs Screen::Color(const int kind, const int sub) const {
    switch ( kind ) { // foreground_background
    case LIQUID: switch ( sub ) {
        case WATER: return CYAN_BLUE;
        case ACID:  return GREEN_MAGENTA;
        default:    return RED_YELLOW;
    } // no break;
    case ACTIVE: switch ( sub ) {
        case WATER: return CYAN_WHITE;
        case SAND:  return YELLOW_WHITE;
    } // no break;
    default: switch ( sub ) {
        default: return WHITE_BLACK;
        case STONE:      return BLACK_WHITE;
        case GREENERY:   return BLACK_GREEN;
        case WOOD:
        case HAZELNUT:
        case SOIL:       return BLACK_YELLOW;
        case SAND:       return YELLOW_WHITE;
        case COAL:       return BLACK_WHITE;
        case IRON:       return WHITE_BLACK;
        case A_MEAT:     return WHITE_RED;
        case H_MEAT:     return BLACK_RED;
        case WATER:      return WHITE_CYAN;
        case GLASS:      return BLUE_WHITE;
        case NULLSTONE:  return MAGENTA_BLACK;
        case MOSS_STONE: return GREEN_WHITE;
        case ROSE:       return RED_GREEN;
        case CLAY:       return WHITE_RED;
        case PAPER:      return MAGENTA_WHITE;
        case GOLD:       return WHITE_YELLOW;
        case BONE:       return MAGENTA_WHITE;
        case FIRE:       return RED_YELLOW;
        case EXPLOSIVE:  return WHITE_RED;
        case SUN_MOON:   return ( NIGHT == w->PartOfDay() ) ?
            WHITE_WHITE : YELLOW_YELLOW;
        case SKY:
        case STAR:
            if ( w->GetEvernight() ) return BLACK_BLACK;
            switch ( w->PartOfDay() ) {
            case NIGHT:   return WHITE_BLACK;
            case MORNING: return WHITE_BLUE;
            case NOON:    return CYAN_CYAN;
            case EVENING: return WHITE_CYAN;
            }
        }
    case DWARF:     return WHITE_BLUE;
    case RABBIT:    return RED_WHITE;
    case PREDATOR:  return RED_BLACK;
    case TELEGRAPH: return CYAN_BLACK;
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

void Screen::MovePlayer(const int dir) {
    if ( player->GetDir() == dir ) {
        player->Move(dir);
    } else {
        player->SetDir(dir);
        SetUpdated(false);
    }
}

void Screen::MovePlayerDiag(const int dir1, const int dir2) const {
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
    default: Notify(tr("Unknown key. Press 'H' for help.")); break;
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
    case '=': player->Move(); break;

    case '>': player->SetDir(World::TurnRight(player->GetDir())); break;
    case '<': player->SetDir(World::TurnLeft (player->GetDir())); break;
    case KEY_NPAGE: player->SetDir(DOWN); break;
    case KEY_PPAGE: player->SetDir(UP);   break;

    case 'I':
    case KEY_HOME: player->Backpack(); break;
    case 8:
    case KEY_BACKSPACE: player->Damage(); break;
    case 13:
    case '\n': player->Use();      break;
    case  '?': player->Examine();  break;
    case  '~': player->Inscribe(); break;
    case 27: /* esc */ player->StopUseAll(); break;

    case 'B': SetActionMode(ACTION_BUILD);    break;
    case 'C': SetActionMode(ACTION_CRAFT);    break;
    case 'D':
    case 'T': SetActionMode(ACTION_THROW);    break;
    case 'E': SetActionMode(ACTION_EAT);      break;
    case 'F': SetActionMode(ACTION_TAKEOFF);  break;
    case 'N': SetActionMode(ACTION_INSCRIBE); break;
    case 'G':
    case 'O': SetActionMode(ACTION_OBTAIN);   break;
    case 'U': SetActionMode(ACTION_USE);      break;
    case 'W': SetActionMode(ACTION_WIELD);    break;
    case 'S':
        if ( player->PlayerInventory() ) {
              player->PlayerInventory()->Shake();
        }
    break;
    case KEY_HELP:
    case 'H':
        DisplayFile(QString("help_%1/help.txt").arg(locale.left(2)));
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
    }
    SetUpdated(false);
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
    actionMode = mode;
    SetUpdated(false);
}

void Screen::InventoryAction(const int num) const {
    switch ( actionMode ) {
    case ACTION_USE:      player->Use     (num); break;
    case ACTION_WIELD:    player->Wield   (num); break;
    case ACTION_INSCRIBE: player->Inscribe(num); break;
    case ACTION_EAT:      player->Eat     (num); break;
    case ACTION_CRAFT:    player->Craft   (num); break;
    case ACTION_TAKEOFF:  player->TakeOff (num); break;
    case ACTION_OBTAIN:   player->Obtain  (num); break;
    case ACTION_THROW:    player->Throw   (num); break;
    case ACTION_BUILD:    player->Build   (num); break;
    default: fprintf(stderr,
        "Screen::InventoryAction: action mode ?: %d\n", actionMode);
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
    wcolor_set(window, Color(kind, sub), nullptr);
    return CharName(kind, sub);
}

void Screen::SetUpdated(const bool upd) {
    static QMutex mutex;
    mutex.lock();
    updated = upd;
    mutex.unlock();
}

void Screen::Print() {
    if ( not player->IfPlayerExists() || updated ) return;
    SetUpdated(true);
    w->Lock();
    PrintHUD();
    const int dir = player->GetDir();
    switch ( player->UsingSelfType() ) { // left window
    default:
        PrintNormal(leftWin, (UP==dir || DOWN==dir) ?
            NORTH : dir);
        break;
    case USAGE_TYPE_OPEN:
        if ( player->PlayerInventory() ) {
            PrintInv(leftWin, *player->PlayerInventory());
            break;
        } // no break;
    }
    if ( not fileToShow ) { // right window
        switch ( player->UsingType() ) {
        default:
            if ( UP==dir || DOWN==dir ) {
                PrintNormal(rightWin, dir);
            } else {
                PrintFront(rightWin);
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
            const Inventory * const inv = w->GetBlock(x, y, z)->HasInventory();
            if ( inv ) {
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
    int y_save, x_save;
    getyx(rightWin, y_save, x_save);

    werase(hudWin);
    // quick inventory
    Inventory * const inv = player->PlayerInventory();
    if ( inv && IsScreenWide() ) {
        for (int i=inv->Size()-1; i>=0; --i) {
            wstandend(hudWin);
            const int x = QUICK_INVENTORY_X_SHIFT+i*2;
            mvwaddch(hudWin, 0, x, 'a'+i);
            const int number = inv->Number(i);
            if ( number ) {
                mvwaddch(hudWin, 1, x, PrintBlock(*inv->ShowBlock(i), hudWin));
                if ( number > 1 ) {
                    mvwaddch(hudWin, 2, x, number+'0');
                }
            }
        }
    }
    // focused block
    wstandend(hudWin);
    int x, y, z;
    ActionXyz(&x, &y, &z);
    Block * const focused = GetWorld()->GetBlock(x, y, z);
    if ( not IsLikeAir(focused->Sub()) && z < HEIGHT-1 ) {
        PrintBar(((SCREEN_SIZE*2+2) * (IsScreenWide() ? 2 : 1)) - 15,
            Color(focused->Kind(), focused->Sub()),
            (focused->IsAnimal() == nullptr) ? '+' : '*',
            focused->GetDurability(),
            MAX_DURABILITY,
            false);
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
        const int dur = player->HP();
        if ( dur > 0 ) { // HitPoints line
            PrintBar(0, (dur > MAX_DURABILITY/5) ? RED_BLACK : BLACK_RED,
                ascii ? '@' : 0x2665, dur, MAX_DURABILITY);
        }
        const int breath = player->Breath();
        if ( -1!=breath && breath!=MAX_BREATH ) { // breath line
            PrintBar(16, BLUE_BLACK, ascii ? 'o' : 0x00b0, breath, MAX_BREATH);
        }
        const int satiation = player->SatiationPercent();
        if ( -1 != satiation ) { // satiation line
            if ( 100 < satiation ) {
                wcolor_set(hudWin, BLUE_BLACK, nullptr);
                mvwaddstr(hudWin, 2, 0, qPrintable(tr("Gorged")));
            } else if ( 75 < satiation ) {
                wcolor_set(hudWin, GREEN_BLACK, nullptr);
                mvwaddstr(hudWin, 2, 0, qPrintable(tr("Full")));
            } else if ( 25 > satiation ) {
                wcolor_set(hudWin, RED_BLACK, nullptr);
                mvwaddstr(hudWin, 2, 0, qPrintable(tr("Hungry")));
            }
        }
    }
    wrefresh(hudWin);
    wmove(miniMapWin, 1, 1);
    const int x_center = Shred::CoordOfShred(player->X());
    const int y_center = Shred::CoordOfShred(player->Y());
    const int j_start = qMax(x_center-2, 0);
    const int j_end = qMin(x_center+2, world->NumShreds()-1);
    const int i_end = qMin(y_center+2, world->NumShreds()-1);
    for (int i=qMax(y_center-2, 0); i<=i_end; ++i, waddstr(miniMapWin, "\n_"))
    for (int j=j_start;             j<=j_end; ++j) {
        Shred * const shred = world->GetShredByPos(j, i);
        if ( shred == nullptr ) {
            wstandend(miniMapWin);
            waddstr(miniMapWin, "  ");
        } else {
            wcolor_set(miniMapWin,ColorShred(shred->GetTypeOfShred()),nullptr);
            wprintw(miniMapWin, "%c ", shred->GetTypeOfShred());
        }
    }
    wstandend(miniMapWin);
    box(miniMapWin, 0, 0);
    wrefresh(miniMapWin);
    (void)wmove(rightWin, y_save, x_save);
} // void Screen::PrintHUD()

void Screen::PrintNormal(WINDOW * const window, const int dir) const {
    const int k_start = ( UP!=dir ) ?
        ( DOWN==dir ?
            player->Z()-1 : player->Z() ) :
        player->Z()+1;
    const int k_step = ( UP!=dir ) ? (-1) : 1;

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
        if ( (w->Enlightened(i, j, k) && player->Visible(i, j, k)) ||
                player->GetCreativeMode() )
        {
            waddch(window, PrintBlock(*shred->GetBlock(i_in,j_in,k), window));
            waddch(window, CharNumber(k));
        } else {
            wstandend(window);
            waddch(window, OBSCURE_BLOCK);
            waddch(window, ' ');
        }
    }
    if ( player->IfPlayerExists() && dir > DOWN ) {
        const Block * const block =
            w->GetBlock(player->X(), player->Y(), player->Z());
        static const QString arrow_left (QChar(ascii ? '<' : 0x2190));
        static const QString arrow_up   (QChar(ascii ? '^' : 0x2191));
        static const QString arrow_right(QChar(ascii ? '>' : 0x2192));
        static const QString arrow_down (QChar(ascii ? 'v' : 0x2193));
        wcolor_set(window, Color(block->Kind(), block->Sub()), nullptr);
        (void)wmove(window, player->Y()-start_y+1, (player->X()-start_x)*2+2);
        switch ( player->GetDir() ) {
        case NORTH: waddstr(window, qPrintable(arrow_up));    break;
        case SOUTH: waddstr(window, qPrintable(arrow_down));  break;
        case EAST:  waddstr(window, qPrintable(arrow_right)); break;
        case WEST:  waddstr(window, qPrintable(arrow_left));  break;
        case UP:    waddch(window, '.'); break;
        case DOWN:  waddch(window, 'x'); break;
        default:    waddch(window, '*');
            fprintf(stderr, "Screen::PrintNormal: (?) dir: %d\n",
                player->GetDir());
        }
    }
    PrintTitle(window, UP==dir ? UP : DOWN);
    Arrows(window, (player->X()-start_x)*2+1, player->Y()-start_y+1, true);
    wrefresh(window);
} // void Screen::PrintNormal(WINDOW * window, int dir)

void Screen::PrintFront(WINDOW * const window) const {
    if ( window == nullptr ) return;
    const int dir = player->GetDir();
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
        fprintf(stderr, "Screen::PrintFront(): unlisted dir: %d\n", (int)dir);
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
            const Block * const block = w->GetBlock(i, j, k);
            if ( (*z==z_end || (w->Enlightened(i, j, k)
                    && player->Visible(i, j, k)))
                        || player->GetCreativeMode() )
            {
                if ( *z != z_end ) {
                    waddch(window, PrintBlock(*block, window));
                    waddch(window, CharNumberFront(i, j));
                    continue;
                } else {
                    wcolor_set(window, sky_colour, nullptr);
                    waddch(window, sky_char);
                }
            } else {
                wstandend(window);
                waddch(window, OBSCURE_BLOCK);
            }
            waddch(window, ' ');
        }
    }
    PrintTitle(window, dir);
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

void Screen::PrintTitle(WINDOW * const window, const int dir) const {
    wstandend(window);
    box(window, 0, 0);
    wcolor_set(window, BLACK_WHITE, nullptr);
    QString dir_string;
    switch ( dir ) {
    case NORTH: dir_string = tr("^ North ^"); break;
    case SOUTH: dir_string = tr("v South v"); break;
    case EAST:  dir_string = tr("> East >");  break;
    case WEST:  dir_string = tr("< West <");  break;
    case DOWN:  dir_string = tr("x Down x");  break;
    case UP:    dir_string = tr(".  Up  .");  break;
    }
    mvwaddstr(window, 0, 1, qPrintable(dir_string));
}

void Screen::PrintInv(WINDOW * const window, const Inventory & inv) const {
    werase(window);
    wstandend(window);
    switch ( inv.Kind() ) {
    case DWARF:
        mvwaddstr(window, 2, 1, qPrintable(tr("      Head")));
        mvwaddstr(window, 3, 1, qPrintable(tr("Right hand")));
        mvwaddstr(window, 4, 1, qPrintable(tr(" Left hand")));
        mvwaddstr(window, 5, 1, qPrintable(tr("      Body")));
        mvwaddstr(window, 6, 1, qPrintable(tr("      Legs")));
    break;
    case WORKBENCH: mvwaddstr(window, 2, 4, qPrintable(tr("Product"))); break;
    }
    const int start = inv.Start();
    int shift = 0; // to divide inventory sections
    for (int i=0; i<inv.Size(); ++i) {
        if ( start == i && i != 0) {
            ++shift;
            mvwhline(window, 2+i, 0, ACS_HLINE, SCREEN_SIZE*2+2);
        }
        mvwprintw(window, 2+i+shift, 12, "%c)", 'a'+i);
        if ( not inv.Number(i) ) {
            continue;
        }
        const Block * const block = inv.ShowBlock(i);
        wprintw(window, "[%c]%s",
            PrintBlock(*block, window),
            qPrintable(inv.InvFullName(i)) );
        if ( 1 < inv.Number(i) ) {
            waddstr(window, qPrintable(inv.NumStr(i)));
        }
        const QString str = inv.GetInvNote(i);
        if ( not str.isEmpty() ) {
            if ( str.size() < 24 ) {
                wprintw(window, " ~:%s", qPrintable(str));
            } else {
                wprintw(window, " ~:%s...", qPrintable(str.left(13)));
            }
        }
        wstandend(window);
        mvwprintw(window, 2+i+shift, 53, "%5hu mz", inv.GetInvWeight(i));
    }
    mvwprintw(window, 2+inv.Size()+shift, 40,
        qPrintable(tr("All weight: %1 mz").
            arg(inv.Weight(), 6, 10, QChar(' '))));
    wcolor_set(window, Color(inv.Kind(), inv.Sub()), nullptr);
    box(window, 0, 0);
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
    if ( beepOn ) {
        if ( str == DING ) {
            beep();
        } else if ( str == OUCH ) {
            flash();
        }
    }
    if ( str.at(str.size()-1) == '!' ) {
        wcolor_set(notifyWin, RED_BLACK, nullptr);
    }
    static int notification_repeat_count = 1;
    if ( str == lastNotification ) {
        ++notification_repeat_count;
        int x, y;
        getyx(notifyWin, y, x);
        mvwprintw(notifyWin, y-1, 0, "%s (%dx)\n",
            qPrintable(str), notification_repeat_count);
    } else {
        notification_repeat_count = 1;
        wprintw(notifyWin, "%s\n", qPrintable(str));
        lastNotification = str;
    }
    wstandend(notifyWin);
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
    SetUpdated(true);
}

Screen::Screen(
        World  * const wor,
        Player * const pl,
        int & error,
        bool _ascii)
    :
        VirtScreen(wor, pl),
        leftWin(nullptr),
        rightWin(nullptr),
        notifyWin(nullptr),
        hudWin(nullptr),
        miniMapWin(nullptr),
        lastNotification(),
        input(new IThread(this)),
        updated(false),
        notifyLog(fopen("texts/messages.txt", "at")),
        fileToShow(nullptr),
        beepOn(false),
        ascii(_ascii)
{
    #ifndef Q_OS_WIN32
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
        rightWin = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, COLS/2);
        leftWin  = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, left_border);
        hudWin   = newwin(3, preferred_width, SCREEN_SIZE+2, left_border);
        notifyWin = newwin(0,preferred_width-13, SCREEN_SIZE+5,left_border+13);
        miniMapWin = newwin(7, 12, SCREEN_SIZE+5, left_border);
    } else if ( COLS >= preferred_width/2 ) {
        const int left_border = COLS/2-SCREEN_SIZE-1;
        rightWin   = nullptr;
        miniMapWin = nullptr;
        leftWin  = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, left_border);
        hudWin   = newwin(3, SCREEN_SIZE*2+2, SCREEN_SIZE+2, left_border);
        notifyWin  = newwin(0, SCREEN_SIZE*2+2, SCREEN_SIZE+2+3, left_border);
    } else {
        world->CleanAll();
        CleanAll();
        puts(qPrintable(tr("Set your terminal width at least %1 chars.").
            arg(SCREEN_SIZE*2+2)));
        error = WIDTH_NOT_ENOUGH;
        return;
    }
    scrollok(notifyWin, TRUE);

    QSettings sett(QDir::currentPath()+"/freg.ini", QSettings::IniFormat);
    sett.beginGroup("screen_curses");
    shiftFocus = sett.value("focus_shift", 0).toInt();
    actionMode = static_cast<actions>
        (sett.value("action_mode", ACTION_USE).toInt());
    command    = sett.value("last_command", "hello").toString();
    beepOn     = sett.value("beep_on", false).toBool();
    sett.setValue("beep_on", beepOn);

    if ( not PrintFile(stdscr, "texts/splash.txt") ) {
        addstr("Free-Roaming Elementary Game\nby mmaulwurff\n");
    }
    addstr(qPrintable(tr("\nVersion %1.\n\nPress any key.").arg(VER)));
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
    connect(wor, SIGNAL(UpdatesEnded()), SLOT(Print()));
} // Screen::Screen(World * wor, Player * pl)

void Screen::CleanAll() {
    static bool cleaned = false;
    if ( cleaned ) return;
    cleaned = true; // prevent double cleaning
    input->Stop();
    input->wait();
    delete input;

    if ( leftWin   ) delwin(leftWin);
    if ( rightWin  ) delwin(rightWin);
    if ( notifyWin ) delwin(notifyWin);
    if ( hudWin    ) delwin(hudWin);
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

bool Screen::IsScreenWide() { return COLS >= (SCREEN_SIZE*2+2)*2; }

void Screen::PrintBar(const int x, const int color, const int ch,
        const int value, const int max_value, const bool value_position_right)
{
    wstandend(hudWin);
    mvwprintw(hudWin, 0, x,
        value_position_right ? "[..........]%hd" : "%3hd[..........]", value);
    wcolor_set(hudWin, color, nullptr);
    const QString str(10, QChar(ch));
    mvwaddstr(hudWin, 0, x + (not value_position_right ? 4 : 1),
        qPrintable(str.left(10*value/max_value+1)));
}
