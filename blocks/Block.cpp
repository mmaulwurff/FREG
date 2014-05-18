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

#include "blocks/Block.h"
#include "BlockManager.h"

QString Block::FullName() const {
    switch ( Sub() ) {
    case STAR: case SUN_MOON: case SKY:
    case AIR:        return QObject::tr("Air");
    case WATER:      return QObject::tr("Ice");
    case STONE:      return QObject::tr("Stone");
    case MOSS_STONE: return QObject::tr("Moss stone");
    case NULLSTONE:  return QObject::tr("Nullstone");
    case GLASS:      return QObject::tr("Glass");
    case SOIL:       return QObject::tr("Soil");
    case HAZELNUT:   return QObject::tr("Hazelnut");
    case WOOD:       return QObject::tr("Wood");
    case GREENERY:   return QObject::tr("Leaves");
    case ROSE:       return QObject::tr("Rose");
    case A_MEAT:     return QObject::tr("Animal meat");
    case H_MEAT:     return QObject::tr("Not animal meat");
    case IRON:       return QObject::tr("Iron block");
    case SAND:       return QObject::tr("Sandstone");
    case CLAY:       return QObject::tr("Clay brick");
    case GOLD:       return QObject::tr("Block of gold");
    case COAL:       return QObject::tr("Block of coal");
    default: fprintf(stderr, "Block::FullName: sub ?: %d.\n", Sub());
        return "Unknown block";
    }
}

quint8 Block::Transparency(const quint8 transp, const int sub) const {
    if ( UNDEF == transp ) {
        switch ( sub ) {
        case AIR:   return INVISIBLE;
        case WATER:
        case GREENERY:
        case GLASS: return BLOCK_TRANSPARENT;
        default:    return BLOCK_OPAQUE;
        }
    } else {
        return transp;
    }
}

void Block::Damage(const ushort dmg, const int dmg_kind) {
    if ( dmg_kind == NO_HARM ) return;
    ushort mult = 1; // default
    switch ( Sub() ) {
    case DIFFERENT:
        if ( TIME == dmg_kind ) {
            durability = 0;
            return;
        }
        // no break
    case NULLSTONE:
    case STAR:
    case AIR:
    case SKY:
    case SUN_MOON: return;
    case WATER: mult = ( HEAT==dmg_kind || TIME==dmg_kind ); break;
    case MOSS_STONE:
    case STONE: switch ( dmg_kind ) {
        case CRUSH:
        case DAMAGE_HANDS:
        case CUT:   return;
        case MINE:  mult = 2; break;
    } break;
    case GREENERY:
    case HAZELNUT:
    case GLASS: durability = 0; return;
    case WOOD: switch ( dmg_kind ) {
        default:  mult = 1; break;
        case CUT: mult = 2; break;
        case DAMAGE_HANDS: return;
    } break;
    case SAND:
    case A_MEAT:
    case H_MEAT: ++(mult = (THRUST==dmg_kind)); break;
    case SOIL: switch ( dmg_kind ) {
        case DIG: mult = 2;
        case DAMAGE_FALL: return;
    } break;
    case FIRE: mult = (FREEZE==dmg_kind || TIME==dmg_kind); break;
    }
    durability -= mult*dmg;
}

Block * Block::DropAfterDamage() { return GLASS==Sub() ? nullptr : this; }

int  Block::PushResult(int) const {
    return ( AIR==Sub() ) ? ENVIRONMENT : NOT_MOVABLE;
}

quint8 Block::Kind() const { return BLOCK; }
quint16 Block::GetId() const { return id; }
bool Block::Catchable() const { return false; }
void Block::Push(const int, Block * const) {}
bool Block::Move(const int) { return false; }
usage_types Block::Use(Block *) { return USAGE_TYPE_NO; }
int  Block::Wearable() const { return WEARABLE_NOWHERE; }
int  Block::DamageKind() const { return CRUSH; }
ushort Block::DamageLevel() const { return 1; }
uchar Block::LightRadius() const { return 0; }
void Block::ReceiveSignal(const QString) {}

bool Block::Inscribe(const QString str) {
    if ( note ) {
        *note = str.left(MAX_NOTE_LENGTH);
    } else {
        note = new QString(str.left(MAX_NOTE_LENGTH));
    }
    if ( "" == *note ) {
        delete note;
        note = 0;
    }
    return true;
}

Inventory * Block::HasInventory() { return 0; }
Animal * Block::IsAnimal() { return 0; }
Active * Block::ActiveBlock() { return 0; }

void Block::Restore() { durability = MAX_DURABILITY; }
void Block::Break() { durability = 0; }
int  Block::GetDir() const { return direction; }
int  Block::Sub() const { return sub; }
int  Block::Transparent() const { return transparent; }
short Block::GetDurability() const { return durability; }
QString Block::GetNote() const { return note ? *note : ""; }

void Block::Mend() {
    if ( GetDurability() < MAX_DURABILITY ) {
        ++durability;
    }
}

int Block::Temperature() const {
    switch ( Sub() ) {
    case WATER: return -100;
    case FIRE:  return   50;
    default:    return    0;
    }
}

ushort Block::Weight() const {
    switch ( Sub() ) {
    case NULLSTONE: return WEIGHT_NULLSTONE;
    case SAND:      return WEIGHT_SAND;
    case SOIL:      return WEIGHT_SAND+WEIGHT_WATER;
    case GLASS:     return WEIGHT_GLASS;
    case WOOD:      return WEIGHT_WATER-1;
    case IRON:      return WEIGHT_IRON;
    case GREENERY:  return WEIGHT_GREENERY;
    case PAPER:
    case ROSE:
    case HAZELNUT:  return WEIGHT_MINIMAL;
    case MOSS_STONE:
    case STONE:     return WEIGHT_STONE;
    case A_MEAT:
    case H_MEAT:    return WEIGHT_WATER-10;
    case SKY:
    case STAR:
    case SUN_MOON:
    case FIRE:
    case DIFFERENT:
    case AIR:       return WEIGHT_AIR;
    default:        return WEIGHT_WATER;
    }
}

void Block::SetDir(const int dir) {
    if ( BLOCK!=Kind() || WOOD==Sub() ) {
        direction = dir;
    }
}

bool Block::operator!=(const Block & block) const { return !(*this == block); }

bool Block::operator==(const Block & block) const {
    return ( block.GetId()==GetId() &&
        block.GetDir()==GetDir() &&
        block.GetDurability()==GetDurability() &&
        ( (!note && !block.note) ||
            (note && block.note && *block.note==*note) ) );
}

void Block::SaveAttributes(QDataStream &) const {}

void Block::SaveToFile(QDataStream & out) const {
    if ( this == block_manager.NormalBlock(sub) ) {
        out << quint8( 0x80 | sub );
    } else {
        quint16 data = direction;
        out << sub << BlockManager::KindFromId(GetId()) <<
            ( ( ( ( data
            <<= 7 ) |= durability )
            <<= 1 ) |= !!note );
        if ( Q_UNLIKELY(note) ) {
            out << *note;
        }
        SaveAttributes(out);
    }
}

Block::Block(const int subst, const quint16 i, const quint8 transp) :
        note(0),
        durability(MAX_DURABILITY),
        transparent(Transparency(transp, subst)),
        sub(subst),
        id(i),
        direction(UP)
{}

Block::Block(QDataStream & str, const int subst, const quint16 i,
        const quint8 transp)
    :
        note(0),
        transparent(Transparency(transp, subst)),
        sub(subst),
        id(i)
{
    quint16 data;
    str >> data;
    if ( Q_UNLIKELY(data & 1) ) {
        str >> *(note = new QString);
    }
    durability = ( data >>=1 ) & 0x7F;
    direction = (data >>= 7);
}

Block::Block(Block const & block) :
        Block(block.sub, block.id, block.transparent)
{}

Block::~Block() { delete note; }
