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

#ifndef VIRTSCREEN_H
#define VIRTSCREEN_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <QObject>
#include <QSettings>
#include <QString>
#pragma GCC diagnostic pop

#define COLOR_LIST(color) \
    color ## _BLACK,  \
    color ## _RED,    \
    color ## _GREEN,  \
    color ## _YELLOW, \
    color ## _BLUE,   \
    color ## _MAGENTA,\
    color ## _CYAN,   \
    color ## _WHITE,

class Player;
class World;

enum screen_errors {
    SCREEN_NO_ERROR = 0,
    HEIGHT_NOT_ENOUGH,
    WIDTH_NOT_ENOUGH
};

class VirtScreen : public QObject {
    /** \class VirtScreen VirtScreen.h
     *  \brief This class provides base for all screens for freg.
     *
     * Provides interface for world-screen and player-screen communications. */
    Q_OBJECT
    Q_DISABLE_COPY(VirtScreen)
public:
    /// Constructor makes player and world connections.
    VirtScreen(Player *);
    VirtScreen(VirtScreen &) = delete;
    virtual ~VirtScreen();

    static char CharName(int kind, int sub);
    static int  Color   (int kind, int sub);

    /// This is called for a notification to be displayed.
    virtual void Notify(QString) const = 0;

    /// This is called when string is needed to be received from input.
    /** It is connected to world in constructor. */
    virtual void PassString(QString &) const = 0;

    /// This is called when block at (x, y, z) should be updated in screen.
    /** When implemented, this should work fast.
     *  It is connected to world in constructor. */
    virtual void Update(int x, int y, int z) = 0;

    /// This is called when all world should be updated in sceen.
    /** When implemented, this should work fast.
     *  It is connected to world in constructor. */
    virtual void UpdateAll() = 0;

    /// Called when world loaded zone is moved to update world in screen.
    /** When implemented, this should work fast.
     *  It is connected to world in constructor. */
    virtual void Move(int) = 0;

    /// Called when some player property needs to be updated in screen.
    /** When implemented, this should work fast.
     *  It is connected to world in constructor. */
    virtual void UpdatePlayer() = 0;

    /// Called when area around xyz with range needs to be updated.
    /** When implemented, this should work fast.
     *  It is connected to world in constructor. */
    virtual void UpdateAround(int x, int y, int z, int rng) = 0;

    /// This is called when current group of updates is ended.
    /** This is called from world when pack of world changing is ended.
     *  ( Can be used in screen optimization. ) */
    virtual void UpdatesEnd();

    /// This is called when player is dead, and displayed until respawn.
    void DeathScreen(); // virtual ?

    /// Used to get player focus coordinates from screen.
    /** x, y, z are coordinates where player will make action.
     *  May be reimplemented in derivative class to get xyz other than
     *  world direction-based focus. */
    virtual void ActionXyz(int * x, int * y, int * z) const;

    /// This shows a file by path.
    /** Standard (non-reimpemented) version does nothing. */
    virtual void DisplayFile(QString path);

signals:
    /// This is emitted when input receives exit key.
    /** This is connected to application exit. */
    void ExitReceived();

    /// Emitted to pause physics in world.
    void PauseWorld();
    /// Emitted to resume paused physics in world.
    void ResumeWorld();

protected:
    /// All available colors. Format: foreground_background (e.ge WHITE_BLACK)
    enum color_pairs { // do not change colors order!
        UNUSED_ZERO_COLOR, ///< colors should start from 1, so occupy 0 by this
        COLOR_LIST(BLACK)
        COLOR_LIST(RED)
        COLOR_LIST(GREEN)
        COLOR_LIST(YELLOW)
        COLOR_LIST(BLUE)
        COLOR_LIST(MAGENTA)
        COLOR_LIST(CYAN)
        COLOR_LIST(WHITE)
    };

    /// Returns true if command is recognized and processed.
    bool ProcessCommand(QString command);

    Player * const player;
    QSettings settings;
    QString previousCommand;
};

#endif // VIRTSCREEN_H
