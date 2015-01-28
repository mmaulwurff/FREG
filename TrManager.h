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

#ifndef TRMANAGER_H
#define TRMANAGER_H

#include "header.h"
#include "Weather.h"

GCC_IGNORE_WEFFCPP_BEGIN
#include <QString>
#include <QHash>
GCC_IGNORE_WEFFCPP_END

class TrManager {
public:
    TrManager();

    /// Returs translated kind name.
    static QString KindName(int kind);
    /// Returns translated substance name.
    static QString SubName(int sub);
    /// Returns translated substance name with first upper letter.
    static QString SubNameUpper(int sub);

    /// Returns translated direction.
    static QString DirName(dirs);

    /// Returns translated shred type.
    static QString ShredTypeName(shred_type);

    /// If kind is unknown, returns "unknown_kind".
    static QString KindToString(int kind);
    /// If substance is unknown, returns "unknown_sub".
    static QString SubToString(int sub);
    /// If string is not convertible to kind, returns LAST_KIND.
    static int StrToKind(QString);
    /// If string is not convertible to substance, returns LAST_SUB.
    static int StrToSub(QString);

    /// Universal function, returns translated "off" or "on".
    static QString OffOn(bool on);

    /// Returns translated weather string.
    static QString GetWeatherString(weathers);

private:
    Q_DISABLE_COPY(TrManager)

    class QTranslator * LoadTranslator() const;

    class QTranslator * const translator;
    static const QByteArray rawKinds[KIND_COUNT];
    static const QByteArray rawSubs [SUB_COUNT ];
    QString subNames[SUB_COUNT];
    QString kindNames[KIND_COUNT];
    QHash<char, QString> shredTypeNames;

    static TrManager * trManager;
};

#endif // TRMANAGER_H
