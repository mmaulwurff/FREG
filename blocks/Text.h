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

#ifndef TEXT_H
#define TEXT_H

#include "blocks/Block.h"
#include <QtGlobal>

class Text : public Block {
public:
    using Block::Block;

    bool Inscribe(const QString&) override;
    QString FullName() const override;
    usage_types Use(Active* who) override;
};

class Map : public Text {
public:
    BLOCK_CONSTRUCTORS(Map)

    wearable Wearable() const override;
    usage_types Use(Active* user) override;
    usage_types UseOnShredMove(Active* user) override;

protected:
    void SaveAttributes(QDataStream& out) const override;

private:
    /// coordinates map titled in. also ~center.
    qint64 longiStart, latiStart;
    quint16 savedShift;
    qint8 savedChar;
};

#endif // TEXT_H

