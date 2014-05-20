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

/**\file TextScreen.h
 * \brief Provides minimal text screen for freg.
 * Not suitable for playing. Can be used for testing and porting game to other
 * platforms. */

#ifndef TEXTSCREEN_H
#define TEXTSCREEN_H

#define NOX

#include "screens/VirtScreen.h"

enum screen_errors {
    SCREEN_NO_ERROR = 0,
    HEIGHT_NOT_ENOUGH,
    WIDTH_NOT_ENOUGH
};

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

class IThread;
class Inventory;
class QTimer;
class QFile;
class Block;
class QMutex;

class Screen final : public VirtScreen {
    Q_OBJECT
public:
     Screen(World *, Player *, int & error, bool ascii);
    ~Screen() override;

    int  GetChar() const override;
    void ControlPlayer(int command) override;
    void FlushInput() const override;

public slots:
    void Notify(QString) const override;
    void CleanAll() override;
    void PassString(QString &) const override;
    void Update(int, int, int) override;
    void UpdateAll() override;
    void UpdatePlayer() override;
    void UpdateAround(int, int, int, ushort) override;
    void Move(int) override;
    void DeathScreen();
    void DisplayFile(QString path) override;
    void ActionXyz(int * x, int * y, int * z) const override;

private slots:
    void Print() override;

private:
    char CharNumber(ushort z) const;
    char CharNumberFront(ushort x, ushort y) const;
    /// Returns false when file does not exist, otherwise true.
    bool PrintFile(QString const & file_name);
    void CleanFileToShow();
    void InventoryAction(ushort num) const;
    void SetActionMode(actions mode);
    void ProcessCommand(QString command);
    void MovePlayer(int dir) const;
    void MovePlayerDiag(int dir1, int dir2) const;

    IThread * const input;
    volatile bool updated;
    QTimer * const timer;
    FILE * const notifyLog;
    actions actionMode;
    /// Can be -1, 0, 1 for low, normal, and high focus.
    short shiftFocus;
    /// Save previous command for further execution.
    QString command;
    QFile * fileToShow;
    const bool ascii;
};

#endif // TEXTSCREEN_H
