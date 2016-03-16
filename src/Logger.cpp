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

#include "Logger.h"

Logger::Logger(const QString& name)
    : Singleton<Logger>(this)
    , filename(name)
{
    start();
}

Logger::~Logger()
{
    quit();
    wait();
}

Logger *Logger::GetInstance() { return Singleton<Logger>::GetInstance(); }

void Logger::run()
{
    QFile file(filename);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        connect(this, &Logger::Log, &file, [&file](const QString& message) {
            const QByteArray utf8 = message.toUtf8();
            file.write(utf8);
            file.write("\n");
        });
        emit ready();
        exec();
    }
    disconnect(this, &Logger::Log, nullptr, nullptr);
}
