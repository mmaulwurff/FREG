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

#include "screens/CursedScreen.h"
#include "screens/IThread.h"
#include "Shred.h"
#include "World.h"
#include "Player.h"
#include "TranslationsManager.h"
#include "blocks/Block.h"
#include "blocks/Inventory.h"
#include <QDir>
#include <QLocale>
#include <QMutexLocker>

const char OBSCURE_BLOCK = ' ';
const int SHADOW_COLOR = COLOR_PAIR(BLACK_BLACK) | A_BOLD | A_REVERSE;

const int ACTIONS_WIDTH  = 23;
const int MINIMAP_WIDTH  = 11;
const int MINIMAP_HEIGHT =  7;

const int MOUSEMASK = BUTTON1_CLICKED | BUTTON1_RELEASED;

const int ACTIVE_HAND = 3;

const int AVERAGE_SCREEN_SIZE = 60;

void Screen::PrintVerticalDirection(WINDOW * const window, const int y,
        const int x, const dirs direction)
{
    mvwaddwstr(window, y, x + 3,
        tr_manager->DirString(direction).toStdWString().c_str());
}

void Screen::Arrows(WINDOW * const window, const int x, const int y,
        const dirs direction)
const {
    wcolor_set(window, WHITE_BLACK, nullptr);
    if ( direction >= DOWN ) {
        PrintVerticalDirection(window, 0, x, UP);
        PrintVerticalDirection(window, screenHeight-1, x, DOWN);
    } else {
        PrintVerticalDirection(window, 0, x, NORTH);
        PrintVerticalDirection(window, screenHeight-1, x, SOUTH);
    }
    wcolor_set(window, WHITE_RED, nullptr);
    mvwaddwstr(window, 0, x, arrows[SOUTH]);
    waddwstr  (window,       arrows[SOUTH]);
    mvwaddwstr(window, screenHeight-1, x, arrows[NORTH]);
    waddwstr  (window, arrows[NORTH]);
    HorizontalArrows(window, y, direction);
    (void)wmove(window, y, x);
}

void Screen::HorizontalArrows(WINDOW * const window, const int y,
        const dirs direction)
const {
    wcolor_set(window, WHITE_BLACK, nullptr);
    const static QString dir_chars[] = {
        QString(),
        QString(),
        tr_manager->DirString(NORTH).left(1),
        tr_manager->DirString(SOUTH).left(1),
        tr_manager->DirString(EAST ).left(1),
        tr_manager->DirString(WEST ).left(1)
    };
    QString left, right;
    switch ( direction ) {
    case UP:
    case DOWN:
    case NORTH: left = dir_chars[WEST];  right = dir_chars[EAST]; break;
    case SOUTH: left = dir_chars[EAST];  right = dir_chars[WEST]; break;
    case EAST:  left = dir_chars[NORTH]; right = dir_chars[SOUTH]; break;
    case WEST:  left = dir_chars[SOUTH]; right = dir_chars[NORTH]; break;
    }
    mvwaddwstr(window, y-1, 0, left.toStdWString().c_str());
    mvwaddwstr(window, y-1, screenWidth-1, right.toStdWString().c_str());

    wcolor_set(window, WHITE_RED, nullptr);
    mvwaddwstr(window, y,             0, arrows[EAST]);
    mvwaddwstr(window, y, screenWidth-1, arrows[WEST]);
}

void Screen::RePrint() {
    clear();
    static const QString action_strings[] = {
        tr("[U] use, eat"),
        tr("[T] throw"),
        tr("[G] get"),
        tr("[N] inscribe"),
        tr("[B] build"),
        tr("[C] craft"),
        tr("[E] equipment"),
    };
    (void)wmove(actionWin, 0, 0);
    for (int i=0; i<=ACTION_WIELD; ++i) {
        waddwstr(actionWin, action_strings[i].toStdWString().c_str());
        waddch(actionWin, '\n');
    }
    mvwchgat(actionWin, actionMode, 0, -1, A_NORMAL, BLACK_WHITE, nullptr);
    refresh();
    wrefresh(actionWin);
    updated = false;
}

void Screen::Update(int, int, int) { updated = false; }
void Screen::UpdatePlayer() { updated = false; }
void Screen::UpdateAround(int, int, int, int) { updated = false; }
void Screen::Move(int) { updated = false; }
bool Screen::IsScreenWide() { return COLS >= (AVERAGE_SCREEN_SIZE + 2) * 2; }

void Screen::UpdateAll() {
    CleanFileToShow();
    updated = false;
}

void Screen::PassString(QString & str) const {
    inputActive = true;
    wattrset(notifyWin, A_UNDERLINE);
    waddwstr(notifyWin, tr("Enter input: ").toStdWString().c_str());
    char temp_str[MAX_NOTE_LENGTH + 1];
    echo();
    wgetnstr(notifyWin, temp_str, MAX_NOTE_LENGTH);
    inputActive = false;
    noecho();
    lastNotification = str;
    fprintf(notifyLog, "%llu: Command: %s\n", world->Time(), temp_str);
    str = QString::fromUtf8(temp_str);
}

char Screen::CharNumber(int z) const {
    if ( HEIGHT-1 == z ) { // sky
        return ' ';
    }
    z = ( UP == player->GetDir() ) ?
        z - player->Z() : player->Z() - z;
    return ( z == 0 ) ?
        ' ' :
        ( z < 0 ) ?
            '-' :
            ( z < 10 ) ?
                z + '0' :
                ( farDistance && z <= 0xf ) ?
                    z - 10 + 'a' :
                    '+';
}

char Screen::CharNumberFront(int i, const int j) const {
    i = ( ( player->GetDir() > SOUTH ) ? // east or west
        abs(player->X() - i) :
        abs(player->Y() - j) ) - 1;
    return ( i == 0 ) ?
        ' ' :
        ( i < 10 ) ?
            i + '0' :
            (farDistance && i <= 0xf) ?
                i - 10 + 'a' :
                '+';
}

int  Screen::RandomBlink() { return (RandomBit() * A_REVERSE); }
bool Screen::RandomBit()   { return (qrand() & 1); }

int Screen::Color(const int kind, const int sub) {
    const int color = COLOR_PAIR(VirtScreen::Color(kind, sub));
    switch ( kind ) { // foreground_background
    case TELEGRAPH: return color | A_BOLD;
    case TELEPORT:  return color | (RandomBit() ? A_BOLD : 0);
    case LIQUID: switch ( sub ) {
        case H_MEAT:
        case A_MEAT:
        case SUB_CLOUD: return color;
        default:        return color | RandomBlink();
        case ACID:      return color | RandomBlink() | A_BOLD;
        case WATER: return RandomBit() ? color : COLOR_PAIR(BLUE_BLUE)|A_BOLD;
        } break;
    default: switch ( sub ) {
        case GOLD:       return color | RandomBlink();
        case IRON:
        case NULLSTONE:
        case DIAMOND:    return color | A_BOLD;
        case ACID:
        case SUB_DUST:   return color | A_BOLD  | A_REVERSE;
        case FIRE:       return color | A_BLINK | RandomBlink();
        case SKY:
        case STAR:
            if ( world->GetEvernight() ) return COLOR_PAIR(BLACK_BLACK);
            switch ( world->PartOfDay() ) {
            case TIME_NIGHT: return
                COLOR_PAIR(WHITE_BLACK) | ( RandomBit() ? A_BOLD : 0 );
            case TIME_MORNING: return COLOR_PAIR(WHITE_BLUE);
            case TIME_NOON:    return COLOR_PAIR( CYAN_CYAN);
            case TIME_EVENING: return COLOR_PAIR(WHITE_CYAN);
            } break;
        default: return color;
        } break;
    }
} // color_pairs Screen::Color(int kind, int sub)

int Screen::ColorShred(const shred_type type) const {
    switch ( type ) { // foreground_background
    case SHRED_NORMAL_UNDERGROUND:
    case SHRED_TESTSHRED:
    case SHRED_EMPTY:
    case SHRED_CHAOS:       return COLOR_PAIR( WHITE_BLACK);
    case SHRED_DEAD_FOREST: return COLOR_PAIR(YELLOW_BLACK);
    case SHRED_DEAD_HILL:   return COLOR_PAIR( BLACK_WHITE) | A_BOLD;
    case SHRED_PYRAMID:
    case SHRED_WASTE:
    case SHRED_CASTLE:
    case SHRED_MOUNTAIN:  return Color(BLOCK,  STONE);
    case SHRED_PLAIN:     return Color(BLOCK,  GREENERY);
    case SHRED_DESERT:    return Color(BLOCK,  SAND);
    case SHRED_WATER:     return Color(LIQUID, WATER);
    case SHRED_LAVA_LAKE: return Color(LIQUID, STONE);
    case SHRED_ACID_LAKE: return Color(LIQUID, ACID);
    case SHRED_CRATER:    return COLOR_PAIR( WHITE_WHITE) | A_BOLD;
    case SHRED_FOREST:    return COLOR_PAIR(YELLOW_GREEN);
    case SHRED_HILL:      return COLOR_PAIR( WHITE_GREEN);
    case SHRED_NULLMOUNTAIN: return Color(BLOCK, NULLSTONE);
    }
    Q_UNREACHABLE();
    return COLOR_PAIR(WHITE_BLACK);
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

void Screen::ControlPlayer() { ControlPlayer(getch()); }

void Screen::ControlPlayer(const int ch) {
    if ( player->GetBlock() == nullptr ) return;
    if ( ch != KEY_MOUSE ) {
        CleanFileToShow();
    }
    /// \todo: ctrl-z (to background) support
    // Q, ctrl-c, ctrl-d, ctrl-q, ctrl-x
    if ( 'Q' == ch
            || 3 == ch
            || 4 == ch
            || 17 == ch
            || 24 == ch
            || 'X' == ch
            || KEY_F(10) == ch )
    {
        Notify(tr("Exiting game..."));
        emit ExitReceived();
        input->Stop();
        return;
    } // else:
    if ( 'a'<=ch && ch<='z' ) { // actions with inventory
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
    case 'W': case KEY_UP:    case '8': MovePlayer(NORTH);  break;
    case 'S': case KEY_DOWN:  case '2': MovePlayer(SOUTH);  break;
    case 'D': case KEY_RIGHT: case '6': MovePlayer(EAST);   break;
    case 'A': case KEY_LEFT:  case '4': MovePlayer(WEST);   break;
    case KEY_END:   case '5': player->Move(DOWN); break;
    case '7': MovePlayerDiag(NORTH, WEST); break;
    case '9': MovePlayerDiag(NORTH, EAST); break;
    case '1': MovePlayerDiag(SOUTH, WEST); break;
    case '3': MovePlayerDiag(SOUTH, EAST); break;
    case '=':
    case '0': MovePlayer(player->GetDir()); break;
    case ' ': player->Jump(); break;
    case '{': player->Move(World::TurnLeft (player->GetDir())); break;
    case '}': player->Move(World::TurnRight(player->GetDir())); break;

    case '>': player->SetDir(World::TurnRight(player->GetDir())); break;
    case '<': player->SetDir(World::TurnLeft (player->GetDir())); break;
    case 'V':
    case KEY_NPAGE: player->SetDir(DOWN); break;
    case '^':
    case KEY_PPAGE: player->SetDir(UP);   break;

    case 'I':
    case KEY_HOME: player->Backpack(); break;
    case 8:
    case KEY_F(8):
    case KEY_DC: // delete
    case KEY_BACKSPACE: player->Damage(); break;

    case 13:
    case '\n': switch ( actionMode ) {
        case ACTION_USE:      player->Use();              break;
        case ACTION_THROW:    player->Throw(ACTIVE_HAND); break;
        case ACTION_OBTAIN:   player->Obtain(0);          break;
        case ACTION_INSCRIBE: player->Inscribe();         break;
        case ACTION_BUILD:    player->Build(ACTIVE_HAND); break;
        case ACTION_CRAFT:    player->Craft(ACTIVE_HAND); break;
        case ACTION_WIELD:    player->Wield(ACTIVE_HAND); break;
        }
        break;
    case 'F': case KEY_F(2): player->Use();      break;
    case '?': case KEY_F(3): player->Examine();  break;
    case '~': case KEY_F(4): player->Inscribe(); break;
    case 27: /* esc */ player->StopUseAll(); break;

    case KEY_IC: player->Build(ACTIVE_HAND); break; // insert
    case 'B': SetActionMode(ACTION_BUILD);    break;
    case 'C': SetActionMode(ACTION_CRAFT);    break;
    case 'T': SetActionMode(ACTION_THROW);    break;
    case 'N': SetActionMode(ACTION_INSCRIBE); break;
    case 'G':
    case 'O': SetActionMode(ACTION_OBTAIN);   break;
    case 'E': SetActionMode(ACTION_WIELD);    break;
    case 'U': SetActionMode(ACTION_USE);      break;
    case '[':
        SetActionMode((actionMode == ACTION_USE) ?
            ACTION_WIELD : static_cast<actions>(actionMode-1));
        break;
    case ']':
    case '\t':
        SetActionMode(actionMode == ACTION_WIELD ?
            ACTION_USE : static_cast<actions>(actionMode+1));
        break;
    case 'Z':
        if ( player->PlayerInventory() ) {
              player->PlayerInventory()->Shake();
              Notify(tr("Inventory reorganized."));
        }
        break;
    case KEY_HELP:
    case KEY_F(1):
    case 'H': ProcessCommand("help"); break;
    case KEY_F(5):
    case 'R':
    case 'L': RePrint(); break;

    case '-': shiftFocus -= 2; // no break;
    case '+': {
        ++shiftFocus;
        if ( abs(shiftFocus) == 2 ) {
            shiftFocus = 0;
        }
        static const QString levels[] = {
            tr("Low"),
            tr("Normal"),
            tr("Hight")
        };
        Notify(tr("%1 focus is set.").arg(levels[shiftFocus+1]));
        } break;

    case KEY_F(12):
    case '!': player->SetCreativeMode( not player->GetCreativeMode() ); break;
    case KEY_F(9):
    case '\\':
    case ':':
    case '/': PassString(previousCommand); // no break
    case '.': ProcessCommand(previousCommand); break;

    case 'M':
        mouseOn = not mouseOn;
        mousemask((mouseOn ? MOUSEMASK : noMouseMask), nullptr);
        Notify(tr("Mouse %1.").arg(mouseOn ?
            tr("turned on") :
            tr("turned off")));
        break;

    case KEY_MOUSE: ProcessMouse(); break;
    }
    updated = false;
} // void Screen::ControlPlayer(int ch)

void Screen::ExamineOnNormalScreen(int x, int y, int z, const int step) const {
    World * const world = GetWorld();
    x = (x-1)/2 + GetNormalStartX();
    y =  y-1    + GetNormalStartY();
    for ( ; world->GetBlock(x, y, z)->Transparent() == INVISIBLE; z += step);
    player->Examine(x, y, z);
}

void Screen::ProcessMouse() {
    MEVENT mevent;
    if ( getmouse(&mevent) == ERR ) return;
    int window_index = WINDOW_LEFT;
    for (; window_index < WINDOW_COUNT; ++window_index ) {
        if ( wenclose(windows[window_index], mevent.y, mevent.x) ) break;
    }
    switch ( window_index ) {
    case WINDOW_LEFT:
        if ( not wmouse_trafo(leftWin, &mevent.y, &mevent.x, false) ) return;
        if ( player->UsingSelfType() == USAGE_TYPE_OPEN ) {
            Notify(tr("Your inventory."));
            return;
        }
        if ( not (
                0 < mevent.x && mevent.x < screenWidth  - 1 &&
                0 < mevent.y && mevent.y < screenHeight - 1 ) )
        {
            Notify(tr("Left window, Down view."));
            return;
        }
        ExamineOnNormalScreen(mevent.x, mevent.y, player->Z(), -1);
        break;
    case WINDOW_RIGHT:
        if ( not wmouse_trafo(rightWin, &mevent.y, &mevent.x, false) ) return;
        if ( fileToShow != nullptr ) {
            Notify(tr("Reading file: \"%1\".").arg(fileToShow->fileName()));
        } else if (player->UsingType() == USAGE_TYPE_OPEN ) {
            Notify(tr("Opened inventory."));
        } else if ( not (
                0 < mevent.x && mevent.x < screenWidth  - 1 &&
                0 < mevent.y && mevent.y < screenHeight - 1 ) )
        {
            Notify(tr("Right window, %1 view.").
                arg(tr_manager->DirString(player->GetDir())));
        } else {
            switch ( player->GetDir() ) {
            case UP:
                ExamineOnNormalScreen(mevent.x, mevent.y, player->Z()+1, 1);
                break;
            case DOWN:
                ExamineOnNormalScreen(mevent.x, mevent.y, player->Z()-1, -1);
                break;
            default:
                PrintFront(player->GetDir(), mevent.x, mevent.y);
                break;
            }
        }
        break;
    case WINDOW_NOTIFY:
        Notify(tr("Notifications area."));
        break;
    case WINDOW_HUD:
        if ( not wmouse_trafo(hudWin, &mevent.y, &mevent.x, false) ) return;
        mevent.x -= getmaxx(hudWin)/2 - 'z' + 'a';
        mevent.x /= 2;
        if ( not ( IsScreenWide() && 0 <= mevent.x && mevent.x <= 'z'-'a' ) ) {
            Notify(tr("Information: left - player, right - focused thing."));
            return;
        } else {
            Inventory * const inv = player->PlayerInventory();
            if ( inv == nullptr ) return;
            Notify( tr("In inventory at slot '%1': %2.").
                arg(char(mevent.x + 'a')).
                arg( inv->Number(mevent.x) ?
                    inv->InvFullName(mevent.x) :
                    tr("nothing") ) );
        }
        break;
    case WINDOW_MINIMAP:
        if (not wmouse_trafo(minimapWin, &mevent.y, &mevent.x, false)) return;
        if ( not (
                0 < mevent.x && mevent.x < MINIMAP_WIDTH-1 &&
                0 < mevent.y && mevent.y < MINIMAP_HEIGHT-1 ) )
        {
            Notify(tr("Minimap."));
            return;
        } else {
            const int shred_x = mevent.x/2 + GetMinimapStartX();
            const int shred_y = mevent.y-1 + GetMinimapStartY();
            Notify((0 <= shred_x && shred_x < world->NumShreds() &&
                    0 <= shred_y && shred_y < world->NumShreds() ) ?
                tr("On minimap: %1.").arg( Shred::ShredTypeName(
                    world->GetShredByPos(shred_x, shred_y)->GetTypeOfShred())):
                tr("You can't see that far.") );
        }
        break;
    case WINDOW_ACTION:
        if ( not wmouse_trafo(actionWin, &mevent.y, &mevent.x, false) ) return;
        SetActionMode(static_cast<actions>(mevent.y));
        break;
    default:
        Notify(tr("Nothing here. Click on something to get information."));
        break;
    }
} // Screen::ProcessMouse()

void Screen::ProcessCommand(const QString command) {
    if ( command.length()==1 && command.at(0)!='.' ) {
        ControlPlayer(command.at(0).toLatin1());
        return;
    }
    if ( VirtScreen::ProcessCommand(command) ) return;
    switch ( Player::UniqueIntFromString(qPrintable(command)) ) {
    case Player::UniqueIntFromString("distance"):
        showDistance = not showDistance;
        Notify(tr("Show distance: %1.").
            arg(showDistance ? tr("on") : tr("off")));
        break;
    case Player::UniqueIntFromString("far"):
        farDistance = not farDistance;
        Notify(tr("Use \"abcdef\" as distance: %1.").
            arg(farDistance ? tr("on") : tr("off")));
        break;
    case Player::UniqueIntFromString("blink"):
        blinkOn = not blinkOn;
        Notify(tr("Block blink is now %1.").
            arg(blinkOn ? tr("on") : tr("off")));
        break;
    case Player::UniqueIntFromString("size"):
        Notify(tr("Terminal height: %1 lines, width: %2 chars.").
            arg(LINES).arg(COLS));
        break;
    default: player->ProcessCommand(command); break;
    }
}

void Screen::SetActionMode(const actions mode) {
    mvwchgat(actionWin, actionMode,     0, -1, A_NORMAL, WHITE_BLACK, nullptr);
    mvwchgat(actionWin, actionMode=mode,0, -1, A_NORMAL, BLACK_WHITE, nullptr);
    switch ( mode ) {
    case ACTION_USE:      Notify(tr("Action: use in inventory."));      break;
    case ACTION_THROW:    Notify(tr("Action: throw from inventory."));  break;
    case ACTION_OBTAIN:   Notify(tr("Action: get to inventory."));      break;
    case ACTION_INSCRIBE: Notify(tr("Action: inscribe in inventory.")); break;
    case ACTION_BUILD:    Notify(tr("Action: build from inventory."));  break;
    case ACTION_CRAFT:    Notify(tr("Action: craft in inventory."));    break;
    case ACTION_WIELD:    Notify(tr("Action: organize equipment."));    break;
    }
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
    case ACTION_WIELD:    player->Wield   (num); break;
    }
}

void Screen::ActionXyz(int * x, int * y, int * z) const {
    VirtScreen::ActionXyz(x, y, z);
    if (
            DOWN != player->GetDir() &&
            UP   != player->GetDir() &&
            ( AIR==world->GetBlock(*x, *y, *z)->Sub() || AIR==world->GetBlock(
                player->X(),
                player->Y(),
                player->Z()+shiftFocus)->Sub() ))
    {
        *z += shiftFocus;
    }
}

Block * Screen::GetFocusedBlock() const {
    int x, y, z;
    ActionXyz(&x, &y, &z);
    return ( player->Visible(x, y, z) ) ?
        GetWorld()->GetBlock(x, y, z) :
        nullptr;
}

void Screen::PrintBlock(const Block* const block, WINDOW* const window,
        const char second)
{
    const int kind = block->Kind();
    const int sub  = block->Sub();
    const int color = Color(kind, sub);
    waddch(window, color | CharName(kind, sub));
    waddch(window, color | second);
}

int Screen::ColoredChar(const Block * const block) const {
    const int kind = block->Kind();
    const int sub  = block->Sub();
    return CharName(kind, sub) | Color(kind, sub);
}

void Screen::Print() {
    if ( updated ) return;
    updated = true;
    world->Lock();
    PrintHUD();
    const dirs dir = player->GetDir();
    if ( player->UsingSelfType() != USAGE_TYPE_OPEN ) { // left window
        PrintNormal(leftWin, (UP==dir || DOWN==dir) ? NORTH : dir);
    } else {
        PrintInv(leftWin, player->GetBlock(), player->PlayerInventory());
    }
    if ( fileToShow == nullptr ) { // right window
        switch ( player->UsingType() ) {
        default:
            if ( dir <= DOWN ) {
                PrintNormal(rightWin, dir);
            } else if ( rightWin != nullptr ) {
                PrintFront(dir);
            }
            break;
        case USAGE_TYPE_READ_IN_INVENTORY:
            DisplayFile(QString(home_path + world->WorldName() + "/texts/"
                + player->PlayerInventory()->ShowBlock(
                    player->GetUsingInInventory())->GetNote()));
            player->SetUsingTypeNo();
            break;
        case USAGE_TYPE_READ: {
            const Block * const focused = GetFocusedBlock();
            if ( focused != nullptr ) {
                DisplayFile(QString(home_path + world->WorldName()
                    + "/texts/" + GetFocusedBlock()->GetNote()));
                player->SetUsingTypeNo();
            }
            } break;
        case USAGE_TYPE_OPEN: {
            Block * const focused = GetFocusedBlock();
            if ( focused != nullptr ) {
                PrintInv(rightWin, focused, focused->HasInventory());
            }
            } break;
        }
    }
    world->Unlock();
} // void Screen::Print()

void Screen::PrintHUD() {
    werase(hudWin);
    if ( player->GetCreativeMode() ) {
        mvwaddwstr(hudWin, 0, 0,
            (tr("Creative Mode\nxyz: %1, %2, %3.\nShred: %4")
            .arg(player->GlobalX()).arg(player->GlobalY()).arg(player->Z())
            .arg(Shred::FileName(GetWorld()->WorldName(),
                player->GetLongitude(), player->GetLatitude()) )).
                    toStdWString().c_str());
    } else {
        const int dur = player->GetBlock()->GetDurability();
        if ( dur > 0 ) { // HitPoints line
            static const int player_health_char = ascii ? '@' : 0x2665;
            PrintBar(hudWin, player_health_char,
                int((dur > MAX_DURABILITY/5) ?
                    COLOR_PAIR(RED_BLACK) : (COLOR_PAIR(BLACK_RED) | A_BLINK)),
                dur*100/MAX_DURABILITY);
        }
        static const struct {
            std::wstring name;
            int color;
        } satiation[] = {
            { tr("Hungry" ).toStdWString(),   RED_BLACK },
            { tr("Content").toStdWString(), WHITE_BLACK },
            satiation[1],
            { tr("Full"   ).toStdWString(), GREEN_BLACK },
            { tr("Gorged" ).toStdWString(),  BLUE_BLACK }
        };
        waddstr(hudWin, "   ");
        const int satiation_state = player->SatiationPercent() / 25;
        wcolor_set(hudWin, satiation[satiation_state].color, nullptr);
        waddwstr(hudWin,
            satiation[satiation_state].name.c_str());
        waddch(hudWin, '\n');
        const int breath = player->BreathPercent();
        if ( breath != 100 ) {
            static const int breath_char = ascii ? 'o' : 0x00b0;
            PrintBar(hudWin, breath_char, COLOR_PAIR(BLUE_BLACK), breath);
        }
    }
    const Block * const focused = GetFocusedBlock();
    if ( focused && Block::GetSubGroup(focused->Sub()) != GROUP_AIR ) {
        const int left_border = getmaxx(hudWin);
        (void)wmove(hudWin, 0, left_border - 15);
        PrintBar(hudWin, CharName(focused->Kind(), focused->Sub()),
            Color(focused->Kind(), focused->Sub()),
            focused->GetDurability()*100/MAX_DURABILITY);
        wstandend(hudWin);
        const QString name = focused->FullName();
        mvwaddwstr(hudWin, 1, left_border - name.length(),
            name.toStdWString().c_str());
        const QString note = focused->GetNote();
        if ( not note.isEmpty() && IsScreenWide() ) {
            const int width = qMin(34, note.length());
            mvwaddstr(hudWin, 2, left_border - width - 2, "~:");
            if ( note.length() <= width ) {
                waddwstr(hudWin, note.toStdWString().c_str());
            } else {
                waddnwstr(hudWin, note.toStdWString().c_str(),
                    width - 1 - (ascii * 2));
                waddwstr(hudWin, ellipsis);
            }
        }
    }
    PrintQuickInventory();
    wrefresh(hudWin);
    PrintMiniMap();
} // void Screen::PrintHUD()

void Screen::PrintQuickInventory() {
    const Inventory * const inv = player->PlayerInventory();
    if ( inv==nullptr || not IsScreenWide() ) return;

    wstandend(hudWin);
    const int inventory_size = inv->Size();
    int x = getmaxx(hudWin)/2 - inventory_size;
    for (int i=0; i<inventory_size; ++i) {
        mvwaddch(hudWin, 0, x, 'a'+i | COLOR_PAIR(WHITE_BLACK));
        switch ( inv->Number(i) ) {
        case  0: break;
        default: mvwaddch(hudWin, 2, x, inv->Number(i)+'0'); // no break;
        case  1: mvwaddch(hudWin, 1, x, ColoredChar(inv->ShowBlock(i)));
            break;
        }
        x += 2;
    }
}

int Screen::GetMinimapStartX() const {
    return Shred::CoordOfShred(player->X()) - 2;
}

int Screen::GetMinimapStartY() const {
    return Shred::CoordOfShred(player->Y()) - 2;
}

void Screen::PrintMiniMap() {
    (void)wmove(minimapWin, 1, 0);
    const int i_start = GetMinimapStartY();
    const int j_start = GetMinimapStartX();
    for (int i=i_start; i <= i_start+4; ++i, waddch(minimapWin, '\n'))
    for (int j=j_start; j <= j_start+4; ++j) {
        if ( i<0 || j<0 || i>=world->NumShreds() || j>=world->NumShreds() ) {
            waddch(minimapWin, ' ' | COLOR_PAIR(BLACK_BLACK));
            waddch(minimapWin, ' ' | COLOR_PAIR(BLACK_BLACK));
        } else {
            const shred_type t = world->GetShredByPos(j, i)->GetTypeOfShred();
            const int color = ColorShred(t);
            waddch(minimapWin, color | ' ');
            waddch(minimapWin, color | t);
        }
    }
    DrawBorder(minimapWin);
    static const std::wstring title = tr("Minimap").toStdWString();
    mvwaddwstr(minimapWin, 0, 1, title.c_str());
    wrefresh(minimapWin);
}

int Screen::GetNormalStartX() const {
    return qBound(0, player->X() - screenWidth/4,
        GetWorld()->GetBound() - screenWidth/2 - 1);
}

int Screen::GetNormalStartY() const {
    return qBound(0, player->Y() - screenHeight/2,
        GetWorld()->GetBound() - screenHeight - 1);
}

void Screen::PrintNormal(WINDOW * const window, const dirs dir) const {
    (void)wmove(window, 1, 1);
    int k_start, k_step;
    if ( UP == dir ) {
        k_start = player->Z() + 1;
        k_step  = 1;
    } else {
        k_start = player->Z() - ( DOWN==dir ) + shiftFocus * (DOWN!=dir);
        k_step  = -1;
    }
    const int start_x = GetNormalStartX();
    const int start_y = GetNormalStartY();
    const int end_x = start_x + screenWidth/2 - 1;
    const int end_y = start_y + screenHeight  - 2;
    for (int j=start_y; j<end_y; ++j, waddstr(window, "__")) {
        for (int i=start_x; i<end_x; ++i ) {
            Shred * const shred = world->GetShred(i, j);
            const int i_in = Shred::CoordInShred(i);
            const int j_in = Shred::CoordInShred(j);
            int k = k_start;
            for ( ; INVISIBLE == shred->GetBlock(i_in, j_in, k)->Transparent();
                k += k_step);
            if ( player->Visible(i, j, k) ) {
                PrintBlock(shred->GetBlock(i_in, j_in, k), window,
                    showDistance ? CharNumber(k) : ' ');
            } else {
                waddch(window, SHADOW_COLOR | OBSCURE_BLOCK);
                waddch(window, SHADOW_COLOR | ' ');
            }
        }
    }

    if ( dir > DOWN ) {
        const Block * const block = player->GetBlock();
        wattrset(window, Color(block->Kind(), block->Sub()));
        mvwaddwstr(window, player->Y()-start_y+1, (player->X()-start_x)*2+2,
            arrows[player->GetDir()]);
    }

    DrawBorder(window);
    Arrows(window, (player->X()-start_x)*2+1, player->Y()-start_y+1, UP);
    if ( shiftFocus ) {
        const int ch =
            (( shiftFocus == 1 ) ? '+' : '-') | COLOR_PAIR(WHITE_BLUE);
        mvwaddch(leftWin, 0,               0, ch);
        mvwaddch(leftWin, 0, screenWidth+1, ch);
        mvwaddch(leftWin, screenHeight+1,   0, ch);
        mvwaddch(leftWin, screenHeight+1, screenWidth+1, ch);
    }
    wrefresh(window);
} // void Screen::PrintNormal(WINDOW * window, int dir)

void Screen::DrawBorder(WINDOW * const window) const {
    static const int BORDER_COLOR = COLOR_PAIR(BLACK_BLACK) | A_BOLD;
    (void)wborder( window,
        ACS_VLINE    | BORDER_COLOR, ACS_VLINE    | BORDER_COLOR,
        ACS_HLINE    | BORDER_COLOR, ACS_HLINE    | BORDER_COLOR,
        ACS_ULCORNER | BORDER_COLOR, ACS_URCORNER | BORDER_COLOR,
        ACS_LLCORNER | BORDER_COLOR, ACS_LRCORNER | BORDER_COLOR );
}

void Screen::PrintFront(const dirs dir, const int block_x, const int block_y)
const {
    int x_step,  z_step,
        x_start, z_start,
        x_end,   z_end;
    int * x, * z;
    int i, j;
    int arrow_X;
    switch ( dir ) {
    case NORTH:
        x = &i;
        x_step  = 1;
        x_start = GetNormalStartX();
        x_end   = x_start + screenWidth/2 - 1;
        z = &j;
        z_step  = -1;
        z_start = player->Y() - 1;
        z_end   = qMax(0, player->Y() - SHRED_WIDTH*2);
        arrow_X = (player->X() - x_start)*2 + 1;
        break;
    case SOUTH:
        x = &i;
        x_step  = -1;
        x_end   = GetNormalStartX() - 1;
        x_start = screenWidth/2 - 1 + x_end;
        z = &j;
        z_step  = 1;
        z_start = player->Y() + 1;
        z_end   = qMin(player->Y() + SHRED_WIDTH*2, world->GetBound());
        arrow_X = (screenWidth/2 - player->X() + x_end)*2 - 1;
        break;
    case EAST:
        x = &j;
        x_step  = 1;
        x_start = GetNormalStartY();
        x_end   = screenWidth/2 - 1 + x_start;
        z = &i;
        z_step  = 1;
        z_start = player->X() + 1;
        z_end   = qMin(player->X() + SHRED_WIDTH*2, world->GetBound());
        arrow_X = (player->Y() - x_start)*2 + 1;
        break;
    case WEST:
        x = &j;
        x_step  = -1;
        x_end   = GetNormalStartY() - 1;
        x_start = screenWidth/2 - 1 + x_end;
        z = &i;
        z_step  = -1;
        z_start = player->X() - 1;
        z_end   = qMax(0, player->X() - SHRED_WIDTH*2);
        arrow_X = (screenWidth/2 - player->Y() + x_end)*2 - 1;
        break;
    default:
        Q_UNREACHABLE();
        return;
    }
    const int k_start =
        qBound(screenHeight-1, player->Z()+screenHeight/2, HEIGHT-1);
    if ( block_x > 0 ) {
        // ugly! use print function to get block by screen coordinates.
        const int k = k_start - block_y + 1;
        *x = x_start + x_step * (block_x-1)/2;
        for (*z=z_start; *z!=z_end && world->GetBlock(i, j, k)->
                    Transparent()==INVISIBLE;
                *z += z_step);
        player->Examine(i, j, k);
        return;
    }
    const int k_end = k_start - screenHeight + 2;
    const int sky_color = COLOR_PAIR(VirtScreen::Color(BLOCK, SKY));
    (void)wmove(rightWin, 1, 1);
    for (int k=k_start; k>k_end; --k, waddstr(rightWin, "__")) {
        for (*x=x_start; *x!=x_end; *x+=x_step) {
            for (*z=z_start; *z!=z_end && world->GetBlock(i, j, k)->
                        Transparent()==INVISIBLE;
                    *z += z_step);
            if ( *z == z_end ) {
                static const int sky_char = CharName(BLOCK, SKY);
                waddch(rightWin, sky_color | sky_char);
                waddch(rightWin, sky_color | ' ');
            } else if ( player->Visible(i, j, k) ) {
                PrintBlock(world->GetBlock(i, j, k), rightWin,
                    showDistance ? CharNumberFront(i, j) : ' ');
            } else {
                waddch(rightWin, SHADOW_COLOR | OBSCURE_BLOCK);
                waddch(rightWin, SHADOW_COLOR | ' ');
            }
        }
    }
    DrawBorder(rightWin);
    const int arrow_Y = k_start + 1 - player->Z();
    if ( shiftFocus ) {
        const int ch =
            (( shiftFocus == 1 ) ? '^' : 'v') | COLOR_PAIR(WHITE_BLUE);
        for (int q=arrow_Y-shiftFocus; 0<q && q<=screenWidth/2; q-=shiftFocus){
            mvwaddch(rightWin, q,               0, ch);
            mvwaddch(rightWin, q, screenWidth+1, ch);
        }
    }
    Arrows(rightWin, arrow_X, arrow_Y, dir);
    wrefresh(rightWin);
} // void Screen::PrintFront(dirs)

void Screen::PrintInv(WINDOW * const window,
        const Block * const block, const Inventory * const inv)
const {
    if ( inv == nullptr ) return;
    werase(window);
    const int start = inv->Start();
    int shift = 0; // to divide inventory sections
    for (int i=0; i<inv->Size(); ++i) {
        shift += ( start == i && i != 0 );
        wstandend(window);
        mvwprintw(window, 2+i+shift, 1, "%c) ", 'a'+i);
        const Block * const block = inv->ShowBlock(i);
        if ( block == nullptr ) {
            wattrset(window, COLOR_PAIR(BLACK_BLACK) | A_BOLD);
            waddwstr(window, inv->InvFullName(i).toStdWString().c_str());
            continue;
        }
        PrintBlock(block, window, ' ');
        wstandend(window);
        waddch(window, ' ');
        waddwstr(window, inv->InvFullName(i).toStdWString().c_str());
        if ( MAX_DURABILITY != block->GetDurability() ) {
            wprintw(window, "{%d}", block->GetDurability()*100/MAX_DURABILITY);
        }
        const QString str = block->GetNote();
        if ( not str.isEmpty() ) {
            const int width = screenWidth - getcurx(window) - 3 - 8;
            waddstr(window, " ~:");
            if ( str.length() <= width ) {
                waddwstr(window, str.toStdWString().c_str());
            } else {
                waddnwstr( window, str.toStdWString().c_str(),
                    width - 2 - ( ascii * 2 ) );
                waddwstr(window, ellipsis);
            }
        }
        wstandend(window);
        mvwprintw(window, 2+i+shift, 53, "%5hu mz", inv->GetInvWeight(i));
    }
    wstandend(window);
    QString full_weight = tr("Full weight: %1 mz").
        arg(inv->Weight(), 6, 10, QChar(' '));
    mvwaddwstr( window, 2+inv->Size()+shift,
        screenWidth + 1 - full_weight.length(),
        full_weight.toStdWString().c_str() );
    wattrset(window, Color(block->Kind(), block->Sub()));
    box(window, 0, 0);
    if ( start != 0 ) {
        mvwhline(window, 2+start, 1, ACS_HLINE, screenWidth);
    }
    mvwprintw(window, 0, 1, "[%c] ", CharName( block->Kind(), block->Sub()));
    waddwstr(window, ((player->PlayerInventory() == inv) ?
        tr("Your inventory") : block->FullName()).toStdWString().c_str() );
    wrefresh(window);
} // void Screen::PrintInv(WINDOW *, const Block *, const Inventory *)

void Screen::CleanFileToShow() {
    delete fileToShow;
    fileToShow = nullptr;
}

bool Screen::PrintFile(WINDOW * const window, QString const & file_name) {
    CleanFileToShow();
    fileToShow = new QFile(file_name);
    if ( fileToShow->open(QIODevice::ReadOnly | QIODevice::Text) ) {
        werase(window);
        const QByteArray text = fileToShow->readAll();
        waddwstr(window,
            QString::fromUtf8(text.constData()).toStdWString().c_str() );
        wrefresh(window);
        return true;
    } else {
        CleanFileToShow();
        return false;
    }
}

void Screen::DisplayFile(QString path) {
    wstandend(rightWin);
    if ( not PrintFile(rightWin, path) ) {
        Notify(tr("There is no such file."));
    }
}

void Screen::Notify(const QString str) const {
    fputs(qPrintable(QString("%1 %2\n").arg(world->TimeOfDayStr()).arg(str)),
        notifyLog);
    if ( inputActive ) return;
    wstandend(notifyWin);
    switch ( str.at(str.size()-1).unicode() ) {
    case '!': wcolor_set(notifyWin, RED_BLACK, nullptr); // no break;
    case '*': if ( flashOn ) flash(); break;
    case '^': if (  beepOn ) beep();  break;
    }
    static int notification_repeat_count = 1;
    if ( str == lastNotification ) {
        ++notification_repeat_count;
        mvwaddwstr(notifyWin, getcury(notifyWin)-1, 0,
            str.toStdWString().c_str());
        wprintw(notifyWin, " (x%d)\n", notification_repeat_count);
    } else {
        notification_repeat_count = 1;
        waddwstr(notifyWin, (lastNotification = str).toStdWString().c_str());
        waddch(notifyWin, '\n');
    }
    wrefresh(notifyWin);
}

void Screen::DeathScreen() {
    werase(rightWin);
    werase(hudWin);
    wcolor_set(leftWin, WHITE_RED, nullptr);
    if ( not PrintFile(leftWin, ":/texts/death.txt") ) {
        waddwstr(leftWin,
            tr("You die.\nWaiting for respawn...").toStdWString().c_str());
    }
    box(leftWin, 0, 0);
    wnoutrefresh(leftWin);
    wnoutrefresh(rightWin);
    wnoutrefresh(hudWin);
    doupdate();
    updated = true;
}

Screen::Screen(Player * const pl, int &) :
        VirtScreen(pl),
        lastNotification(),
        input(new IThread(this)),
        updated(),
        notifyLog(fopen(qPrintable(home_path + "log.txt"), "at")),
        actionMode(ACTION_USE),
        shiftFocus(settings.value("focus_shift", 0).toInt()),
        fileToShow(nullptr),
        beepOn (settings.value("beep_on",  false).toBool()),
        flashOn(settings.value("flash_on", true ).toBool()),
        ascii  (settings.value("ascii",    false).toBool()),
        blinkOn(settings.value("blink_on", true ).toBool()),
        arrows{
            { wchar_t('.') },
            { wchar_t('x') },
            { wchar_t(ascii ? '^' : 0x2191) },
            { wchar_t(ascii ? 'v' : 0x2193) },
            { wchar_t(ascii ? '>' : 0x2192) },
            { wchar_t(ascii ? '<' : 0x2190) }
        },
        ellipsis{
            wchar_t(ascii ? '.' : 0x2026 ),
            wchar_t(ascii ? '.' : 0),
            wchar_t(ascii ? '.' : 0),
        },
        screen(newterm(nullptr, stdout, stdin)),
        showDistance(settings.value("show_distance", true).toBool()),
        farDistance(settings.value("use_abcdef_distance", false).toBool()),
        noMouseMask(),
        mouseOn(settings.value("mouse_on", true).toBool()),
        screenWidth((COLS / 2) - (COLS/2)%2),
        screenHeight(LINES - 10)
{
    #ifndef Q_OS_WIN32
        set_escdelay(10);
    #endif
    start_color();
    nodelay(stdscr, FALSE);
    noecho(); // do not print typed symbols
    nonl();
    keypad(stdscr, TRUE); // use arrows
    mousemask(BUTTON1_CLICKED | BUTTON1_RELEASED, &noMouseMask);
    if ( not mouseOn ) {
        mousemask((mouseOn ? MOUSEMASK : noMouseMask), nullptr);
    }
    memset(windows, 0, sizeof(windows));

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

    leftWin  = newwin(screenHeight, screenWidth, 0, COLS/2%2);
    rightWin = newwin(screenHeight, screenWidth, 0, COLS/2);
    hudWin   = newwin(3, 0, screenHeight, 0);
    minimapWin = newwin(MINIMAP_HEIGHT, MINIMAP_WIDTH, LINES-7, 0);
    actionWin = newwin(7, ACTIONS_WIDTH, LINES-7, MINIMAP_WIDTH + 1);
    notifyWin = newwin(0, 0, LINES-7, MINIMAP_WIDTH + 1 + ACTIONS_WIDTH + 1);

    scrollok(notifyWin, TRUE);

    if ( not PrintFile(stdscr, ":/texts/splash.txt") ) {
        addstr("Free-Roaming Elementary Game\nby mmaulwurff\n");
    }
    addwstr((tr("\nVersion %1.\n\nPress any key.").arg(VER)).
        toStdWString().c_str());
    qsrand(getch());
    CleanFileToShow();
    RePrint();
    Greet();
    SetActionMode(static_cast<actions>
        (settings.value("action_mode", ACTION_USE).toInt()));
    Print();
    input->start();
    connect(world, SIGNAL(UpdatesEnded()), SLOT(Print()),
        Qt::DirectConnection);
} // Screen::Screen(Player * const pl, int & error, bool _ascii)

Screen::~Screen() {
    world->Lock();
    disconnect(world, SIGNAL(UpdatesEnded()), this, SLOT(Print()));
    world->Unlock();

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
    settings.setValue("focus_shift", shiftFocus);
    settings.setValue("action_mode", actionMode);
    settings.setValue("last_command", previousCommand);
    settings.setValue("beep_on",  beepOn);
    settings.setValue("flash_on", flashOn);
    settings.setValue("blink_on", blinkOn);
    settings.setValue("ascii", ascii);
    settings.setValue("mouse_on", mouseOn);
    settings.setValue("show_distance", showDistance);
    settings.setValue("use_abcdef_distance", farDistance);
}

void Screen::Greet() const {
    Notify(tr("--- Game started. Press 'H' for help. ---"));
}

void Screen::PrintBar(WINDOW * const window,
        const wchar_t ch, const int color, const int percent)
{
    wstandend(window);
    const int x = getcurx(window) + 1;
    waddstr(window, "[..........]");
    const wchar_t durability_string[10] =
        {ch, ch, ch, ch, ch, ch, ch, ch, ch, ch};
    wattrset(window, color);
    mvwaddnwstr(window, getcury(window), x, durability_string, percent/10);
    mvwprintw  (window, getcury(window), x + 11, "%3d", percent);
}
