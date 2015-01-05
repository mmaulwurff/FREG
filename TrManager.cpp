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
#include <QObject>
#include <QTranslator>
#include <QCoreApplication>

const TrManager * tr_manager;

TrManager::TrManager() :
        translator(LoadTranslator()),
        subNames(),
        kindNames(),
        dirNames {
            QObject::tr("Up"   ),
            QObject::tr("Down" ),
            QObject::tr("North"),
            QObject::tr("East" ),
            QObject::tr("South"),
            QObject::tr("West" )
        },
        shredTypeNames()
{
    #define X(column1, column2) column1,
    const char * const rawSubs[] = { SUB_TABLE };
    for (int i = 0; i < SUB_COUNT; ++i) {
        subNames[i] = QCoreApplication::translate("Block", rawSubs[i]);
    }
    const char * const rawKinds[] = { KIND_TABLE };
    for (int i = 0; i < KIND_COUNT; ++i) {
        kindNames[i] = QCoreApplication::translate("Block", rawKinds[i]);
    }
    #undef X

    const char * const rawShredTypes[] = {
        #define X(column1, column2, column3) column1,
        SHRED_TABLE
        #undef X
    };
    const char shredChars[] = {
        #define X(column1, column2, column3) column3,
        SHRED_TABLE
        #undef X
    };
    int index = 0;
    for (const char * raw : rawShredTypes) {
        shredTypeNames.insert(shredChars[index++],
            QCoreApplication::translate("Shred", raw));
    }
}

QTranslator * TrManager::LoadTranslator() const {
    QCoreApplication * const application = QCoreApplication::instance();
    QTranslator * translator = new QTranslator(application);
    translator->load(QString(":/freg_") + QLocale::system().name());
    application->installTranslator(translator);
    return translator;
}

QString TrManager::  DirName(const dirs dir) const { return  dirNames[ dir]; }
QString TrManager::  SubName(const int  sub) const { return  subNames[ sub]; }
QString TrManager:: KindName(const int kind) const { return kindNames[kind]; }

QString TrManager::SubNameUpper(const int sub) const {
    QString result = SubName(sub);
    return result.replace(0, 1, result.at(0).toUpper());
}

QString TrManager::ShredTypeName(const shred_type type) const {
    return shredTypeNames[type];
}
