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
#include "TrManager.h"

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QTextStream>

#include <cstdlib>

// Text::
CONSTRUCT_AS_PARENT(Text, Block)

QString Text::FullName() const {
    TrString paperName     = QObject::tr("Paper page");
    TrString glassName     = QObject::tr("Screen");
    TrString differentName = QObject::tr("Logger");

    switch ( Sub() ) {
    case PAPER:     return paperName;
    case GLASS:     return glassName;
    case DIFFERENT: return differentName;
    default:        return Block::FullName();
    }
}

usage_types Text::Use(Active* const who) {
    if ( noteId == 0 ) {
        TrString nothingString = QObject::tr("Nothing is written here.");
        who->ReceiveSignal(nothingString);
        return USAGE_TYPE_INNER;
    } else {
        if ( Sub() == DIFFERENT ) { // logger
            TrString usedString = QObject::tr("Used by ");
            writeLog(usedString + who->FullName() + Str("."));
            return USAGE_TYPE_NO;
        }
        return USAGE_TYPE_READ;
    }
}

bool Text::Inscribe(const QString& str) {
    if ( '.' != str.at(0).toLatin1() && (noteId == 0 || GLASS == Sub()) ) {
        Block::Inscribe(str);
        return true;
    } else {
        if ( Sub() == DIFFERENT ) {
            TrString inscribedString = QObject::tr("Inscribed: ");
            writeLog(inscribedString + str + Str("."));
        }
        return false;
    }
}

void Text::writeLog(const QString& string) const {
    QFile logFile(World::WorldPath() + Str("/texts/") +
        World::GetWorld()->GetNote(noteId));
    logFile.open(QIODevice::Append | QIODevice::Text);
    QTextStream(&logFile) << string << endl;
}

void Text::Damage(const int damage, const int damage_kind) {
    if ( Sub() != DIFFERENT ) return;
    if ( damage_kind == DAMAGE_TIME ) {
        Block::Damage(damage, damage_kind);
    }
    TrString damageString = QObject::tr("Received damage: %1 points, type: ");
    writeLog( damageString.arg(damage)
            + TrManager::GetDamageString(static_cast<damage_kinds>(damage_kind))
            + Str("."));
}

void Text::ReceiveSignal(const QString& string) {
    if ( Sub() != DIFFERENT ) return;
    TrString messageString = QObject::tr("Received message: \"");
    writeLog(messageString + string + Str("\"."));
}

QString Text::Description() const {
    TrString instruction = QObject::tr("Can be inscribed with file name in ");
    return instruction + World::WorldPath() + Str("/texts/.");
}

// Map::
wearable    Map::Wearable() const { return WEARABLE_OTHER; }
usage_types Map::UseOnShredMove(Active* const user) { return Use(user); }

usage_types Map::Use(Active* const user) {
    if ( user == nullptr ) return USAGE_TYPE_READ;
    if ( noteId == 0 ) {
        TrString titleString = QObject::tr("Set title to this map first.");
        user->ReceiveSignal(titleString);
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
    if (    ( std::labs(lati  - latiStart ) > border_dist ) ||
            ( std::labs(longi - longiStart) > border_dist ) )
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
    savedChar = World::GetCWorld()->GetMap()->TypeOfShred(longi, lati);
    map_file.seek(TEXT_WIDTH * (TEXT_WIDTH - 1));
    map_file.write(" @ = ");
    map_file.putChar(savedChar);
    return USAGE_TYPE_READ;
} // usage_types Map::Use(Active* const user)

void Map::SaveAttributes(QDataStream& out) const {
    out << longiStart << latiStart << savedShift << savedChar;
}

Map::Map(const kinds kind, const subs sub)
    : Text(kind, sub)
    , longiStart()
    , latiStart()
    , savedShift()
    , savedChar(0)
{}

Map::Map(QDataStream& str, const kinds kind, const subs sub)
    : Text(str, kind, sub)
    , longiStart()
    , latiStart()
    , savedShift()
    , savedChar()
{
    str >> longiStart >> latiStart >> savedShift >> savedChar;
}
