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

#ifndef TR_MANAGER_H
#define TR_MANAGER_H

#include "header.h"
#include "Weather.h"
#include "Singleton.h"
#include <QHash>
#include <QString>

class TrManager : private Singleton<TrManager> {
public:
    TrManager();

    /// Returns translated kind name.
    static QString KindName(int kind);
    /// Returns translated substance name.
    static QString SubName(int sub);
    /// Returns translated substance name with first upper letter.
    static QString SubNameUpper(int sub);

    /// Returns translated direction.
    static QString DirName(dirs);

    /// Returns translated shred type.
    static QString ShredTypeName(shred_type);

    /// Returns untranslated (raw) kind name.
    static QString KindToString(int kind);
    /// Returns untranslated (raw) substance name.
    static QString SubToString(int sub);
    /// If string is not convertible to kind, returns LAST_KIND.
    static kinds StrToKind(const QString&);
    /// If string is not convertible to substance, returns LAST_SUB.
    static subs StrToSub(const QString&);

    /// Universal function, returns translated "off" or "on".
    static QString OffOn(bool on);

    /// Returns translated weather string.
    static QString GetWeatherString(weathers);

    static QString GetDamageString(damage_kinds);

private:

    Q_DISABLE_COPY(TrManager)

    class QTranslator* LoadTranslator() const;

    static QString Capitalized(const QString&);
    static QString Uncapitalized(const QString&);

    class QTranslator* const translator;
    static const QString rawKinds[KIND_COUNT], rawSubs [SUB_COUNT];
    QString             kindNames[KIND_COUNT], subNames[SUB_COUNT];
    QHash<char, QString> shredTypeNames;
};

#endif // TR_MANAGER_H
