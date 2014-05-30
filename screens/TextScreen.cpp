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

/**\file TextScreen.cpp
 * \brief This file is related to text screen for freg. */

#include <QTimer>
#include <QSettings>
#include <QDir>
#include <QMutex>
#include <QLocale>
#include "screens/TextScreen.h"
#include "screens/IThread.h"
#include "world.h"
#include "blocks/Block.h"
#include "blocks/Inventory.h"
#include "Player.h"

void Screen::Update(int, int, int) {}
void Screen::UpdatePlayer() {}
void Screen::UpdateAll() {}
void Screen::UpdateAround(int, int, int, ushort) {}
void Screen::Move(int) {}

void Screen::PassString(QString & str) const {
    static const ushort NOTE_LENGTH = 144;
    char temp_str[NOTE_LENGTH+1];
    fgets(temp_str, NOTE_LENGTH, stdin);
    fprintf(notifyLog, "%lu: Command: %s\n", w->Time(), temp_str);
    str = QString::fromUtf8(temp_str);
}

int  Screen::GetChar() const { return getchar(); }
void Screen::FlushInput() const {}

void Screen::MovePlayer(const int dir) const {
    if ( player->GetDir() == dir ) {
        player->Move(dir);
    } else {
        player->SetDir(dir);
    }
}

void Screen::MovePlayerDiag(const int dir1, const int dir2) const {
    player->SetDir(dir1);
    static bool step_trigger = true;
    player->Move(step_trigger ? dir1 : dir2);
    step_trigger = !step_trigger;
}

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
    case '8': MovePlayer(NORTH);  break;
    case '2': MovePlayer(SOUTH);  break;
    case '6': MovePlayer(EAST);   break;
    case '4': MovePlayer(WEST);   break;
    case '5': player->Move(DOWN); break;
    case '7': MovePlayerDiag(NORTH, WEST); break;
    case '9': MovePlayerDiag(NORTH, EAST); break;
    case '1': MovePlayerDiag(SOUTH, WEST); break;
    case '3': MovePlayerDiag(SOUTH, EAST); break;
    case ' ': player->Jump(); break;
    case '=': player->Move(); break;

    case '>': player->SetDir(World::TurnRight(player->GetDir())); break;
    case '<': player->SetDir(World::TurnLeft (player->GetDir())); break;
    case 'A': player->SetDir(DOWN); break;
    case 'Z': player->SetDir(UP);   break;

    case 'I': player->Backpack(); break;
    case 8: player->Damage(); break;
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
    case 'H':
        DisplayFile(QString("help_%1/help.txt").arg(locale.left(2)));
    break;
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
} // void Screen::ControlPlayer(int ch)

void Screen::ProcessCommand(QString command) {
    if ( command.length()==1 && "."!=command ) {
        ControlPlayer(command.at(0).toLatin1());
    } else if ( "warranty" == command ) {
        PrintFile("texts/warranty.txt");
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

void Screen::SetActionMode(const actions mode) { actionMode = mode; }

void Screen::InventoryAction(const ushort num) const {
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

void Screen::Print() {}

void Screen::CleanFileToShow() {
    delete fileToShow;
    fileToShow = 0;
}

bool Screen::PrintFile(QString const & file_name) {
    CleanFileToShow();
    fileToShow = new QFile(file_name);
    if ( fileToShow->open(QIODevice::ReadOnly | QIODevice::Text) ) {
        puts(qPrintable(
            QString::fromLocal8Bit(fileToShow->readAll().constData())) );
        return true;
    } else {
        CleanFileToShow();
        return false;
    }
}

void Screen::DisplayFile(QString path) {
    Notify( PrintFile(path) ?
        tr("File path: %1/%2").arg(QDir::currentPath()).arg(path) :
        tr("There is no such help file: %1/%2.")
            .arg(QDir::currentPath()).arg(path) );
}

void Screen::Notify(const QString str) const {
    fputs(qPrintable(QString("%1 %2\n").arg(w->TimeOfDayStr()).arg(str)),
        notifyLog);
    static int notification_repeat_count = 1;
    static QString last_notification;
    if ( ++notification_repeat_count && str == last_notification ) {
        printf("%s (%dx)\n", qPrintable(str), notification_repeat_count);
    } else {
        notification_repeat_count = 1;
        printf("%s\n", qPrintable(str));
        last_notification = str;
    }
}

void Screen::DeathScreen() {
    Notify(DING);
    if ( not PrintFile("texts/death.txt") ) {
        puts(qPrintable(tr("You die.\nWaiting for respawn...")));
    }
}

Screen::Screen(
        World  * const wor,
        Player * const pl,
        int &, // error
        bool _ascii)
    :
        VirtScreen(wor, pl),
        input(new IThread(this)),
        timer(new QTimer(this)),
        notifyLog(fopen("texts/messages.txt", "at")),
        fileToShow(nullptr),
        ascii(_ascii)
{
    QSettings sett(QDir::currentPath()+"/freg.ini", QSettings::IniFormat);
    sett.beginGroup("screen_curses");
    shiftFocus = sett.value("focus_shift", 0).toInt();
    actionMode = static_cast<actions>
        (sett.value("action_mode", ACTION_USE).toInt());
    command    = sett.value("last_command", "hello").toString();

    if ( not PrintFile("texts/splash.txt") ) {
        puts("Free-Roaming Elementary Game\nby mmaulwurff\n");
    }
    puts(qPrintable(tr("\nVersion %1.\n\nPress any key.").arg(VER)));
    qsrand(getchar());
    CleanFileToShow();
    Notify(tr("*--- Game started. Press 'H' for help. ---*"));

    input->start();
    connect(timer, SIGNAL(timeout()), SLOT(Print()));
    timer->start(100);
}

void Screen::CleanAll() {
    static bool cleaned = false;
    if ( cleaned ) return;
    cleaned = true; // prevent double cleaning
    input->Stop();
    input->wait();
    delete input;
    delete timer;

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
