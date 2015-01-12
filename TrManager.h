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
#include <QString>
#include <QHash>

class QTranslator;

class TrManager {
public:
    TrManager();

    /// Returs translated kind name.
    QString KindName(int kind) const;
    /// Returns translated substance name.
    QString SubName(int sub) const;
    /// Returns translated substance name with first upper letter.
    QString SubNameUpper(int sub) const;

    /// Returns translated direction.
    static QString DirName(dirs);

    /// Returns translated shred type.
    QString ShredTypeName(shred_type) const;

    /// If kind is unknown, returns "unknown_kind".
    static QString KindToString(int kind);
    /// If substance is unknown, returns "unknown_sub".
    static QString SubToString(int sub);
    /// If string is not convertible to kind, returns LAST_KIND.
    static int StringToKind(QString);
    /// If string is not convertible to substance, returns LAST_SUB.
    static int StringToSub(QString);

    /// Universal function, returns translated "off" or "on".
    static QString OffOn(bool on);

    /// Returns translated weather string.
    static QString GetWeatherString(weathers);

private:
    Q_DISABLE_COPY(TrManager)

    QTranslator * LoadTranslator() const;

    QTranslator * const translator;
    static const QByteArray rawKinds[];
    static const QByteArray rawSubs [];
    QString subNames[SUB_COUNT];
    QString kindNames[KIND_COUNT];
    QHash<char, QString> shredTypeNames;
};

extern const TrManager * tr_manager;

#endif // TRMANAGER_H
