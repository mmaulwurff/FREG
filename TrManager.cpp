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

#include "TrManager.h"

#include <QLocale>
#include <QCoreApplication>
#include <QObject>
#include <QTranslator>
#include <QDebug>

const QString TrManager::rawKinds[] = { KIND_TABLE(X_STRING) };
const QString TrManager::rawSubs [] = {  SUB_TABLE(X_STRING) };

TrManager* TrManager::trManager = nullptr;

TrManager::TrManager()
    : translator(LoadTranslator())
    , kindNames()
    , subNames()
    , shredTypeNames()
{
    Q_ASSERT(trManager == nullptr);
    trManager = this;

    for (int i = 0; i < SUB_COUNT; ++i) {
        subNames[i] =
            QCoreApplication::translate("Block", rawSubs[i].toLatin1());
    }
    for (int i = 0; i < KIND_COUNT; ++i) {
        kindNames[i] =
            QCoreApplication::translate("Block", rawKinds[i].toLatin1());
    }

    const QString rawShredTypes[] = { SHRED_TABLE(X_STRING) };
    const char shredChars[] = { SHRED_TABLE(X_CHAR) };

    int index = 0;
    for (const QString& raw : rawShredTypes) {
        shredTypeNames.insert(shredChars[index++],
            QCoreApplication::translate("Shred", raw.toLatin1()));
    }
}

QTranslator* TrManager::LoadTranslator() const {
    QCoreApplication* const application = QCoreApplication::instance();
    QTranslator* const new_translator = new QTranslator(application);
    new_translator->load(Str(":/freg_") + QLocale::system().name());
    application->installTranslator(new_translator);
    return new_translator;
}

QString TrManager::  SubName(const int  sub) {
    return trManager->subNames[sub];
}

QString TrManager:: KindName(const int kind) {
    return trManager->kindNames[kind];
}

QString TrManager::KindToString(const int kind) { return rawKinds[kind]; }
QString TrManager:: SubToString(const int sub ) { return  rawSubs[sub ]; }

QString TrManager::OffOn(const bool on) {
    static const QString offOn[] = {
        QCoreApplication::translate("Global", "off"),
        QCoreApplication::translate("Global", "on")
    };
    return offOn[on];
}

QString TrManager::DirName(const dirs dir) {
    static const QString dirNames[] = {
        QCoreApplication::translate("Global", "Up"   ),
        QCoreApplication::translate("Global", "Down" ),
        QCoreApplication::translate("Global", "North"),
        QCoreApplication::translate("Global", "East" ),
        QCoreApplication::translate("Global", "South"),
        QCoreApplication::translate("Global", "West" )
    };
    return dirNames[dir];
}

QString TrManager::GetWeatherString(const weathers weather) {
    static const QString weatherStrings[] = {
        QCoreApplication::translate("Global", "Clear"),
        QCoreApplication::translate("Global", "Rain"),
        QCoreApplication::translate("Global", "Rain"),
        QCoreApplication::translate("Global", "Clouds")
    };
    return weatherStrings[weather];
}

QString TrManager::GetDamageString(const damage_kinds damage_kind) {
    static const QString damageNames[] = {
        QCoreApplication::translate("Global", "mining"   ),
        QCoreApplication::translate("Global", "digging"  ),
        QCoreApplication::translate("Global", "cutting"  ),
        QCoreApplication::translate("Global", "thrusting"),
        QCoreApplication::translate("Global", "crushing" ),
        QCoreApplication::translate("Global", "heating"  ),
        QCoreApplication::translate("Global", "freezing" ),
        QCoreApplication::translate("Global", "electricity"),
        QCoreApplication::translate("Global", "corrode"  ),
        QCoreApplication::translate("Global", "hunger"   ),
        QCoreApplication::translate("Global", "breathing"),
        QCoreApplication::translate("Global", "biting"   ),
        QCoreApplication::translate("Global", "time"     ),
        QCoreApplication::translate("Global", "pushing"  )
    };
    QStringList result;
    for (int i = 1, c = 0; i < DAMAGE_PUSH_UP; i <<= 1, ++c) {
        if (damage_kind & i) {
            result.append(damageNames[c]);
        }
    }
    if (damage_kind & DAMAGE_PUSH_ANYWHERE) {
        result.append(damageNames[sizeofArray(damageNames) - 1]);
    }
    return result.join(Str(", "));
}

QString& TrManager::Capitalized(QString& str) {
    return str.replace(0, 1, str.at(0).toUpper());
}

QString TrManager::SubNameUpper(const int sub) {
    QString result = trManager->SubName(sub);
    return Capitalized(result);
}

QString TrManager::ShredTypeName(const shred_type type) {
    return trManager->shredTypeNames[type];
}

kinds TrManager::StrToKind(QString str) {
    return static_cast<kinds>(
        std::find(ALL(rawKinds), Capitalized(str)) - rawKinds );
}

subs TrManager::StrToSub(const QString str) {
    return static_cast<subs>(std::find(ALL(rawSubs), str) - rawSubs);
}
