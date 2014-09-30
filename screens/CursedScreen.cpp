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
const int SHADOW_COLOR = COLOR_PAIR(BLACK_BLACK) | A_BOLD | A_REVERSE;

const int MINIMAP_WIDTH = 11;
const int MINIMAP_HEIGHT = 7;

void Screen::PrintVerticalDirection(WINDOW * const window, const int y,
        const int x, const dirs direction)
{
    mvwaddstr(window, y, x + 3, qPrintable(Block::DirString(direction)));
}

void Screen::Arrows(WINDOW * const window, const int x, const int y,
        const dirs direction)
const {
    wcolor_set(window, WHITE_BLACK, nullptr);
    if ( direction >= DOWN ) {
        PrintVerticalDirection(window, 0, x, UP);
        PrintVerticalDirection(window, SCREEN_SIZE+1, x, DOWN);
    } else {
        PrintVerticalDirection(window, 0, x, NORTH);
        PrintVerticalDirection(window, SCREEN_SIZE+1, x, SOUTH);
    }
    wcolor_set(window, WHITE_RED, nullptr);
    mvwaddstr(window, 0, x, qPrintable(arrows[SOUTH]));
    waddstr  (window,       qPrintable(arrows[SOUTH]));
    mvwaddstr(window, SCREEN_SIZE+1, x, qPrintable(arrows[NORTH]));
    waddstr  (window, qPrintable(arrows[NORTH]));
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
        Block::DirString(NORTH).left(1),
        Block::DirString(SOUTH).left(1),
        Block::DirString(EAST ).left(1),
        Block::DirString(WEST ).left(1)
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
    mvwaddstr(window, y-1, 0, qPrintable(left));
    mvwaddstr(window, y-1, SCREEN_SIZE*2+1, qPrintable(right));

    wcolor_set(window, WHITE_RED, nullptr);
    mvwaddstr(window, y, 0, qPrintable(arrows[EAST]));
    mvwaddstr(window, y, SCREEN_SIZE*2+1, qPrintable(arrows[WEST]));
}

void Screen::RePrint() {
    clear();
    static const QString action_strings[] = {
        tr("[U] use, eat"),
        tr("[T] throw"),
        tr("[O] obtain"),
        tr("[N] inscribe"),
        tr("[B] build"),
        tr("[C] craft"),
        tr("[E] equipment"),
    };
    (void)wmove(actionWin, 0, 0);
    for (int i=0; i<=ACTION_WIELD; ++i) {
        waddstr(actionWin, qPrintable(action_strings[i]));
        waddch(actionWin, '\n');
    }
    mvwchgat(actionWin, actionMode, 0, 20, A_NORMAL, BLACK_WHITE, nullptr);
    refresh();
    wrefresh(actionWin);
    updated = false;
}

void Screen::Update(int, int, int) { updated = false; }
void Screen::UpdatePlayer() { updated = false; }
void Screen::UpdateAround(int, int, int, int) { updated = false; }
void Screen::Move(int) { updated = false; }
bool Screen::IsScreenWide() { return COLS >= (SCREEN_SIZE*2+2)*2; }

void Screen::UpdateAll() {
    CleanFileToShow();
    updated = false;
}

void Screen::PassString(QString & str) const {
    inputActive = true;
    wattrset(notifyWin, A_UNDERLINE);
    waddstr(notifyWin, qPrintable(tr("Enter input: ")));
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

int  Screen::RandomBlink() const { return RandomBit() ? 0 : A_REVERSE; }
bool Screen::RandomBit()   const { return (randomBlink >>= 1) & 1; }

int Screen::Color(const int kind, const int sub) const {
    const int color = COLOR_PAIR(VirtScreen::Color(kind, sub));
    switch ( kind ) { // foreground_background
    case TELEGRAPH: return color | A_BOLD;
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
            if ( w->GetEvernight() ) return COLOR_PAIR(BLACK_BLACK);
            switch ( w->PartOfDay() ) {
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
    CleanFileToShow();
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
    case '\n':
    case 13:
    case KEY_F(2):
    case 'F': player->Use();      break;
    case '*':
    case KEY_F(3):
    case '?': player->Examine();  break;
    case KEY_F(4):
    case '~': player->Inscribe(); break;
    case 27: /* esc */ player->StopUseAll(); break;

    case KEY_IC: player->Build(1); break; // insert
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

    case '-':
        shiftFocus = -!shiftFocus;
        Notify(tr("%1 focus is set.").
            arg(shiftFocus ? tr("Low") : tr("Normal")));
        break; // move focus down
    case '+':
        shiftFocus =  !shiftFocus;
        Notify(tr("%1 focus is set.").
            arg(shiftFocus ? tr("High") : tr("Normal")));
        break;

    case KEY_F(12):
    case '!': player->SetCreativeMode( not player->GetCreativeMode() ); break;
    case KEY_F(9):
    case '\\':
    case ':':
    case '/': PassString(previousCommand); // no break
    case '.': ProcessCommand(previousCommand); break;

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
    switch ( mevent.bstate ) {
    case BUTTON4_CLICKED: ControlPlayer('['); return;
    case BUTTON5_CLICKED: ControlPlayer(']'); return;
    case BUTTON1_CLICKED: break;
    default: return;
    }
    if ( wenclose(leftWin, mevent.y, mevent.x) ) { // left window
        if ( not wmouse_trafo(leftWin, &mevent.y, &mevent.x, false) ) return;
        if ( not (
                0 < mevent.x && mevent.x < SCREEN_SIZE*2 + 1 &&
                0 < mevent.y && mevent.y < SCREEN_SIZE ) )
        {
            Notify(tr("Left window, Down view."));
            return;
        }
        ExamineOnNormalScreen(mevent.x, mevent.y, player->Z(), -1);
    } else if ( wenclose(notifyWin, mevent.y, mevent.x) ) { // notify
        Notify(tr("Notifications area."));
    } else if ( wenclose(actionWin, mevent.y, mevent.x) ) { // actions
        wmouse_trafo(actionWin, &mevent.y, &mevent.x, false);
        SetActionMode(static_cast<actions>(mevent.y));
    } else if ( wenclose(minimapWin, mevent.y, mevent.x) ) { // minimap
        if (not wmouse_trafo(minimapWin, &mevent.y, &mevent.x, false)) return;
        if ( not (
                0 < mevent.x && mevent.x < MINIMAP_WIDTH-1 &&
                0 < mevent.y && mevent.y < MINIMAP_HEIGHT-1 ) )
        {
            Notify(tr("Minimap."));
            return;
        }
        const int shred_x = mevent.x/2 + GetMinimapStartX();
        const int shred_y = mevent.y-1 + GetMinimapStartY();
        Notify((0 <= shred_x && shred_x < w->NumShreds() &&
                0 <= shred_y && shred_y < w->NumShreds() ) ?
            tr("On minimap: %1").arg( Shred::ShredTypeName(
                w->GetShredByPos(shred_x, shred_y)->GetTypeOfShred())) :
            tr("You can't see that far.") );
    } else if ( wenclose(hudWin, mevent.y, mevent.x) ) { // HUD
        if ( not wmouse_trafo(hudWin, &mevent.y, &mevent.x, false) ) return;
        mevent.x -= QUICK_INVENTORY_X_SHIFT;
        mevent.x /= 2;
        if ( not ( IsScreenWide() && 0 <= mevent.x && mevent.x <= 'z'-'a' ) ) {
            Notify(tr("Information: left - player, right - focused thing."));
            return;
        }
        Inventory * const inv = player->PlayerInventory();
        if ( inv == nullptr ) return;
        Notify( tr("In inventory at slot '%1': %2.").
            arg(char(mevent.x + 'a')).
            arg( inv->Number(mevent.x) ?
                inv->InvFullName(mevent.x) :
                tr("nothing") ) );
    } else if ( wenclose(rightWin, mevent.y, mevent.x) ) { // right window
        if ( not wmouse_trafo(rightWin, &mevent.y, &mevent.x, false) ) return;
        if ( not (
                0 < mevent.x && mevent.x < SCREEN_SIZE*2 + 1 &&
                0 < mevent.y && mevent.y < SCREEN_SIZE ) )
        {
            Notify(tr("Right window, %1 view.").
                arg(Block::DirString(player->GetDir())));
            return;
        }
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
    } else {
        Notify(tr("Nothing here. Click on something to get information."));
    }
}

void Screen::ProcessCommand(const QString command) {
    if ( command.length()==1 && command.at(0)!='.' ) {
        ControlPlayer(command.at(0).toLatin1());
        return;
    }
    switch ( Player::UniqueIntFromString(qPrintable(command)) ) {
    case Player::UniqueIntFromString("blink"):
        blinkOn = not blinkOn;
        Notify(tr("Block blink is now %1.").
            arg(blinkOn ? tr("on") : tr("off")));
        break;
    case Player::UniqueIntFromString("size"):
        Notify(tr("Terminal height: %1 lines, width: %2 chars.").
            arg(LINES).arg(COLS));
        break;
    case Player::UniqueIntFromString("moo"):
        Notify("^__^");
        Notify("(oo)\\_______");
        Notify("(__)\\       )\\/\\");
        Notify("    ||----w |");
        Notify("    ||     ||");
        break;
    default: player->ProcessCommand(command); break;
    }
}

void Screen::SetActionMode(const actions mode) {
    mvwchgat(actionWin, actionMode,      0,20, A_NORMAL, WHITE_BLACK, nullptr);
    mvwchgat(actionWin, actionMode=mode, 0,20, A_NORMAL, BLACK_WHITE, nullptr);
    switch ( mode ) {
    case ACTION_USE:      Notify(tr("Action: use in inventory."));      break;
    case ACTION_THROW:    Notify(tr("Action: throw from inventory."));  break;
    case ACTION_OBTAIN:   Notify(tr("Action: obtain to inventory."));   break;
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
            ( AIR==w->GetBlock(*x, *y, *z)->Sub() || AIR==w->GetBlock(
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
    return GetWorld()->GetBlock(x, y, z);
}

char Screen::PrintBlock(const Block & block, WINDOW * const window) const {
    const int kind = block.Kind();
    const int sub  = block.Sub();
    wattrset(window, Color(kind, sub));
    return CharName(kind, sub);
}

void Screen::Print() {
    if ( updated ) return;
    updated = true;
    w->Lock();
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
            DisplayFile(QString(home_path + w->WorldName() + "/texts/"
                + player->PlayerInventory()->ShowBlock(
                    player->GetUsingInInventory())->GetNote()));
            player->SetUsingTypeNo();
            break;
        case USAGE_TYPE_READ:
            DisplayFile(QString(home_path + w->WorldName()
                + "/texts/" + GetFocusedBlock()->GetNote()));
            player->SetUsingTypeNo();
            break;
        case USAGE_TYPE_OPEN: {
                Block * const block = GetFocusedBlock();
                PrintInv(rightWin, block, block->HasInventory());
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
        const int dur = player->GetBlock()->GetDurability();
        if ( dur > 0 ) { // HitPoints line
            static const int player_health_char =  ascii ? '@' : 0x2665;
            PrintBar(0,
                (dur > MAX_DURABILITY/5) ?
                    COLOR_PAIR(RED_BLACK) : (COLOR_PAIR(BLACK_RED) | A_BLINK),
                player_health_char, dur*100/MAX_DURABILITY);
        }
        const int breath = player->BreathPercent();
        if ( breath != 100 ) {
            static const int player_breath_char = ascii ? 'o' : 0x00b0;
            PrintBar(16, COLOR_PAIR(BLUE_BLACK), player_breath_char, breath);
        }
        static const QString satiation_strings[] = {
            tr("Hungry"),
            tr("Content"),
            satiation_strings[1],
            tr("Full"),
            tr("Gorged")
        };
        static const int satiation_colors[] = {
            RED_BLACK,
            WHITE_BLACK,
            satiation_colors[1],
            GREEN_BLACK,
            BLUE_BLACK
        };
        const int satiation_state = player->SatiationPercent() / 25;
        wcolor_set(hudWin, satiation_colors[satiation_state], nullptr);
        mvwaddstr(hudWin, 1,1, qPrintable(satiation_strings[satiation_state]));
    }
    Block * const focused = GetFocusedBlock();
    if ( Block::GetSubGroup(focused->Sub()) != GROUP_AIR ) {
        const int left_border = IsScreenWide() ?
            (SCREEN_SIZE*2+2) * 2 :
            SCREEN_SIZE*2+2 - 15;
        PrintBar(left_border - 15,
            Color(focused->Kind(), focused->Sub()),
            (focused->IsAnimal() == nullptr) ? '+' : '*',
            focused->GetDurability()*100/MAX_DURABILITY,
            false);
        const QString name = focused->FullName();
        mvwaddstr(hudWin, 1, left_border-name.length() - 1,
            qPrintable(focused->FullName()));
        const QString note = focused->GetNote();
        if ( not note.isEmpty() && IsScreenWide() ) {
            const int width = qMin(36, note.length() + 2);
            (void)wmove(hudWin, 2, left_border - width - 1);
            if ( note.length()+2 <= width ) {
                wprintw(hudWin, "~:%s", qPrintable(note));
            } else {
                wprintw(hudWin, "~:%s ...", qPrintable(note.left(width - 6)));
            }
        }
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
        if ( i<0 || j<0 || i>=w->NumShreds() || j>=w->NumShreds() ) {
            wstandend(minimapWin);
            waddstr  (minimapWin, "  ");
        } else {
            Shred * const shred = w->GetShredByPos(j, i);
            wattrset(minimapWin, ColorShred(shred->GetTypeOfShred()));
            wprintw (minimapWin, " %c", shred->GetTypeOfShred());
        }
    }
    wstandend(minimapWin);
    box(minimapWin, 0, 0);
    mvwaddstr(minimapWin, 0, 1, qPrintable(tr("Minimap")));
    wrefresh(minimapWin);
}

int Screen::GetNormalStartX() const {
    return ( player->X()/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
}

int Screen::GetNormalStartY() const {
    return ( player->Y()/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
}

void Screen::PrintNormal(WINDOW * const window, const dirs dir) const {
    (void)wmove(window, 1, 1);
    int k_start, k_step;
    if ( UP == dir ) {
        k_start = player->Z() + 1;
        k_step  = 1;
    } else {
        k_start = player->Z() - ( DOWN==dir );
        k_step  = -1;
    }
    const int start_x = GetNormalStartX();
    const int start_y = GetNormalStartY();
    const int end_x = start_x + SCREEN_SIZE;
    const int end_y = start_y + SCREEN_SIZE;
    for (int j=start_y; j<end_y; ++j, waddstr(window, "\n_")) {
        randomBlink = blinkOn ? qrand() : 0;
        for (int i=start_x; i<end_x; ++i ) {
            Shred * const shred = w->GetShred(i, j);
            const int i_in = Shred::CoordInShred(i);
            const int j_in = Shred::CoordInShred(j);
            int k = k_start;
            for ( ; INVISIBLE == shred->GetBlock(i_in, j_in, k)->Transparent();
                k += k_step);
            if ( player->Visible(i, j, k) ) {
                waddch(window,
                    PrintBlock(*shred->GetBlock(i_in, j_in, k), window));
                waddch(window, CharNumber(k));
            } else {
                wattrset(window, SHADOW_COLOR);
                waddch(window, OBSCURE_BLOCK);
                waddch(window, ' ');
            }
        }
    }

    if ( dir > DOWN ) {
        const Block * const block = player->GetBlock();
        wattrset(window, Color(block->Kind(), block->Sub()));
        mvwaddstr(window, player->Y()-start_y+1, (player->X()-start_x)*2+2,
            qPrintable(arrows[player->GetDir()]));
    }

    wstandend(window);
    box(window, 0, 0);
    Arrows(window, (player->X()-start_x)*2+1, player->Y()-start_y+1, UP);
    wrefresh(window);
} // void Screen::PrintNormal(WINDOW * window, int dir)

void Screen::PrintFront(const dirs dir, const int block_x, const int block_y)
const {
    const int pX = player->X();
    const int begin_x = ( pX/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
    const int pY = player->Y();
    const int begin_y = ( pY/SHRED_WIDTH )*SHRED_WIDTH +
        ( SHRED_WIDTH-SCREEN_SIZE )/2;
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
        x_start = begin_x;
        x_end   = x_start + SCREEN_SIZE;
        z = &j;
        z_step  = -1;
        z_start = pY - 1;
        z_end   = qMax(0, pY - SHRED_WIDTH*2);
        arrow_X = (pX - begin_x)*2 + 1;
        break;
    case SOUTH:
        x = &i;
        x_step  = -1;
        x_start = SCREEN_SIZE - 1 + begin_x;
        x_end   = begin_x - 1;
        z = &j;
        z_step  = 1;
        z_start = pY + 1;
        z_end   = qMin(pY+SHRED_WIDTH*2, w->GetBound());
        arrow_X = (SCREEN_SIZE - pX + begin_x)*2 - 1;
        break;
    case EAST:
        x = &j;
        x_step  = 1;
        x_start = begin_y;
        x_end   = SCREEN_SIZE + begin_y;
        z = &i;
        z_step  = 1;
        z_start = pX + 1;
        z_end   = qMin(pX + SHRED_WIDTH*2, w->GetBound());
        arrow_X = (pY - begin_y)*2 + 1;
        break;
    case WEST:
        x = &j;
        x_step  = -1;
        x_start = SCREEN_SIZE - 1 + begin_y;
        x_end   = begin_y - 1;
        z = &i;
        z_step  = -1;
        z_start = pX - 1;
        z_end   = qMax(0, pX - SHRED_WIDTH*2);
        arrow_X = (SCREEN_SIZE - pY + begin_y)*2 - 1;
        break;
    default:
        Q_UNREACHABLE();
        return;
    }
    const int k_start =
        qBound(SCREEN_SIZE-1, player->Z()+SCREEN_SIZE/2, HEIGHT-1);
    if ( block_x > 0 ) {
        // ugly! use print function to get block by screen coordinates.
        int k = k_start - block_y + 1;
        *x = x_start + x_step * (block_x-1)/2;
        for (*z=z_start; *z!=z_end && w->GetBlock(i, j, k)->
                    Transparent()==INVISIBLE;
                *z += z_step);
        player->Examine(i, j, k);
        return;
    }
    const int sky_color = Color(BLOCK, SKY);
    (void)wmove(rightWin, 1, 1);
    for (int k=k_start; k>k_start-SCREEN_SIZE; --k, waddstr(rightWin, "\n_")) {
        randomBlink = blinkOn ? qrand() : 0;
        for (*x=x_start; *x!=x_end; *x+=x_step) {
            for (*z=z_start; *z!=z_end && w->GetBlock(i, j, k)->
                        Transparent()==INVISIBLE;
                    *z += z_step);
            if ( *z == z_end ) {
                static const int sky_char = CharName(BLOCK, SKY);
                wattrset(rightWin, sky_color);
                waddch(rightWin, sky_char);
                waddch(rightWin, ' ');
            } else if ( player->Visible(i, j, k) ) {
                waddch(rightWin, PrintBlock(*w->GetBlock(i, j, k), rightWin));
                waddch(rightWin, CharNumberFront(i, j));
            } else {
                wattrset(rightWin, SHADOW_COLOR);
                waddch(rightWin, OBSCURE_BLOCK);
                waddch(rightWin, ' ');
            }
        }
    }
    wstandend(rightWin);
    box(rightWin, 0, 0);
    const int arrow_Y = k_start + 1 - player->Z();
    if ( shiftFocus ) {
        wattrset(rightWin, COLOR_PAIR(WHITE_BLUE));
        const int ch = ( shiftFocus == 1 ) ? '^' : 'v';
        for (int q=arrow_Y-shiftFocus; 0<q && q<=SCREEN_SIZE; q-=shiftFocus) {
            mvwaddch(rightWin, q,               0, ch);
            mvwaddch(rightWin, q, SCREEN_SIZE*2+1, ch);
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
        const int number = inv->Number(i);
        if ( 0 == number ) {
            wattrset(window, COLOR_PAIR(BLACK_BLACK) | A_BOLD);
            waddstr(window, qPrintable(inv->InvFullName(i)));
            continue;
        }
        const Block * const block = inv->ShowBlock(i);
        wprintw(window, "[%c]%s",
            PrintBlock(*block, window), qPrintable(inv->InvFullName(i)) );
        if ( MAX_DURABILITY != block->GetDurability() ) {
            wprintw(window, "{%d}", block->GetDurability()*100/MAX_DURABILITY);
        }
        const QString str = block->GetNote();
        if ( not str.isEmpty() ) {
            const int x = getcurx(window);
            const int width = SCREEN_SIZE*2 - x - 3 - 8;
            if ( str.length() <= width ) {
                wprintw(window, " ~:%s", qPrintable(str));
            } else {
                wprintw(window, " ~:%s ...", qPrintable(str.left(width - 4)));
            }
        }
        wstandend(window);
        mvwprintw(window, 2+i+shift, 53, "%5hu mz", inv->GetInvWeight(i));
    }
    wstandend(window);
    QString full_weight = tr("Full weight: %1 mz").
        arg(inv->Weight(), 6, 10, QChar(' '));
    mvwprintw(window, 2+inv->Size()+shift,
        SCREEN_SIZE*2 + 1 - full_weight.length(), qPrintable(full_weight));
    wattrset(window, Color(block->Kind(), block->Sub()));
    box(window, 0, 0);
    if ( start != 0 ) {
        mvwhline(window, 2+start, 1, ACS_HLINE, SCREEN_SIZE*2);
    }
    mvwprintw(window, 0, 1, "[%c] %s", CharName( block->Kind(), block->Sub()),
        qPrintable((player->PlayerInventory() == inv) ?
            tr("Your inventory") : block->FullName()) );
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
    if ( not PrintFile(rightWin, path) ) {
        Notify(tr("There is no such file."));
    }
}

void Screen::Notify(const QString str) const {
    fputs(qPrintable(QString("%1 %2\n").arg(w->TimeOfDayStr()).arg(str)),
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
        mvwprintw(notifyWin, getcury(notifyWin)-1, 0, "%s (x%d)\n",
            qPrintable(str), notification_repeat_count);
    } else {
        notification_repeat_count = 1;
        waddstr(notifyWin, qPrintable(lastNotification = str));
        waddch(notifyWin, '\n');
    }
    wrefresh(notifyWin);
}

void Screen::DeathScreen() {
    werase(rightWin);
    werase(hudWin);
    wcolor_set(leftWin, WHITE_RED, nullptr);
    if ( not PrintFile(leftWin, ":/texts/death.txt") ) {
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
        updated(),
        notifyLog(fopen(qPrintable(home_path + "log.txt"), "at")),
        actionMode(ACTION_USE),
        shiftFocus(settings.value("focus_shift", 0).toInt()),
        fileToShow(nullptr),
        beepOn (settings.value("beep_on",  false).toBool()),
        flashOn(settings.value("flash_on", true ).toBool()),
        ascii(_ascii && settings.value("ascii", true).toBool() ),
        blinkOn(not _ascii || settings.value("blink_on", true).toBool()),
        arrows{'.', 'x',
            ascii ? '^' : 0x2191,
            ascii ? 'v' : 0x2193,
            ascii ? '>' : 0x2192,
            ascii ? '<' : 0x2190
        },
        screen(newterm(nullptr, stdout, stdin)),
        randomBlink()
{
    #ifndef Q_OS_WIN32
        set_escdelay(10);
    #endif
    start_color();
    nodelay(stdscr, FALSE);
    noecho(); // do not print typed symbols
    nonl();
    keypad(stdscr, TRUE); // use arrows
    mousemask(BUTTON1_CLICKED | BUTTON1_RELEASED | BUTTON4_CLICKED | BUTTON5_CLICKED, nullptr);
    memset(windows, 0, sizeof(windows));
    if ( LINES < 41 && IsScreenWide() ) {
        printf("Make your terminal height to be at least 41 lines.\n");
        error = HEIGHT_NOT_ENOUGH;
        return;
    } else if ( LINES < 39 ) {
        printf("Make your terminal height to be at least 39 lines.\n");
        error = HEIGHT_NOT_ENOUGH;
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
    if ( IsScreenWide() ) {
        const int left_border = COLS/2-SCREEN_SIZE*2-2;
        rightWin  = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, COLS/2);
        leftWin   = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, left_border);
        hudWin    = newwin(3, preferred_width, SCREEN_SIZE+2, left_border);
        notifyWin = newwin(0, 0, SCREEN_SIZE+5,left_border+33);
        minimapWin =
            newwin(MINIMAP_HEIGHT, MINIMAP_WIDTH, SCREEN_SIZE+5, left_border);
        actionWin  = newwin(7, 20, SCREEN_SIZE+5, left_border+12);
    } else if ( COLS >= preferred_width/2 ) {
        const int left_border = COLS/2-SCREEN_SIZE-1;
        leftWin   = newwin(SCREEN_SIZE+2, SCREEN_SIZE*2+2, 0, left_border);
        hudWin    = newwin(2,SCREEN_SIZE*2+2-15, SCREEN_SIZE+2,left_border+15);
        notifyWin = newwin(0,  0, SCREEN_SIZE+2+2, left_border+15);
        actionWin = newwin(7, 15, SCREEN_SIZE+2,   left_border);
    } else {
        puts(qPrintable(tr("Set your terminal width at least %1 chars.").
            arg(SCREEN_SIZE*2+2)));
        error = WIDTH_NOT_ENOUGH;
        return;
    }
    scrollok(notifyWin, TRUE);

    if ( not PrintFile(stdscr, ":/texts/splash.txt") ) {
        addstr("Free-Roaming Elementary Game\nby mmaulwurff\n");
    }
    addstr(qPrintable(tr("\nVersion %1.\n\nPress any key.").arg(VER)));
    qsrand(getch());
    CleanFileToShow();
    RePrint();
    SetActionMode(static_cast<actions>
        (settings.value("action_mode", ACTION_USE).toInt()));
    Print();
    Notify(tr("--- Game started. Press 'H' for help. ---"));
    if ( not IsScreenWide() ) {
        Notify(tr("For better gameplay set your"));
        Notify(tr("terminal width at least %1 chars.").arg(preferred_width));
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
    settings.setValue("focus_shift", shiftFocus);
    settings.setValue("action_mode", actionMode);
    settings.setValue("last_command", previousCommand);
    settings.setValue("beep_on",  beepOn);
    settings.setValue("flash_on", flashOn);
    settings.setValue("blink_on", blinkOn);
    settings.setValue("ascii", ascii);
}

void Screen::PrintBar(const int x, const int attr, const int ch,
        const int percent, const bool value_position_right)
{
    wstandend(hudWin);
    mvwprintw(hudWin, 0, x,
        value_position_right ? "[..........]%hd" : "%3hd[..........]",percent);
    wattrset(hudWin, attr);
    mvwaddstr(hudWin, 0, x + (value_position_right ? 1 : 4),
        qPrintable(QString(10, QChar(ch)).left(percent/10)));
}
