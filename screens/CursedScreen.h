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

/**\file CursedScreen.h
 * \brief Provides curses (text-based graphics interface) screen for freg.*/

#ifndef CURSEDSCREEN_H
#define CURSEDSCREEN_H

#include "header.h"

#ifdef Q_OS_WIN32
#define NCURSES_MOUSE_VERSION 2
#define PDC_WIDE
#define _LP64 0
#include "pdcurses/curses.h"
#else
#define _X_OPEN_SOURCE_EXTENDED
#include <ncursesw/ncurses.h>
#endif
#include "screens/VirtScreen.h"

#define wPrintable(string) QString(string).toStdWString().c_str()

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
    Q_DISABLE_COPY(Screen)
public:
     Screen(Player *, int & error);
    ~Screen() override;

    void ControlPlayer();
    void ControlPlayer(int ch);

public slots:
    /// This is called for a notification to be displayed.
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
    /// Prints world. Should not be called not within screen.
    void Print();

private:
    enum windowIndex {
        WIN_ACTION,
        WIN_NOTIFY,
        WIN_HUD, ///< head-up display
        WIN_MINIMAP,
        WIN_LEFT,
        WIN_RIGHT,
        WIN_COUNT // keep it last
    };

    enum actions {
        ACTION_USE,
        ACTION_THROW,
        ACTION_OBTAIN,
        ACTION_INSCRIBE,
        ACTION_BUILD,
        ACTION_CRAFT,
        ACTION_WIELD,
    };

    enum cursed_screen_sizes {
        ACTIONS_WIDTH  = 23,
        MINIMAP_WIDTH  = 11,
        MINIMAP_HEIGHT =  7,
        AVERAGE_SCREEN_SIZE = 60,
        FRONT_MAX_DISTANCE = SHRED_WIDTH * 2,
    };

    const chtype OBSCURE_BLOCK = COLOR_PAIR(BLACK_BLACK) | A_BOLD|ACS_CKBOARD;
    const int ARROWS_COLOR = COLOR_PAIR(WHITE_RED);
    const int MOUSEMASK = BUTTON1_CLICKED | BUTTON1_RELEASED;

    char CharNumber(int z) const;
    char CharNumberFront(int x, int y) const;
    char Distance(int distance) const;
    void Arrows(WINDOW *, int x, int y, dirs, bool is_normal) const;
    void HorizontalArrows(WINDOW *, int y, dirs) const;
    void PrintNormal(WINDOW *, dirs) const;
    /// Has two functions: first - when x == -1 - prints front,
    /// second - otherwise - examines block at position x, y.
    void PrintFront(dirs direction, int x = -1, int y = 0) const;
    void PrintInv(WINDOW *, const Block *, const Inventory *) const;
    /// Returns false when file does not exist, otherwise true.
    bool PrintFile(WINDOW *, QString const & file_name);
    void PrintHud();
    void PrintMiniMap();
    void PrintQuickInventory();
    void CleanFileToShow();
    void RePrint();
    void InventoryAction(int num) const;
    int  ColorShred(shred_type)   const;
    int  ColoredChar(const Block *) const;
    void SetActionMode(actions mode);
    void ProcessCommand(QString command);
    void ProcessMouse();
    void MovePlayer(dirs dir);
    void MovePlayerDiag(dirs dir1, dirs dir2) const;

    /// Can print health, breath and other bars on hudWin.
    static void PrintBar(WINDOW *, wchar_t ch, int color, int percent);
    static int  Color(int kind, int sub);
    static void PrintBlock(const Block *, WINDOW *, char second);
    static bool IsScreenWide();
    static int  RandomBlink();
    static bool RandomBit();

    /// Returns nullptr if block is not player->Visible().
    Block * GetFocusedBlock() const;
    static void PrintVerticalDirection(WINDOW *, dirs);

    int GetNormalStartX() const;
    int GetNormalStartY() const;
    int GetFrontStartZ() const;
    int GetMinimapStartX() const;
    int GetMinimapStartY() const;
    void ExamineOnNormalScreen(int x, int y, int z, int step) const;

    void Greet() const;
    void DrawBorder(WINDOW *) const;

    SCREEN * const screen;
    const int screenWidth;
    const int screenHeight;

    WINDOW * const windows[WIN_COUNT];
    WINDOW * const & actionWin  = windows[WIN_ACTION ];
    WINDOW * const & hudWin     = windows[WIN_HUD    ];
    WINDOW * const & minimapWin = windows[WIN_MINIMAP];
    WINDOW * const & leftWin    = windows[WIN_LEFT   ];
    WINDOW * const & rightWin   = windows[WIN_RIGHT  ];
    mutable QString lastNotification;
    IThread * const input;
    mutable volatile bool updatedHud, updatedMinimap;
    mutable volatile bool updatedNormal, updatedFront;
    FILE * const notifyLog;
    actions actionMode;
    /// Can be -1, 0, 1 for low, normal, and high focus.
    int shiftFocus;
    QFile * fileToShow;
    const bool beepOn, flashOn;
    const bool ascii;
    bool blinkOn;
    const chtype arrows[DIRS_COUNT];
    const wchar_t ellipsis[4];
    mutable bool inputActive = false;
    bool showDistance;
    bool farDistance;

    mmask_t noMouseMask;
    bool mouseOn;
};

#endif // CURSEDSCREEN_H
