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

/**\file screen.h
 * \brief Provides curses (text-based graphics interface) screen for freg.*/

#ifndef SCREEN_H
#define SCREEN_H

#define NOX

#define _X_OPEN_SOURCE_EXTENDED
#include <ncursesw/ncurses.h>
#include "VirtScreen.h"

const ushort SCREEN_SIZE = 30;

enum actions {
    ACTION_USE,
    ACTION_THROW,
    ACTION_OBTAIN,
    ACTION_WIELD,
    ACTION_INSCRIBE,
    ACTION_EAT,
    ACTION_BUILD,
    ACTION_CRAFT,
    ACTION_TAKEOFF
};

enum color_pairs { // do not change colors order! // foreground_background
    BLACK_BLACK = 1,
    BLACK_RED,
    BLACK_GREEN,
    BLACK_YELLOW,
    BLACK_BLUE,
    BLACK_MAGENTA,
    BLACK_CYAN,
    BLACK_WHITE,
    //
    RED_BLACK,
    RED_RED,
    RED_GREEN,
    RED_YELLOW,
    RED_BLUE,
    RED_MAGENTA,
    RED_CYAN,
    RED_WHITE,
    //
    GREEN_BLACK,
    GREEN_RED,
    GREEN_GREEN,
    GREEN_YELLOW,
    GREEN_BLUE,
    GREEN_MAGENTA,
    GREEN_CYAN,
    GREEN_WHITE,
    //
    YELLOW_BLACK,
    YELLOW_RED,
    YELLOW_GREEN,
    YELLOW_YELLOW,
    YELLOW_BLUE,
    YELLOW_MAGENTA,
    YELLOW_CYAN,
    YELLOW_WHITE,
    //
    BLUE_BLACK,
    BLUE_RED,
    BLUE_GREEN,
    BLUE_YELLOW,
    BLUE_BLUE,
    BLUE_MAGENTA,
    BLUE_CYAN,
    BLUE_WHITE,
    //
    MAGENTA_BLACK,
    MAGENTA_RED,
    MAGENTA_GREEN,
    MAGENTA_YELLOW,
    MAGENTA_BLUE,
    MAGENTA_MAGENTA,
    MAGENTA_CYAN,
    MAGENTA_WHITE,
    //
    CYAN_BLACK,
    CYAN_RED,
    CYAN_GREEN,
    CYAN_YELLOW,
    CYAN_BLUE,
    CYAN_MAGENTA,
    CYAN_CYAN,
    CYAN_WHITE,
    //
    WHITE_BLACK,
    WHITE_RED,
    WHITE_GREEN,
    WHITE_YELLOW,
    WHITE_BLUE,
    WHITE_MAGENTA,
    WHITE_CYAN,
    WHITE_WHITE
}; // enum color_pairs

enum screen_errors {
    NO_ERROR = 0,
    HEIGHT_NOT_ENOUGH,
    WIDTH_NOT_ENOUGH
};

class IThread;
class Inventory;
class QTimer;
class QFile;
class Block;
class QMutex;

class Screen : public VirtScreen {
    Q_OBJECT
public:
     Screen(World *, Player *, int & error);
    ~Screen();

    void ControlPlayer(int);
public slots:
    void Notify(const QString &);
    void CleanAll();
    QString PassString(QString &) const;
    void Update(ushort, ushort, ushort);
    void UpdateAll();
    void UpdatePlayer();
    void UpdateAround(ushort, ushort, ushort, ushort);
    void Move(int);
    void DeathScreen();
    void DisplayFile(QString path);
private slots:
    void Print();
private:
    char CharName(int, int) const;
    char CharNumber(ushort x, ushort y, ushort z) const;
    char CharNumberFront(ushort x, ushort y) const;
    void Arrows(WINDOW *, ushort x, ushort y, bool show_dir = false) const;
    void HorizontalArrows(WINDOW *, ushort y, short color = WHITE_RED,
            bool show_dir = false) const;
    void ActionXyz(ushort & x, ushort & y, ushort & z) const;
    void PrintNormal(WINDOW *, int dir) const;
    void PrintFront(WINDOW *) const;
    void PrintInv(WINDOW *, const Inventory *) const;
    void PrintText(WINDOW *, QString const &) const;
    /// Returns false when file does not exist, otherwise true.
    bool PrintFile(WINDOW *, QString const & file_name);
    void PrintHUD();
    void CleanFileToShow();
    void RePrint();
    void MouseAction();
    void InventoryAction(ushort num) const;
    color_pairs Color(int kind, int sub) const;
    char PrintBlock(const Block *, WINDOW *) const;
    void SetActionMode(const int mode);
    void ProcessCommand(QString & command);

    WINDOW * leftWin;
    WINDOW * rightWin;
    WINDOW * notifyWin;
    WINDOW * commandWin;
    WINDOW * hudWin; // head-up display
    IThread * const input;
    volatile bool updated;
    volatile bool updatedPlayer;
    QTimer * const timer;
    FILE * const notifyLog;
    int actionMode;
    short shiftFocus; // can be -1, 0, 1 for low, normal, and high focus
    QString command; // save previous command for further execution
    QString lastNotification;
    quint8 notificationRepeatCount;
    QFile * fileToShow;
    bool beepOn;
    QMutex * mutex;
}; // class Screen

/** \class IThread screen.h
 * \brief Keyboard input thread for curses screen for freg.
 *
 * This class is thread, with IThread::run containing input loop. */

#include <QThread>

class IThread : public QThread {
    Q_OBJECT
public:
    IThread(Screen * const);
    void Stop();
protected:
    void run();
private:
    Screen * const screen;
    volatile bool stopped;
}; // class IThread

#endif // SCREEN_H
