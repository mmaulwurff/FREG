    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2015 Alexander 'mmaulwurff' Kromm
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

/** \file CursedScreen.cpp
 *  \brief This file is related to curses screen for freg. */

#include "screens/CursedScreen.h"
#include "screens/IThread.h"
#include "Shred.h"
#include "World.h"
#include "Player.h"
#include "TrManager.h"
#include "blocks/Block.h"
#include "blocks/Inventory.h"

#include <QDesktopServices>
#include <QUrl>

#undef getmaxx // too dangerous macro, just do not use it.
#define getmaxx(smth)

#define wPrintable(string) QString(string).toStdWString().c_str()

#define OPTIONS_SAVE(string, name, ...) \
settings.setValue(QStringLiteral(string), name);
#define OPTIONS_INIT(string, name, default, ...) \
name(settings.value(QStringLiteral(string), default).toBool()),

const chtype Screen::arrows[] = { '.', 'x', '^', '>', 'v', '<' };
const int Screen::MAX_CHAR_DISTANCE = 16;
Screen* Screen::staticScreen = nullptr;

void Screen::Arrows(WINDOW* const window, const int x, const int y,
        const dirs direction, const bool is_normal)
const {
    wstandend(window);
    static const int ARROWS_COLOR = COLOR_PAIR(WHITE_RED);
    // vertical
    const chtype   up_str[] { arrows[SOUTH] | ARROWS_COLOR,   up_str[0] };
    const chtype down_str[] { arrows[NORTH] | ARROWS_COLOR, down_str[0] };
    mvwaddchnstr(window,              0, x,   up_str, 2);
    mvwaddchnstr(window, screenHeight-1, x, down_str, 2);
    static const std::wstring north_up[] {
        TrManager::DirName(UP   ).toStdWString(),
        TrManager::DirName(NORTH).toStdWString()
    };
    static const std::wstring south_down[] {
        TrManager::DirName(DOWN ).toStdWString(),
        TrManager::DirName(SOUTH).toStdWString()
    };
    mvwaddwstr(window,              0, x + 3, north_up  [is_normal].c_str());
    mvwaddwstr(window, screenHeight-1, x + 3, south_down[is_normal].c_str());

    // horizontal
    static const std::wstring dir_chars[] {
        TrManager::DirName(NORTH).left(1).toStdWString(),
        TrManager::DirName(EAST ).left(1).toStdWString(),
        TrManager::DirName(SOUTH).left(1).toStdWString(),
        TrManager::DirName(WEST ).left(1).toStdWString()
    };
    mvwaddwstr(window, y-2,0, dir_chars[World::TurnLeft(direction)-2].c_str());
    mvwaddwstr(window, y-2, screenWidth-1,
        dir_chars[World::TurnRight(direction)-2].c_str());
    mvwaddch(window, y,             0, arrows[EAST] | ARROWS_COLOR);
    mvwaddch(window, y, screenWidth-1, arrows[WEST] | ARROWS_COLOR);
}

void Screen::RePrint() {
    erase();
    refresh();
    std::for_each(ALL(windows), wclear);
    static const std::wstring action_strings[] = {
        tr("[U] use, eat" ).toStdWString(),
        tr("[T] throw"    ).toStdWString(),
        tr("[G] get"      ).toStdWString(),
        tr("[N] inscribe" ).toStdWString(),
        tr("[B] build"    ).toStdWString(),
        tr("[C] craft"    ).toStdWString(),
        tr("[E] equipment").toStdWString(),
    };
    (void)wmove(actionWin, 0, 0);
    for (int i=0; i<=ACTION_WIELD; ++i) {
        waddwstr(actionWin, action_strings[i].c_str());
        waddch(actionWin, '\n');
    }
    mvwchgat(actionWin, actionMode, 0, -1, A_NORMAL, BLACK_WHITE, nullptr);
    UpdateAll();
    Print();
    wrefresh(actionWin);
}

void Screen::UpdatePlayer() { updatedHud = false; }
void Screen::Move(int) {}
void Screen::UpdateAround(int, int, int) {
    updatedNormal = updatedFront = updatedHud = updatedMinimap = false;
}

bool Screen::IsScreenWide() { return COLS >= (AVERAGE_SCREEN_SIZE + 2) * 2; }

void Screen::UpdateAll() {
    updatedNormal = updatedFront = updatedHud = updatedMinimap = false;
}

void Screen::PassString(QString& str) const {
    inputActive = true;
    scroll(notifyWin);
    wattrset(notifyWin, A_UNDERLINE);
    mvwaddwstr(notifyWin, 6, 0, wPrintable(tr("Enter input: ")));
    echo();
    wint_t temp_str[MAX_NOTE_LENGTH + 1];
    wgetn_wstr(notifyWin, temp_str, MAX_NOTE_LENGTH);
    inputActive = false;
    wscrl(notifyWin, -1);
    noecho();
    str = QString::fromUcs4(temp_str);
    Log(Str("Input: ") + str);
}

char Screen::Distance(const int dist) const {
    if ( Q_UNLIKELY(dist < 0) ) return '-';
    static const char dist_chars[MAX_CHAR_DISTANCE+1] = " 123456789abcdef";
    return showCharDistance ?
        (dist < showCharDistance ? dist_chars[dist] : '+') :
        ' ';
}

char Screen::CharNumber(const int z) const {
    return Distance( ( UP == player->GetDir() ) ?
        z - player->Z() :
        player->Z() - z );
}

char Screen::CharNumberFront(const int i, const int j) const {
    return Distance( ( ( player->GetDir() & 1 ) ? // east or west
        abs(player->X() - i) :
        abs(player->Y() - j) ) - 1 );
}

int  Screen::RandomBlink() { return (RandomBit() * A_REVERSE); }

bool Screen::RandomBit() {
    static unsigned rands = 0;
    return 1 & ( rands ?
        (rands >>= 1) :
        (rands = qrand()) );
}

int Screen::Color(const int kind, const int sub) {
    const int color = COLOR_PAIR(VirtScreen::Color(kind, sub));
    switch ( kind ) { // foreground_background
    default: switch ( sub ) {
        default:         return color;
        case IRON:
        case NULLSTONE:
        case DIAMOND:    return color | A_BOLD;
        case ACID:
        case SUB_DUST:   return color | A_BOLD  | A_REVERSE;
        case FIRE:       return color | A_BLINK | RandomBlink();
        case GOLD:       return color | RandomBlink();
        } break;
    case LIQUID: switch ( sub ) {
        case H_MEAT:
        case A_MEAT:
        case SUB_CLOUD: return color;
        default:        return color | RandomBlink() | A_BLINK;
        case ACID:      return color | RandomBlink() | A_BOLD;
        case WATER: return RandomBit() ? color : COLOR_PAIR(BLUE_BLUE)|A_BOLD;
        } break;
    case TELEGRAPH: return color | A_BOLD;
    case TELEPORT:  return color | (RandomBit() ? A_BOLD : 0);
    }
} // color_pairs Screen::Color(int kind, int sub)

int Screen::ColorShred(const shred_type type) {
    switch ( type ) { // foreground_background
    case SHRED_UNDERGROUND:
    case SHRED_TESTSHRED:
    case SHRED_EMPTY:
    case SHRED_FLAT:
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

void Screen::MovePlayer(const dirs dir) const {
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
    if ( Q_UNLIKELY(player->GetConstBlock() == nullptr) ) return;
    if ( 0 < ch && ch < ASCII_SIZE ) {
        keyTable[ch](ch);
    } else {
        switch ( ch ) {
        case ERR: break; // received when idle

        case KEY_UP:    keyTable[int('W')](0); break;
        case KEY_DOWN:  keyTable[int('S')](0); break;
        case KEY_RIGHT: keyTable[int('D')](0); break;
        case KEY_LEFT:  keyTable[int('A')](0); break;
        case KEY_END:   keyTable[int('5')](0); break;
        case KEY_NPAGE: keyTable[int('V')](0); break;
        case KEY_PPAGE: keyTable[int('^')](0); break;

        case KEY_HOME:  keyTable[int('I')](0); break;
        case KEY_BACKSPACE: keyTable[int('K')](0); break;

        case KEY_HELP:
        case KEY_F(1):  keyTable[int('H')](0); break;
        case KEY_F(2):  keyTable[int('F')](0); break;
        case KEY_F(3):  keyTable[int('?')](0); break;
        case KEY_F(4):  keyTable[int('~')](0); break;
        case KEY_F(5):  keyTable[int('R')](0); break;
        case KEY_DC: // delete
        case KEY_F(8):  keyTable[    8   ](0); break;
        case KEY_F(9):  keyTable[int(':')](0); break;
        case KEY_F(10): keyTable[int('Q')](0); break;
        case KEY_F(12): keyTable[int('!')](0); break;

        case KEY_IC: player->Build(ACTIVE_HAND); break; // insert
        case KEY_BREAK: keyTable[int('P')](0); break;

        case KEY_MOUSE: ProcessMouse(); break;

        case KEY_RESIZE:
            screenWidth = (COLS / 2) - ((COLS/2) & 1);
            screenHeight = LINES - 10;
            wresize(leftWin,  screenHeight, screenWidth);
            wresize(rightWin, screenHeight, screenWidth);
            wresize(notifyWin, 7, 0);
            wresize(hudWin,    3, 0);

            mvwin(actionWin, LINES-7, MINIMAP_WIDTH + 1);
            mvwin(notifyWin, LINES-7, MINIMAP_WIDTH + ACTIONS_WIDTH + 2);
            mvwin(hudWin, screenHeight, 0);
            mvwin(minimapWin, LINES-7, 0);
            mvwin(leftWin, 0, (COLS/2) & 1);
            mvwin(rightWin, 0, COLS/2);

            RePrint();
            refresh();
            break;

        default: keyTable[0](ch); break; // unknown key message
        }
    }
    updatedNormal = updatedFront = updatedHud = false;
} // void Screen::ControlPlayer(int ch)

void Screen::ExamineOnNormalScreen(int x, int y, int z, const int step) const {
    const World* const world = World::GetConstWorld();
    x = (x-1)/2 + GetNormalStartX();
    y =  y-1    + GetNormalStartY();
    for ( ; world->GetBlock(x, y, z)->Transparent() == INVISIBLE; z += step);
    player->Examine(x, y, z);
}

void Screen::ProcessMouse() {
    MEVENT mevent;
    if ( Q_UNLIKELY(getmouse(&mevent) == ERR) ) return;
    int window_index = 0;
    for (; window_index < WIN_COUNT; ++window_index ) {
        if ( wenclose(windows[window_index], mevent.y, mevent.x) ) {
            wmouse_trafo(windows[window_index], &mevent.y, &mevent.x, false);
            break;
        }
    }
    switch ( static_cast<windowIndex>(window_index) ) {
    case WIN_ACTION: SetActionMode(static_cast<actions>(mevent.y)); break;
    case WIN_NOTIFY: Notify(tr("Notifications area.")); break;
    case WIN_HUD:
        mevent.x -= screenWidth - 'z' + 'a' - 1;
        mevent.x /= 2;
        if ( not ( IsScreenWide() && 0 <= mevent.x && mevent.x <= 'z'-'a' ) ) {
            Notify(tr("Information: left - player, right - focused thing."));
        } else if ( const Inventory* const inv = player->PlayerInventory() ) {
            if ( inv->IsEmpty(mevent.x) ) {
                Notify(tr("Nothing in inventory at slot '%1'.").
                    arg(char(mevent.x + 'a')));
            } else {
                Notify(tr("In inventory at slot '%1':").
                    arg(char(mevent.x + 'a')));
                player->Examine(inv->ShowBlock(mevent.x));
            }
        }
        break;
    case WIN_MINIMAP:
        if ( IsOutWindow(mevent, MINIMAP_WIDTH-1, MINIMAP_HEIGHT-1) ) {
            Notify(tr("Minimap."));
        } else {
            const int shred_x = mevent.x/2 + GetMinimapStartX();
            const int shred_y = mevent.y-1 + GetMinimapStartY();
            const World* const world = World::GetConstWorld();
            Notify( (0 <= shred_x && shred_x < world->NumShreds() &&
                     0 <= shred_y && shred_y < world->NumShreds() ) ?
                tr("On minimap: %1.").
                    arg(TrManager::ShredTypeName(world->
                        GetShredByPos(shred_x, shred_y)->GetTypeOfShred())) :
                tr("You can't see that far.") );
        }
        break;
    case WIN_LEFT:
        if ( player->UsingSelfType() == USAGE_TYPE_OPEN ) {
            /// \todo examine inventory contents.
            Notify(tr("Your inventory."));
        } else if ( IsOutWindow(mevent, screenWidth-1, screenHeight-1) ) {
            Notify(tr("Left window, Down view."));
        } else {
            ExamineOnNormalScreen(mevent.x, mevent.y, player->Z(), -1);
        }
        break;
    case WIN_RIGHT:
        if (player->UsingType() == USAGE_TYPE_OPEN ) {
            /// \todo examine inventory contents.
            Notify(tr("Opened inventory."));
        } else if ( IsOutWindow(mevent, screenWidth-1, screenHeight-1) ) {
            Notify(tr("Right window, %1 view.").
                arg(TrManager::DirName(player->GetDir())));
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
    case WIN_COUNT:
        Notify(tr("Nothing here. Click on something to get information."));
        break;
    }
} // Screen::ProcessMouse()

bool Screen::IsOutWindow(const MEVENT& mevent,
                         const int hor_bound, const int vert_bound)
{
    return not ( 0 < mevent.x && mevent.x < hor_bound &&
                 0 < mevent.y && mevent.y < vert_bound );
}

void Screen::ProcessCommand(const QString command) {
    if ( command.length() == 1 && command.at(0).toLatin1() != '.' ) {
        ControlPlayer(command.at(0).toLatin1());
        return;
    }
    if ( VirtScreen::ProcessCommand(command) ) return;
    switch ( Player::UniqueIntFromString(qPrintable(command)) ) {
    case Player::UniqueIntFromString("plus_distance"):
        showCharDistance = std::min(showCharDistance+1, MAX_CHAR_DISTANCE);
        Notify(tr("Show distance: %1.").arg(showCharDistance));
        break;
    case Player::UniqueIntFromString("minus_distance"):
        showCharDistance = std::max(showCharDistance-1, 0);
        Notify(tr("Show distance: %1.").arg(showCharDistance));
        break;
    case Player::UniqueIntFromString("blink"):
        blinkOn = not blinkOn;
        Notify(tr("Block blink is now %1.").arg(TrManager::OffOn(blinkOn)));
        break;
    case Player::UniqueIntFromString("size"):
        Notify(tr("Terminal: %1 rows, %2 columns. Screen: %3x%4 blocks.").
            arg(LINES).
            arg(COLS).
            arg(screenHeight - 2).
            arg((screenWidth - 2) / 2));
        break;
    case Player::UniqueIntFromString("palette"): Palette(notifyWin); break;
    case Player::UniqueIntFromString("test"): TestNotify(); break;
    default: player->ProcessCommand(command); break;
    }
}

void Screen::SetActionMode(const actions mode) {
    mvwchgat(actionWin, actionMode,     0, -1, A_NORMAL, WHITE_BLACK, nullptr);
    mvwchgat(actionWin, actionMode=mode,0, -1, A_NORMAL, BLACK_WHITE, nullptr);
    wrefresh(actionWin);
    static const QString actionStrings[] = {
        tr("Action: use in inventory."     ),
        tr("Action: throw from inventory." ),
        tr("Action: get to inventory."     ),
        tr("Action: inscribe in inventory."),
        tr("Action: build from inventory." ),
        tr("Action: craft in inventory."   ),
        tr("Action: organize equipment."   )
    };
    Notify(actionStrings[mode]);
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

void Screen::ActionXyz(int* const x, int* const y, int* const z) const {
    VirtScreen::ActionXyz(x, y, z);
    if ( player->GetDir() > DOWN &&
            ( AIR == World::GetConstWorld()->GetBlock(*x, *y, *z)->Sub() ||
              AIR == World::GetConstWorld()->GetBlock(
                player->X(),
                player->Y(),
                player->Z()+shiftFocus)->Sub() ))
    {
        *z += shiftFocus;
    }
}

Block* Screen::GetFocusedBlock() const {
    int x, y, z;
    ActionXyz(&x, &y, &z);
    return ( player->Visible(x, y, z) == Player::VISIBLE ) ?
        World::GetConstWorld()->GetBlock(x, y, z) :
        nullptr;
}

void Screen::PrintBlock(const Block* const block, WINDOW* const window,
        const char second)
{
    const int kind = block->Kind();
    const int sub  = block->Sub();
    const int color = Color(kind, sub);
    waddch(window, color | (true ? CharName(kind, sub) : ' '));
    waddch(window, color | second);
}

int Screen::ColoredChar(const Block* const block) {
    const int kind = block->Kind();
    const int sub  = block->Sub();
    return CharName(kind, sub) | Color(kind, sub);
}

void Screen::PrintShadow(WINDOW* const window) {
    static const chtype SHADOW = COLOR_PAIR(BLACK_BLACK)|A_BOLD | ACS_CKBOARD;
    waddch(window, SHADOW);
    waddch(window, SHADOW);
}

void Screen::Print() {
    const QMutexLocker locker(World::GetWorld()->GetLock());
    PrintHud();
    updatedMinimap = true;
    const dirs dir = player->GetDir();
    bool printed_normal = false;
    if ( player->UsingSelfType() != USAGE_TYPE_OPEN ) { // left window
        if ( not updatedNormal ) {
            PrintNormal(leftWin, (dir <= DOWN) ? NORTH : dir);
        }
        printed_normal = true;
    } else {
        PrintInv(leftWin, player->GetConstBlock(), player->PlayerInventory());
    }
    updatedHud = true;
    switch ( player->UsingType() ) {
    default:
        if ( dir <= DOWN ) {
            if ( updatedNormal ) return;
            PrintNormal(rightWin, dir);
            printed_normal = true;
        } else {
            PrintFront(dir);
        }
        break;
    case USAGE_TYPE_READ_IN_INVENTORY:
        DisplayFile( World::WorldPath() + Str("/texts/") +
            player->PlayerInventory()->ShowBlock(
                player->GetUsingInInventory())->GetNote() );
        player->SetUsingTypeNo();
        break;
    case USAGE_TYPE_READ:
        if ( const Block* const focused = GetFocusedBlock() ) {
            DisplayFile(World::WorldPath() + Str("/texts/") +
                focused->GetNote());
            player->SetUsingTypeNo();
        }
        break;
    case USAGE_TYPE_OPEN:
        if ( Block* const focused = GetFocusedBlock() ) {
            PrintInv(rightWin, focused, focused->HasInventory());
        }
        break;
    }
    updatedFront = true;
    if ( printed_normal ) {
        updatedNormal = true;
    }
    if ( player->UsingType() != USAGE_TYPE_OPEN ) {
        if ( player->GetDir() <= DOWN ) {
            (void)wmove(rightWin, player->Y() - GetNormalStartY()  + 1,
                             2 * (player->X() - GetNormalStartX()) + 1);
        } else {
            (void)wmove(rightWin, yCursor, xCursor);
        }
    } else {
        (void)wmove(rightWin, screenHeight-2, screenWidth-2);
    }
    wrefresh(rightWin);
} // void Screen::Print()

void Screen::PrintHud() const {
    if ( updatedHud ) return;
    werase(hudWin);
    if ( player->GetCreativeMode() ) {
        static const QString creativeInfo(
            tr("Creative Mode. xyz: %1-%2-%3.\nShred:\n%4") );
        mvwaddwstr(hudWin, 0, 0, wPrintable(creativeInfo
            .arg(player->GlobalX()).arg(player->GlobalY()).arg(player->Z())
            .arg(Shred::FileName(
                player->GetLongitude(), player->GetLatitude()) )));
    } else {
        const int dur = player->GetConstBlock()->GetDurability();
        if ( dur > 0 ) { // HitPoints line
            static const int player_health_char = ascii ? '@' : 0x2665;
            PrintBar(hudWin, player_health_char,
                int((dur > Block::MAX_DURABILITY/4) ?
                    COLOR_PAIR(RED_BLACK) : COLOR_PAIR(BLACK_RED)) | A_BOLD,
                dur*100/Block::MAX_DURABILITY);
        }
        waddstr(hudWin, "   ");
        static const struct {
            const std::wstring name;
            const int color;
        } satiation[] = {
            { tr("Hungry" ).toStdWString(),   RED_BLACK },
            { tr("Content").toStdWString(), WHITE_BLACK },
            satiation[1],
            { tr("Full"   ).toStdWString(), GREEN_BLACK },
            { tr("Gorged" ).toStdWString(),  BLUE_BLACK }
        };
        const int satiation_state = player->SatiationPercent() / 25;
        wcolor_set(hudWin, satiation[satiation_state].color, nullptr);
        waddwstr  (hudWin, satiation[satiation_state].name.c_str());
        waddch(hudWin, '\n');
        const int breath = player->BreathPercent();
        if ( breath != 100 ) {
            static const int breath_char = ascii ? 'o' : 0x00b0;
            PrintBar(hudWin, breath_char, COLOR_PAIR(BLUE_BLACK), breath);
        }
    }
    const Block* const focused = GetFocusedBlock();
    if ( focused && Block::GetSubGroup(focused->Sub()) != GROUP_AIR ) {
        const int right_border = screenWidth*2;
        (void)wmove(hudWin, 0, right_border - 15);
        PrintBar(hudWin, CharName(focused->Kind(), focused->Sub()),
            Color(focused->Kind(), focused->Sub()),
            focused->GetDurability()*100/Block::MAX_DURABILITY);
        wstandend(hudWin);
        const QString name = focused->FullName();
        mvwaddwstr(hudWin, 1, right_border - name.length(), wPrintable(name));
        const QString note = focused->GetNote();
        if ( Q_UNLIKELY(not note.isEmpty() && IsScreenWide()) ) {
            const int width = qMin(34, note.length());
            mvwaddstr(hudWin, 2, right_border - width - 2, "~:");
            if ( note.length() <= width ) {
                waddwstr (hudWin, wPrintable(note));
            } else {
                waddnwstr(hudWin, wPrintable(note), width - 1 - (ascii * 2));
                waddwstr (hudWin, ellipsis);
            }
        }
    }
    PrintQuickInventory();
    wrefresh(hudWin);
    PrintMiniMap();
} // void Screen::PrintHud()

void Screen::PrintQuickInventory() const {
    const Inventory* const inv = player->GetBlock()->HasInventory();
    if ( inv==nullptr || not IsScreenWide() ) return;

    wstandend(hudWin);
    const int inventory_size = inv->Size();
    int x = screenWidth - inventory_size/2;
    for (int i=0; i<inventory_size; ++i) {
        mvwaddch(hudWin, 0, x, 'a' + i);
        switch ( inv->Number(i) ) {
        case  0: break;
        default: mvwaddch(hudWin, 2, x, inv->Number(i)+'0'); // no break;
        case  1: mvwaddch(hudWin, 1, x, ColoredChar(inv->ShowBlock(i)));
            break;
        }
        x += 2;
        if ( Q_UNLIKELY(i == inv->Start()-1 && i != 0) ) {
            for (int i : {0, 1, 2}) mvwaddch(hudWin, i, x-1, ACS_VLINE);
        }
    }
}

int Screen::GetMinimapStartX() const {
    return Shred::CoordOfShred(player->X()) - 2;
}

int Screen::GetMinimapStartY() const {
    return Shred::CoordOfShred(player->Y()) - 2;
}

void Screen::PrintMiniMap() const {
    if ( updatedMinimap ) return;
    (void)wmove(minimapWin, 1, 0);
    const int i_start = GetMinimapStartY();
    const int j_start = GetMinimapStartX();
    const World* const world = World::GetConstWorld();
    for (int i=i_start; i <= i_start+4; ++i, waddch(minimapWin, '\n'))
    for (int j=j_start; j <= j_start+4; ++j) {
        if ( i>=0 && j>=0 && i<world->NumShreds() && j<world->NumShreds() ) {
            const shred_type t = world->GetShredByPos(j, i)->GetTypeOfShred();
            const int color = ColorShred(t);
            waddch(minimapWin, color | ' ');
            waddch(minimapWin, color | t);
        } else {
            waddch(minimapWin, ' ' | COLOR_PAIR(BLACK_BLACK));
            waddch(minimapWin, ' ' | COLOR_PAIR(BLACK_BLACK));
        }
    }
    DrawBorder(minimapWin);
    static const std::wstring title = tr("Minimap").toStdWString();
    mvwaddwstr(minimapWin, 0, 1, title.c_str());
    wrefresh(minimapWin);
}

int Screen::GetNormalStartX() const {
    return qBound(0, player->X() - screenWidth/4 + 1,
        World::GetBound() - screenWidth/2 - 1);
}

int Screen::GetNormalStartY() const {
    return qBound(0, player->Y() - screenHeight/2 + 1,
        World::GetBound() - screenHeight - 1);
}

void Screen::PrintNormal(WINDOW* const window, const dirs dir) const {
    const int k_step  = ( dir == UP ) ? 1 : -1;
    const int k_start = player->Z() + shiftFocus;
    const int start_x = GetNormalStartX();
    const int start_y = GetNormalStartY();
    const int end_x = start_x + screenWidth/2 - 1;
    const int end_y = start_y + screenHeight  - 2;
    const World* const world = World::GetConstWorld();
    (void)wmove(window, 1, 1);
    for (int j=start_y; j<end_y; ++j, waddch(window, 30)) {
        const int j_in = Shred::CoordInShred(j);
        for (int i=start_x; i<end_x; ++i) {
            int k = k_start;
            const Block* const block = world->GetShred(i, j)->
                FindFirstVisible(Shred::CoordInShred(i), j_in, &k, k_step);
            switch ( player->Visible(i, j, k) ) {
            case Player::VISIBLE:
                PrintBlock(block, window, CharNumber(k));
                break;
            case Player::IN_SHADOW:
                PrintShadow(window);
                break;
            case Player::OBSCURED:
                waddch(window, ' ' | COLOR_PAIR(BLACK_BLACK));
                waddch(window, ' ' | COLOR_PAIR(BLACK_BLACK));
                break;
            }
        }
    }

    if ( dir > DOWN ) {
        const Block* const block = player->GetConstBlock();
        mvwaddch(window, player->Y()-start_y+1, (player->X()-start_x)*2+2,
            arrows[player->GetDir()] | Color(block->Kind(), block->Sub()));
    }

    DrawBorder(window);
    Arrows(window, (player->X()-start_x)*2+1, player->Y()-start_y+1,
        NORTH, true);
    if ( shiftFocus ) {
        const int ch = ( ',' - shiftFocus ) | COLOR_PAIR(WHITE_BLUE); // +/-
        mvwaddch(window, 0,              0,  ch);
        mvwaddch(window, 0,  screenWidth-1,  ch);
        mvwaddch(window, screenHeight-1, 0,  ch);
        mvwaddch(window, screenHeight-1, screenWidth-1, ch);
    }
    wrefresh(window);
} // void Screen::PrintNormal(WINDOW* window, int dir)

void Screen::DrawBorder(WINDOW* const window) {
    static const int BORDER_COLOR = COLOR_PAIR(BLACK_BLACK) | A_BOLD;
    (void)wborder( window,
        ACS_VLINE    | BORDER_COLOR, ACS_VLINE    | BORDER_COLOR,
        ACS_HLINE    | BORDER_COLOR, ACS_HLINE    | BORDER_COLOR,
        ACS_ULCORNER | BORDER_COLOR, ACS_URCORNER | BORDER_COLOR,
        ACS_LLCORNER | BORDER_COLOR, ACS_LRCORNER | BORDER_COLOR );
}

int Screen::GetFrontStartZ() const {
    return qBound(screenHeight-3, player->Z()+screenHeight/2, HEIGHT-1);
}

void Screen::PrintFront(const dirs dir, const int block_x, const int block_y)
const {
    if ( updatedFront && block_x <= 0 ) return;
    int x_step,  z_step,
        x_start, z_start,
        x_end,   z_end;
    int* x, * z;
    int i, j;
    switch ( dir ) {
    case NORTH:
        x = &i;
        x_step  = 1;
        x_start = GetNormalStartX();
        x_end   = x_start + screenWidth/2 - 1;
        z = &j;
        z_step  = -1;
        z_start = player->Y() - 1;
        z_end   = qMax(0, player->Y() - FRONT_MAX_DISTANCE);
        xCursor = (player->X() - x_start)*2 + 1;
        break;
    case EAST:
        x = &j;
        x_step  = 1;
        x_start = qBound(0, player->Y() - screenWidth/4 - 1,
            World::GetBound() - screenWidth/2 - 1);
        x_end   = screenWidth/2 - 1 + x_start;
        z = &i;
        z_step  = 1;
        z_start = player->X() + 1;
        z_end   = qMin(player->X() + FRONT_MAX_DISTANCE, World::GetBound());
        xCursor = (player->Y() - x_start)*2 + 1;
        break;
    case SOUTH:
        x = &i;
        x_step  = -1;
        x_end   = GetNormalStartX() - 1;
        x_start = screenWidth/2 - 1 + x_end;
        z = &j;
        z_step  = 1;
        z_start = player->Y() + 1;
        z_end   = qMin(player->Y() + FRONT_MAX_DISTANCE, World::GetBound());
        xCursor = (screenWidth/2 - player->X() + x_end)*2 - 1;
        break;
    case WEST:
        x = &j;
        x_step  = -1;
        x_end   = qBound(0, player->Y() - screenWidth/4 - 1,
            World::GetBound() - screenWidth/2 - 1) - 1;
        x_start = screenWidth/2 - 1 + x_end;
        z = &i;
        z_step  = -1;
        z_start = player->X() - 1;
        z_end   = qMax(0, player->X() - FRONT_MAX_DISTANCE);
        xCursor = (screenWidth/2 - player->Y() + x_end)*2 - 1;
        break;
    default: Q_UNREACHABLE(); return;
    }
    const World* const world = World::GetConstWorld();
    const int k_start = GetFrontStartZ();
    if ( Q_UNLIKELY(block_x > 0) ) {
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
    (void)wmove(rightWin, 1, 1);
    for (int k=k_start; k>k_end; --k, waddch(rightWin, 30)) {
        for (*x=x_start; *x!=x_end; *x+=x_step) {
            const Block* block;
            for (*z=z_start; *z!=z_end && (block = world->GetBlock(i, j, k))->
                        Transparent()==INVISIBLE;
                    *z += z_step);
            if ( *z != z_end ) {
                switch ( player->Visible(i, j, k) ) {
                case Player::VISIBLE:
                    PrintBlock(block, rightWin, CharNumberFront(i, j));
                    break;
                case Player::IN_SHADOW:
                    PrintShadow(rightWin);
                    break;
                case Player::OBSCURED:
                    waddch(rightWin, ' ' | COLOR_PAIR(BLACK_BLACK));
                    waddch(rightWin, ' ' | COLOR_PAIR(BLACK_BLACK));
                    break;
                }
            } else {
                PrintShadow(rightWin);
            }
        }
    }
    DrawBorder(rightWin);
    yCursor = k_start + 1 - player->Z();
    Arrows(rightWin, xCursor, yCursor, dir, false);
    if ( Q_UNLIKELY(shiftFocus) ) {
        const int y = yCursor - shiftFocus;
        mvwaddch(rightWin, y,             0, '>' | COLOR_PAIR(WHITE_BLUE));
        mvwaddch(rightWin, y, screenWidth-1, '<' | COLOR_PAIR(WHITE_BLUE));
    }
    wrefresh(rightWin);
} // void Screen::PrintFront(dirs)

void Screen::PrintInv(WINDOW* const window,
        const Block* const block, const Inventory* const inv)
const {
    if ( Q_UNLIKELY(inv == nullptr) ) {
        if ( player->UsingType() == USAGE_TYPE_OPEN ) {
            player->SetUsingTypeNo();
        }
        return;
    }
    if ( inv == player->PlayerInventory() ) {
        if ( updatedHud ) return;
    } else {
        if ( updatedFront ) return;
    }
    werase(window);
    const int start = inv->Start();
    int shift = 0; // to divide inventory sections
    for (int i=0; i<inv->Size(); ++i) {
        shift += ( start == i && i != 0 );
        wstandend(window);
        mvwprintw(window, 1+i+shift, 1, "%c) ", 'a'+i);
        const Block* const block = inv->ShowBlock(i);
        if ( block == nullptr ) {
            wattrset(window, COLOR_PAIR(BLACK_BLACK) | A_BOLD);
            waddstr (window, "   ");
            waddwstr(window, wPrintable(inv->InvFullName(i)));
            continue;
        }
        PrintBlock(block, window, ' ');
        wstandend(window);
        waddch(window, ' ');
        waddwstr(window, wPrintable(inv->InvFullName(i)));
        if ( Block::MAX_DURABILITY != block->GetDurability() ) {
            wprintw(window, "{%d}",
                block->GetDurability() * 100 / Block::MAX_DURABILITY);
        }
        const QString str = block->GetNote();
        if ( Q_UNLIKELY(not str.isEmpty()) ) {
            const int width = screenWidth - getcurx(window) - 3 - 8;
            waddstr(window, " ~:");
            if ( str.length() <= width ) {
                waddwstr (window, wPrintable(str));
            } else {
                waddnwstr(window, wPrintable(str), width - 2 - ( ascii * 2 ) );
                waddwstr (window, ellipsis);
            }
        }
        mvwprintw(window, 1 + i + shift, screenWidth - 9, "%5hu mz",
            inv->GetInvWeight(i));
    }
    wattrset(window, Color(block->Kind(), block->Sub()));
    box(window, 0, 0);
    if ( start != 0 ) {
        mvwaddch(window, start+1, 0, ACS_LTEE);
        whline(window, ACS_HLINE, screenWidth-2);
        mvwaddch(window, start+1, screenWidth-1, ACS_RTEE);
    }
    mvwprintw(window, 0, 1, "[%c] ", CharName( block->Kind(), block->Sub()));
    waddwstr(window, wPrintable( (player->PlayerInventory() == inv) ?
        tr("Your inventory") :
        block->FullName() + Str(". ") + block->Description() ));
    const QString full_weight = tr("Full weight: %1 mz").arg(inv->Weight());
    mvwaddwstr(window, screenHeight-1, screenWidth-1-full_weight.length(),
        wPrintable(full_weight) );
    wrefresh(window);
} // void Screen::PrintInv(WINDOW*, const Block*, const Inventory*)

void Screen::DisplayFile(const QString path) {
    { QFile(path).open(QIODevice::ReadWrite); } // create file if doesn't exist
    QDesktopServices::openUrl(QUrl(Str("file:///") + path));
    Notify(tr("Open ") + path);
}

void Screen::Notify(const QString str) const {
    if ( str.isEmpty() ) return;
    Log(str);
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
        mvwaddwstr(notifyWin, 6, 0, wPrintable(str));
        wprintw(notifyWin, " (x%d)", notification_repeat_count);
    } else {
        notification_repeat_count = 1;
        scroll(notifyWin);
        mvwaddwstr(notifyWin, 6, 0, wPrintable(lastNotification = str));
    }
    wrefresh(notifyWin);
}

void Screen::DeathScreen() {
    werase(windows[WIN_ACTION]);
    std::for_each(windows + WIN_HUD, windows + WIN_COUNT, werase);
    wattrset(leftWin,  COLOR_PAIR(WHITE_RED) | A_BOLD);
    wattrset(rightWin, COLOR_PAIR(WHITE_RED) | A_BOLD);
    Notify(tr("Waiting for respawn..."));
    box( leftWin, 'X', 'X');
    box(rightWin, 'X', 'X');
    wnoutrefresh(windows[WIN_ACTION]);
    std::for_each(windows + WIN_HUD, windows + WIN_COUNT, wnoutrefresh);
    doupdate();
    updatedNormal = updatedFront = updatedHud = updatedMinimap = true;
}

void Screen::initializeKeyTable() {
    std::fill(ALL(keyTable), [](const int key) {
        GetScreen()->Notify(tr("Unknown key. Press 'H' for help."));
        if ( DEBUG ) {
            GetScreen()->Notify(Str("'%1', code %2.").arg(char(key)).arg(key));
        }
    });

    std::fill(keyTable + 'a', keyTable + 'z' + 1, [](const int key) {
        GetScreen()->InventoryAction(key - 'a');
    });

    const struct {
        std::initializer_list<char> keys;
        void(* command)(int);
    } command_table[] {
        // moving
        {{'W', '8'}, [](int) { GetScreen()->MovePlayer(NORTH); }},
        {{'S', '2'}, [](int) { GetScreen()->MovePlayer(SOUTH); }},
        {{'D', '6'}, [](int) { GetScreen()->MovePlayer(EAST ); }},
        {{'A', '4'}, [](int) { GetScreen()->MovePlayer(WEST ); }},

        {{'5'},      [](int) { GetScreen()->player->Move(DOWN); }},

        {{'7'},      [](int) { GetScreen()->MovePlayerDiag(NORTH, WEST); }},
        {{'9'},      [](int) { GetScreen()->MovePlayerDiag(NORTH, EAST); }},
        {{'1'},      [](int) { GetScreen()->MovePlayerDiag(SOUTH, WEST); }},
        {{'3'},      [](int) { GetScreen()->MovePlayerDiag(SOUTH, EAST); }},

        {{'=', '0'}, [](int) { Screen* const screen = GetScreen();
            screen->MovePlayer(screen->player->GetDir());
        }},
        {{' '},      [](int) { GetScreen()->player->Jump(); }},

        // strafe
        {{'{'},      [](int) { Player* const player = GetScreen()->player;
            player->Move(World::TurnLeft(player->GetDir()));
        }},
        {{'}'},      [](int) { Player* const player = GetScreen()->player;
            player->Move(World::TurnRight(player->GetDir()));
        }},

        // turning
        {{'>'},      [](int) { Player* const player = GetScreen()->player;
            player->SetDir(World::TurnRight(player->GetDir()));
        }},
        {{'<'},      [](int) { Player* const player = GetScreen()->player;
            player->SetDir(World::TurnLeft(player->GetDir()));
        }},
        {{'K'},      [](int) { Player* const player = GetScreen()->player;
            player->SetDir(World::Anti(player->GetDir()));
        }},

        {{'V'},      [](int) { GetScreen()->player->SetDir(DOWN); }},
        {{'^'},      [](int) { GetScreen()->player->SetDir(UP);   }},

        {{'I'},      [](int) { GetScreen()->player->Backpack(); }},

        {{8},        [](int) { GetScreen()->player->Damage(); }},

        {{'F'},      [](int) { GetScreen()->player->Use(); }},
        {{'?'},      [](int) { GetScreen()->player->Examine(); }},
        {{'~'},      [](int) { GetScreen()->player->Inscribe(); }},
        {{27},       [](int) { GetScreen()->player->StopUseAll(); }}, // esc

        {{'B'},      [](int) { GetScreen()->SetActionMode(ACTION_BUILD); }},
        {{'C'},      [](int) { GetScreen()->SetActionMode(ACTION_CRAFT); }},
        {{'T'},      [](int) { GetScreen()->SetActionMode(ACTION_THROW); }},
        {{'N'},      [](int) { GetScreen()->SetActionMode(ACTION_INSCRIBE); }},
        {{'G', 'O'}, [](int) { GetScreen()->SetActionMode(ACTION_OBTAIN); }},
        {{'E'},      [](int) { GetScreen()->SetActionMode(ACTION_WIELD); }},
        {{'U'},      [](int) { GetScreen()->SetActionMode(ACTION_USE); }},

        {{'H'},      [](int) { GetScreen()->ProcessCommand(Str("help")); }},

        {{'R'},      [](int) { GetScreen()->RePrint(); }},

        {{'!'},      [](int) { Player* const player = GetScreen()->player;
            player->SetCreativeMode( not player->GetCreativeMode() );
        }},

        {{'L'},      [](int) {
            QDesktopServices::openUrl(
                QUrl(Str("file:///%1log.txt").arg(home_path)));
        }},

        {{'['},      [](int) { Screen* const screen = GetScreen();
            screen->SetActionMode((screen->actionMode == ACTION_USE) ?
                ACTION_WIELD : static_cast<actions>(screen->actionMode - 1));
        }},
        {{']', '\t'}, [](int) { Screen* const screen = GetScreen();
            screen->SetActionMode(screen->actionMode == ACTION_WIELD ?
                ACTION_USE : static_cast<actions>(screen->actionMode + 1));
        }},

        {{'Z'},      [](int) { Screen* const screen = GetScreen();
            if ( screen->player->PlayerInventory() ) {
                  screen->player->PlayerInventory()->Shake();
                  screen->Notify(tr("Inventory reorganized."));
            }
        }},

        {{13, '\n'}, [](int) { Screen* const screen = GetScreen();
            switch ( screen->actionMode ) {
            case ACTION_USE:      screen->player->Use();              break;
            case ACTION_THROW:    screen->player->Throw(ACTIVE_HAND); break;
            case ACTION_OBTAIN:   screen->player->Obtain(0);          break;
            case ACTION_INSCRIBE: screen->player->Inscribe();         break;
            case ACTION_BUILD:    screen->player->Build(ACTIVE_HAND); break;
            case ACTION_CRAFT:    screen->player->Craft(ACTIVE_HAND); break;
            case ACTION_WIELD:    screen->player->Wield(ACTIVE_HAND); break;
            }
        }},

        {{'\\', ':', '/'}, [](int) { Screen* const screen = GetScreen();
            screen->PassString(screen->previousCommand);
            screen->ProcessCommand(screen->previousCommand);
        }},
        {{'.'}, [](int) { Screen* const screen = GetScreen();
            screen->ProcessCommand(screen->previousCommand);
        }},

        {{'-'}, [](int) { Screen* const screen = GetScreen();
            screen->shiftFocus -= 2;
            screen->keyTable[int('+')](0);
        }},
        {{'+'}, [](int) { Screen* const screen = GetScreen();
            ++(screen->shiftFocus);
            if ( abs(screen->shiftFocus) == 2 ) {
                screen->shiftFocus = 0;
            }
            static const QString levels[] = {
                tr("low"),
                tr("normal"),
                tr("high")
            };
            screen->Notify( tr("Focus is set: %1.").
                arg(levels[screen->shiftFocus + 1]) );
        }},

        {{'M'}, [](int) { Screen* const scr = GetScreen();
            scr->mouseOn = not scr->mouseOn;
            mousemask((scr->mouseOn ? MOUSEMASK : scr->noMouseMask), nullptr);
            scr->Notify(tr("Mouse: %1.").arg(TrManager::OffOn(scr->mouseOn)));
        }},

        {{'P'}, [](int) { Screen* const screen = GetScreen();
            static bool isPaused = false;
            if ( isPaused ) {
                emit screen->ResumeWorld();
                screen->Notify(Str("Game is resumed."));
            } else {
                emit screen->PauseWorld();
                screen->Notify(Str("Game is paused."));
            }
            isPaused = not isPaused;
        }},

        {{'Y'}, [](int) { Screen* const screen = GetScreen();
            screen->Notify(tr("Saving game..."));
            World::GetWorld()->SaveToDisk();
            screen->player->SaveState();
            screen->Notify( tr("Game saved at location \"%1\".").
                arg(World::WorldPath()) );
        }},

        {{'Q', 3, 4, 17, 24, 'X'}, [](int){ Screen* const screen = GetScreen();
            screen->Notify(tr("Exiting game...\n\n"));
            emit screen->ExitReceived();
            screen->input->Stop();
        }}
    };
    for (const auto& keys_command : command_table ) {
        for (int c : keys_command.keys) {
            keyTable[c] = keys_command.command;
        }
    }
} // void Screen::initializeKeyTable()

Screen* Screen::GetScreen() { return staticScreen; }

Screen::Screen(Player* const pl, int&) :
        VirtScreen(pl),
        screen(newterm(nullptr, stdout, stdin)),
        screenWidth((COLS / 2) - ((COLS/2) & 1)),
        screenHeight(LINES - 10),
        windows {
            newwin(7, ACTIONS_WIDTH, LINES-7, MINIMAP_WIDTH + 1), // actions
            newwin(7, 0, LINES-7, MINIMAP_WIDTH+ACTIONS_WIDTH+2), // notify
            newwin(3, 0, screenHeight, 0),                        // HUD
            newwin(MINIMAP_HEIGHT, MINIMAP_WIDTH, LINES-7, 0),    // minimap
            newwin(screenHeight, screenWidth, 0, (COLS/2) & 1),   // left
            newwin(screenHeight, screenWidth, 0, COLS/2)          // right
        },
        lastNotification(),
        input(new IThread(this)),
        updatedHud    (false),
        updatedMinimap(false),
        updatedNormal (false),
        updatedFront  (false),
        actionMode(static_cast<actions>(settings.value(
            Str("action_mode"), ACTION_USE).toInt())),
        shiftFocus(settings.value(Str("focus_shift"), 0).toInt()),
        OPTIONS_TABLE(OPTIONS_INIT)
        showCharDistance(settings.value(Str("show_char_distance"),10).toInt()),
        ellipsis{
            ascii ? L'.' : L'\U00002026',
            ascii ? L'.' : L'\00',
            ellipsis[1]
        },
        noMouseMask(),
        xCursor(),
        yCursor(),
        keyTable()
{
    Q_ASSERT(staticScreen == nullptr);
    staticScreen = this;

    start_color();
    halfdelay(10);
    noecho(); // do not print typed symbols
    nonl();
    keypad(stdscr, true); // use arrows

    mousemask(MOUSEMASK, &noMouseMask); // store old mouse mask in noMouseMask
    if ( not mouseOn ) { // if mouse is off, turn it off.
        mousemask(noMouseMask, nullptr);
    }

    // initiate all possible color pairs
    const int colors[] = { COLOR_LIST(COLOR) };
    for (int i=BLACK_BLACK; i<=WHITE_WHITE; ++i) {
        init_pair(i, colors[(i-1)/8], colors[(i-1) % 8]);
    }

    scrollok(notifyWin, true);
    (void)wmove(notifyWin, 6, 0);

    connect(World::GetWorld(), &World::UpdatesEnded, this, &Screen::Print,
        Qt::DirectConnection);

    ungetch('0');
    getch();
    Notify(tr("\t[[F][r][e][g]] version %1").arg(VER));
    Notify(tr("Copyright (C) 2012-2015 Alexander 'm8f' Kromm"));
    Notify(Str("(mmaulwurff@gmail.com)\n"));
    Notify(tr("Press Space to continue."));

    while (getch() != ' ');

    RePrint();
    Notify(tr("--- Game started. Press 'H' for help. ---"));

    initializeKeyTable();
    input->start();
} // Screen::Screen(Player* const pl, int& error)

Screen::~Screen() {
    World* const world = World::GetWorld();
    world->GetLock()->lock();
    disconnect(world, &World::UpdatesEnded, this, &Screen::Print);
    world->GetLock()->unlock();

    input->Stop();
    input->wait();
    delete input;

    std::for_each(ALL(windows), delwin);
    endwin();
    delscreen(screen);
    settings.setValue(Str("focus_shift" ), shiftFocus);
    settings.setValue(Str("action_mode" ), actionMode);
    settings.setValue(Str("last_command"), previousCommand);
    settings.setValue(Str("show_char_distance"), showCharDistance);
    OPTIONS_TABLE(OPTIONS_SAVE);
}

void Screen::PrintBar(WINDOW* const window,
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

void Screen::Palette(WINDOW* const window) {
    wclear(window);
    const struct {
        chtype attribute;
        std::string name;
    } lines[] {
        {A_NORMAL,    "A_NORMAL    "},
        {A_BOLD,      "A_BOLD      "},
        {A_BLINK,     "A_BLINK     "},
        {A_REVERSE,   "A_REVERSE   "},
        {A_STANDOUT,  "A_STANDOUT  "},
    }, words[] {
        {A_UNDERLINE, "A_UNDERLINE "},
        {A_DIM,       "A_DIM       "},
    };
    for (const auto& type : lines) {
        wstandend(window);
        waddstr(window, type.name.c_str());
        wattrset(window, type.attribute);
        for (int i=BLACK_BLACK; i<=WHITE_WHITE; ++i) {
            wcolor_set(window, i, nullptr);
            waddch(window, ACS_DIAMOND);
        }
        waddch(window, '\n');
    }
    for (const auto& type : words) {
        wattrset(window, type.attribute);
        waddstr(window, type.name.c_str());
    }
    wrefresh(window);
}

void Screen::TestNotify() const {
    Notify(tr("1. Translated line"));
    Notify(Str("2. Notification window should be able to show"));
    Notify(Str("3. at least 7 lines."));
    Notify(Str("4. Line four."));
    Notify(Str("5. Line five."));
    Notify(Str("6. Line six."));
    Notify(Str("7. Line seven."));
}
