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

#ifndef CURSEDSCREEN_H
#define CURSEDSCREEN_H

#include "screens/VirtScreen.h"
#include "header.h"

#ifdef Q_OS_WIN32
    #define NCURSES_MOUSE_VERSION 2
    #define PDC_WIDE
    #define _LP64 0
    #include "pdcurses/curses.h"
#else
    #define _X_OPEN_SOURCE_EXTENDED
    #include <ncursesw/ncurses.h>
    #undef timeout // conflict with Qt
#endif

/// Cursed screen options
/// X(settings_string, variable_name, default_value)
#define OPTIONS_TABLE(X) \
X("beep_on",         beepOn,       false, )\
X("flash_on",        flashOn,      true,  )\
X("blink_on",        blinkOn,      false, )\
X("ascii",           ascii,        false, )\
X("mouse_on",        mouseOn,      true,  )\

#define OPTIONS_DECLARE(string, name, ...) bool name;

class Block;

class Screen final : public VirtScreen {
    Q_OBJECT
    Q_DISABLE_COPY(Screen)
public:
     Screen(Player*, int& error);
    ~Screen() override;

    void ControlPlayer();
    void ControlPlayer(int key);

    /// This is called for a notification to be displayed.
    void Notify(QString) const override;
    void PassString(QString&) const override;
    void UpdateAll() override;
    void UpdatePlayer() override;
    void UpdateAround(int x, int y, int z) override;
    void Move(int) override;
    void DeathScreen() override;
    void DisplayFile(QString path) override;
    void ActionXyz(int* x, int* y, int* z) const override;

    static Screen* GetScreen();

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

    static const chtype arrows[LAST_DIR + 1];
    static const int MOUSEMASK = BUTTON1_CLICKED | BUTTON1_RELEASED;
    static const int MAX_CHAR_DISTANCE;
    static const int ASCII_SIZE = 127;
    static const int ACTIVE_HAND = 3;

    /// Prints world. Should not be called not within screen.
    void Print();
    char CharNumber(int z) const;
    char CharNumberFront(int x, int y) const;
    char Distance(int distance) const;
    void Arrows(WINDOW*, int x, int y, dirs, bool is_normal) const;
    void PrintNormal(WINDOW*, dirs) const;
    /// Has two functions: first - when x == -1 - prints front,
    /// second - otherwise - examines block at position x, y.
    void PrintFront(dirs direction, int x = -1, int y = 0) const;
    void PrintInv(WINDOW*, const Block*, const class Inventory*) const;
    void PrintHud() const;
    void PrintMiniMap() const;
    void PrintQuickInventory() const;
    void RePrint();
    void InventoryAction(int num) const;
    void SetActionMode(actions mode);
    void ProcessCommand(QString command);
    void ProcessMouse();
    void MovePlayer(dirs) const;
    void MovePlayerDiag(dirs dir1, dirs dir2) const;
    void TestNotify() const;

    /// Can print health, breath and other bars on hudWin.
    static void PrintBar(WINDOW*, wchar_t ch, int color, int percent);
    static void PrintBlock(const Block*, WINDOW*, char second);
    static void PrintShadow(WINDOW*);
    static void DrawBorder(WINDOW*);
    static int  Color(int kind, int sub);
    static int  ColorShred(shred_type);
    static int  ColoredChar(const Block*);
    static int  RandomBlink();
    static bool RandomBit();
    static bool IsScreenWide();
    static bool IsOutWindow(const MEVENT&, int hor_bound, int vert_bound);
    static void Palette(WINDOW*);

    /// Returns nullptr if block is not player->Visible().
    Block* GetFocusedBlock() const;

    int GetNormalStartX() const;
    int GetNormalStartY() const;
    int GetFrontStartZ() const;
    int GetMinimapStartX() const;
    int GetMinimapStartY() const;
    void ExamineOnNormalScreen(int x, int y, int z, int step) const;

    void initializeKeyTable();

    SCREEN* const screen;
    int screenWidth, screenHeight;
    WINDOW* const windows[WIN_COUNT];
    WINDOW* const& actionWin  = windows[WIN_ACTION ];
    WINDOW* const& notifyWin  = windows[WIN_NOTIFY ];
    WINDOW* const& hudWin     = windows[WIN_HUD    ];
    WINDOW* const& minimapWin = windows[WIN_MINIMAP];
    WINDOW* const& leftWin    = windows[WIN_LEFT   ];
    WINDOW* const& rightWin   = windows[WIN_RIGHT  ];
    mutable QString lastNotification;
    class IThread* const input;
    mutable volatile bool updatedHud, updatedMinimap;
    mutable volatile bool updatedNormal, updatedFront;
    actions actionMode;
    /// Can be -1, 0, 1 for low, normal, and high focus.
    int shiftFocus;
    mutable bool inputActive = false;
    OPTIONS_TABLE(OPTIONS_DECLARE)
    int showCharDistance;
    const wchar_t ellipsis[4];
    mmask_t noMouseMask;
    mutable int xCursor, yCursor; ///< Save cursor position to show player.
    void (*keyTable[ASCII_SIZE])(int);

    static Screen* staticScreen;
};

#endif // CURSEDSCREEN_H
