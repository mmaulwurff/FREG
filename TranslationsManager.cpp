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

#include "TranslationsManager.h"
#include <QObject>
#include <QCoreApplication>

const TranslationsManager * tr_manager;

TranslationsManager::TranslationsManager() :
        translator(LoadTranslator()),
        subNames{
            QObject::tr("stone"),
            QObject::tr("stone"),
            QObject::tr("nullstone"),
            QObject::tr("air"),
            QObject::tr("air"),
            QObject::tr("diamond"),
            QObject::tr("soil"),
            QObject::tr("meat of rational"),
            QObject::tr("meat"),
            QObject::tr("glass"),
            QObject::tr("wood"),
            QObject::tr("different"),
            QObject::tr("iron"),
            QObject::tr("water"),
            QObject::tr("greenery"),
            QObject::tr("sand"),
            QObject::tr("hazelnut"),
            QObject::tr("rose"),
            QObject::tr("clay"),
            QObject::tr("air"),
            QObject::tr("paper"),
            QObject::tr("gold"),
            QObject::tr("bone"),
            QObject::tr("steel"),
            QObject::tr("adamantine"),
            QObject::tr("fire"),
            QObject::tr("coal"),
            QObject::tr("explosive"),
            QObject::tr("acid"),
            QObject::tr("cloud"),
            QObject::tr("dust"),
            QObject::tr("plastic"),
        },
        kindNames{
            QObject::tr("Block"),
            QObject::tr("Bell"),
            QObject::tr("Chest"),
            QObject::tr("Intellectual"),
            QObject::tr("Pick"),
            QObject::tr("Liquid"),
            QObject::tr("Plant"),
            QObject::tr("Bush"),
            QObject::tr("Herbivore"),
            QObject::tr("Falling"),
            QObject::tr("Clock"),
            QObject::tr("Plate"),
            QObject::tr("Anvil"),
            QObject::tr("Stick"),
            QObject::tr("Ladder"),
            QObject::tr("Door"),
            QObject::tr("Box"),
            QObject::tr("Sign"),
            QObject::tr("Map"),
            QObject::tr("Predator"),
            QObject::tr("Bucket"),
            QObject::tr("Shovel"),
            QObject::tr("Axe"),
            QObject::tr("Hammer"),
            QObject::tr("Illuminator"),
            QObject::tr("Rain Machine"),
            QObject::tr("Converter"),
            QObject::tr("Body armour"),
            QObject::tr("Helmet"),
            QObject::tr("Boots"),
            QObject::tr("Telegraph"),
            QObject::tr("Medkit"),
            QObject::tr("Filter"),
            QObject::tr("Informer"),
            QObject::tr("Teleport"),
            QObject::tr("Accumulator"),
        },
        dirNames{
            QObject::tr("Up"),
            QObject::tr("Down"),
            QObject::tr("North"),
            QObject::tr("South"),
            QObject::tr("East"),
            QObject::tr("West")
        }
{
    static_assert(sizeof_array(kindNames) == KIND_COUNT,
        "Invalid number of strings in tr_kind_names.");
    static_assert(sizeof_array(subNames)  == SUB_COUNT,
        "Invalid number of strings in tr_sub_names.");
}

QTranslator * TranslationsManager::LoadTranslator() const {
    QCoreApplication * const application = QCoreApplication::instance();
    QTranslator * translator = new QTranslator(application);
    translator->load(QString(":/freg_") + locale);
    application->installTranslator(translator);
    return translator;
}

QString TranslationsManager::SubName(const int sub) const {
    return subNames[sub];
}

QString TranslationsManager::KindName(const int kind) const {
    return kindNames[kind];
}

QString TranslationsManager::SubNameUpper(const int sub) const {
    QString result = SubName(sub);
    return result.replace(0, 1, result.at(0).toUpper());
}

QString TranslationsManager::DirString(const dirs dir) const {
    return dirNames[dir];
}

