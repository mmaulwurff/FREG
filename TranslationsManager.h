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

#ifndef TRANSLATIONSMANAGER_H
#define TRANSLATIONSMANAGER_H

#include "header.h"
#include <QString>

class QTranslator;

class TranslationsManager {
public:
    TranslationsManager();

    /// Returs translated kind name.
    QString KindName(int kind) const;
    /// Returns translated substance name.
    QString SubName(int sub) const;
    /// Returns translated substance name with first upper letter.
    QString SubNameUpper(int sub) const;

    /// Returns translated direction.
    QString DirString(dirs) const;

private:
    Q_DISABLE_COPY(TranslationsManager)

    QTranslator * LoadTranslator() const;

    QTranslator * const translator;
    const QString subNames[SUB_COUNT];
    const QString kindNames[KIND_COUNT];
    const QString dirNames[DIRS_COUNT];
};

extern const TranslationsManager * tr_manager;

#endif // TRANSLATIONSMANAGER_H
