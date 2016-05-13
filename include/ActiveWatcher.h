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

#ifndef ACTIVE_WATCHER_H
#define ACTIVE_WATCHER_H

#include "blocks/Active.h"
#include "header.h"
#include <QObject>

class QString;

class ActiveWatcher : public QObject {
    Q_OBJECT
public:

    explicit ActiveWatcher(QObject* const parent)
        : QObject(parent)
        , watched(nullptr)
    {}
    ~ActiveWatcher() { if (watched) watched->SetWatcher(nullptr); }

    void SetWatched(Active* const a) { watched = a; }

signals:

    void Moved(int direction) const;
    void ReceivedText(const QString& text) const;

    void Updated() const;
    void CauseTeleportation() const;

    void Destroyed() const;

private:

    Q_DISABLE_COPY(ActiveWatcher)

    Active* watched;
};

#endif // ACTIVE_WATCHER_H
