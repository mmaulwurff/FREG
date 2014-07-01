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

/**\file CursedScreen.h
 * \brief Provides curses (text-based graphics interface) screen for freg.*/

#ifndef CURSEDSCREEN_H
#define CURSEDSCREEN_H

#define NOX

#include <QMutex>

#ifdef __MINGW32__
#include "pdcurses/curses.h"
#else
#define _X_OPEN_SOURCE_EXTENDED
#include <ncursesw/ncurses.h>
#endif
#include "screens/VirtScreen.h"

const int SCREEN_SIZE = 30;

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
    SCREEN_NO_ERROR = 0,
    HEIGHT_NOT_ENOUGH,
    WIDTH_NOT_ENOUGH
};

class IThread;
class Inventory;
class QFile;
class Block;

class Screen final : public VirtScreen {
    Q_OBJECT
public:
     Screen(World *, Player *, int & error, bool ascii);
    ~Screen() override;

    int  GetChar() const override;
    void FlushInput() const override;
    void ControlPlayer(int command) override;

public slots:
    void Notify(QString) const override;
    void PassString(QString &) const override;
    void Update(int, int, int) override;
    void UpdateAll() override;
    void UpdatePlayer() override;
    void UpdateAround(int, int, int, int) override;
    void Move(int) override;
    void DeathScreen();
    void DisplayFile(QString path) override;
    void ActionXyz(int * x, int * y, int * z) const override;

private slots:
    void Print() override;

private:
    char CharNumber(int z) const;
    char CharNumberFront(int x, int y) const;
    void Arrows(WINDOW *, int x, int y, bool show_dir = false) const;
    void HorizontalArrows(WINDOW *, int y, int color = WHITE_RED,
            bool show_dir = false) const;
    void PrintNormal(WINDOW *, int dir) const;
    void PrintFront(WINDOW *) const;
    void PrintInv(WINDOW *, const Inventory &) const;
    /// Can print health, breath and other bars on hudWin.
    void PrintBar(int x, int color, int ch, int percent,
            bool value_position_right = true);
    /// Returns false when file does not exist, otherwise true.
    bool PrintFile(WINDOW *, QString const & file_name);
    void PrintHUD();
    void CleanFileToShow();
    void RePrint();
    void InventoryAction(int num) const;
    int  Color(int kind, int sub) const;
    static color_pairs ColorShred(int type);
    char PrintBlock(const Block &, WINDOW *) const;
    void SetActionMode(actions mode);
    void ProcessCommand(QString command);
    void PrintTitle(WINDOW *, int dir) const;
    void MovePlayer(dirs dir);
    void MovePlayerDiag(dirs dir1, dirs dir2) const;
    static bool IsScreenWide();

    WINDOW * leftWin;
    WINDOW * rightWin;
    WINDOW * notifyWin;
    WINDOW * hudWin; // head-up display
    WINDOW * miniMapWin;
    mutable QString lastNotification;
    IThread * const input;
    volatile bool updated;
    FILE * const notifyLog;
    actions actionMode;
    /// Can be -1, 0, 1 for low, normal, and high focus.
    int shiftFocus;
    /// Save previous command for further execution.
    QString previousCommand;
    QFile * fileToShow;
    bool beepOn;
    const bool ascii;
    SCREEN * const screen;
};

#endif // CURSEDSCREEN_H
