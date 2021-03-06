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

#ifndef LOGGER_H
#define LOGGER_H

#include "Singleton.h"
#include <QThread>
#include <QFile>

/** Simple logger class. Provides thread-safe log writing in parallel thread.
 *  After construction, Logger is not ready immediately. It emits ready() signal
 *  when it is ready. */
class Logger : public QThread, private Singleton<Logger> {
    Q_OBJECT

public:

    explicit Logger(const QString& filename);
    ~Logger();

    static Logger* GetInstance();

signals:

    void Log(const QString& message) const;
    void ready();

protected:

    void run() override;

private:

    const QString filename;

};

#endif // LOGGER_H
