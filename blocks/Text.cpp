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

#include "blocks/Text.h"
#include "blocks/Active.h"
#include "WorldMap.h"
#include "World.h"
#include "Shred.h"
#include <QObject>
#include <QFile>
#include <QDataStream>

// Text::
QString Text::FullName() const {
    switch ( Sub() ) {
    case PAPER: return QObject::tr("Paper page");
    case GLASS: return QObject::tr("Screen");
    default:    return Block::FullName();
    }
}

usage_types Text::Use(Active* const who) {
    if ( noteId == 0 ) {
        who->ReceiveSignal(QObject::tr("Nothing is written here."));
        return USAGE_TYPE_INNER;
    } else {
        return USAGE_TYPE_READ;
    }
}

bool Text::Inscribe(const QString str) {
    if ( '.' != str.at(0).toLatin1() && (noteId == 0 || GLASS == Sub()) ) {
        Block::Inscribe(str);
        return true;
    } else {
        return false;
    }
}

// Map::
wearable    Map::Wearable() const { return WEARABLE_OTHER; }
usage_types Map::UseOnShredMove(Active* const user) { return Use(user); }

usage_types Map::Use(Active* const user) {
    if ( user == nullptr ) return USAGE_TYPE_READ;
    if ( noteId == 0 ) {
        user->ReceiveSignal(QObject::tr("Set title to this map first."));
        return USAGE_TYPE_INNER;
    } // else:
    QFile map_file(World::WorldPath() + Str("/texts/") + GetNote());
    if ( not map_file.open(QIODevice::ReadWrite | QIODevice::Text) ) {
        return USAGE_TYPE_READ;
    }
    const qint64  lati = user->GetShred()->Latitude();
    const qint64 longi = user->GetShred()->Longitude();
    const int TEXT_WIDTH = 31 + 1;
    if ( 0 == map_file.size() ) { // new map
        const char header[TEXT_WIDTH+1] = "+-----------------------------+\n";
        const char   body[TEXT_WIDTH+1] = "|                             |\n";
        map_file.write(header, TEXT_WIDTH);
        for (int i=0; i<TEXT_WIDTH-3; ++i) {
            map_file.write(body, TEXT_WIDTH);
        }
        map_file.write(header, TEXT_WIDTH);
        longiStart = longi;
        latiStart  = lati;
    }
    const int border_dist = (TEXT_WIDTH - 1) / 2;
    if (    ( labs(lati  - latiStart ) > border_dist ) ||
            ( labs(longi - longiStart) > border_dist ) )
    {
        return USAGE_TYPE_READ;
    }
    if ( savedChar ) {
        map_file.seek(savedShift);
        map_file.putChar(savedChar);
    }
    map_file.seek( savedShift = TEXT_WIDTH *
        (longi - longiStart + border_dist ) +
         lati  - latiStart  + border_dist );
    map_file.putChar('@');
    savedChar = World::GetConstWorld()->GetMap()->TypeOfShred(longi, lati);
    map_file.seek(TEXT_WIDTH * (TEXT_WIDTH - 1));
    map_file.write(" @ = ");
    map_file.putChar(savedChar);
    return USAGE_TYPE_READ;
} // usage_types Map::Use(Active* const user)

void Map::SaveAttributes(QDataStream& out) const {
    out << longiStart << latiStart << savedShift << savedChar;
}

Map::Map(const kinds kind, const subs sub) :
        Text(kind, sub),
        longiStart(),
        latiStart(),
        savedShift(),
        savedChar(0)
{}

Map::Map(QDataStream& str, const kinds kind, const subs sub) :
        Text(str, kind, sub),
        longiStart(),
        latiStart(),
        savedShift(),
        savedChar()
{
    str >> longiStart >> latiStart >> savedShift >> savedChar;
}
