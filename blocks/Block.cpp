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
#include "Inventory.h"

QString Block::FullName() const {
    switch ( Sub() ) {
    case STAR:
    case SUN_MOON:
    case SKY:
    case AIR:        return QObject::tr("Air");
    case STONE:      return QObject::tr("Stone");
    case SOIL:       return QObject::tr("Soil");
    case WATER:      return QObject::tr("Ice");
    case MOSS_STONE: return QObject::tr("Moss stone");
    case NULLSTONE:  return QObject::tr("Nullstone");
    case GLASS:      return QObject::tr("Glass");
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
    case ACID:       return QObject::tr("Acid concentrate");
    default: fprintf(stderr, "%s: sub ?: %d.\n", Q_FUNC_INFO, Sub());
        return "Unknown block";
    }
}

int Block::Transparency(const int transp, const int sub) const {
    if ( UNDEF == transp ) {
        switch ( sub ) {
        default:    return BLOCK_OPAQUE;
        case AIR:   return INVISIBLE;
        case WATER:
        case GREENERY:
        case ACID:
        case SUB_CLOUD:
        case GLASS: return BLOCK_TRANSPARENT;
        }
    } else {
        return transp;
    }
}

void Block::Damage(const int dmg, const int dmg_kind) {
    switch ( Sub() ) {
    case GREENERY:
    case HAZELNUT: durability = 0; return;
    case NULLSTONE:
    case STAR:
    case AIR:
    case SKY:
    case SUN_MOON: return;
    }
    switch ( dmg_kind ) {
    case NO_HARM: return;
    case TIME: durability -= dmg; return;
    case DAMAGE_ACID:
        switch ( Sub() ) {
        default: durability -= 2 * dmg; return;
        case DIFFERENT: durability = 0; return;
        case IRON:
        case GLASS: return;
        }
    }
    int mult = 1; // default
    switch ( Sub() ) {
    case DIFFERENT: return;
    case MOSS_STONE:
    case STONE: switch ( dmg_kind ) {
        case HEAT:
        case DAMAGE_HANDS:
        case CUT:   return;
        case MINE:  mult = 2; break;
        } break;
    case WOOD: switch ( dmg_kind ) {
        case CUT: mult = 2; break;
        case DAMAGE_HANDS: return;
        } break;
    case A_MEAT:
    case H_MEAT:    mult += (THRUST==dmg_kind || HEAT==dmg_kind); break;
    case SAND:
    case SOIL:      mult += ( DIG    == dmg_kind ); break;
    case FIRE:      mult  = ( FREEZE == dmg_kind ); break;
    case WATER:     mult  = ( HEAT   == dmg_kind ); break;
    case GLASS:     mult  = ( HEAT   != dmg_kind ); break;
    case IRON:      mult  = ( DAMAGE_HANDS != dmg_kind ); break;
    }
    durability -= mult*dmg;
}

Block * Block::DropAfterDamage(bool * const delete_block) {
    switch ( Sub() ) {
    case GLASS:
    case AIR: return block_manager.NormalBlock(AIR);
    case STONE: if ( BLOCK==Kind() ) {
        return block_manager.NewBlock(LADDER, STONE);
    } // no break;
    default: {
        Block * const pile = block_manager.NewBlock(CONTAINER, DIFFERENT);
        pile->HasInventory()->Get(this);
        *delete_block = false;
        return pile;
    }
    }
}

push_reaction Block::PushResult(dirs) const {
    return ( AIR==Sub() ) ? ENVIRONMENT : NOT_MOVABLE;
}

int  Block::Kind() const { return BLOCK; }
int  Block::GetId() const { return id; }
bool Block::Catchable() const { return false; }
void Block::Push(dirs, Block *) {}
void Block::Move(dirs) {}
usage_types Block::Use(Block *) { return USAGE_TYPE_NO; }
int  Block::Wearable() const { return WEARABLE_NOWHERE; }
int  Block::DamageKind() const { return CRUSH; }
int  Block::DamageLevel() const { return 1; }
int  Block::LightRadius() const { return 0; }
void Block::ReceiveSignal(QString) {}

bool Block::Inscribe(QString str) {
    if ( note ) {
        *note = str.left(MAX_NOTE_LENGTH);
    } else {
        note = new QString(str.left(MAX_NOTE_LENGTH));
    }
    if ( "" == *note ) {
        delete note;
        note = nullptr;
    }
    return true;
}

Inventory * Block::HasInventory() { return nullptr; }
Animal * Block::IsAnimal() { return nullptr; }
Active * Block::ActiveBlock() { return nullptr; }
Falling * Block::ShouldFall() { return nullptr; }

void Block::Restore() { durability = MAX_DURABILITY; }
void Block::Break()   { durability = 0; }
dirs Block::GetDir() const { return static_cast<dirs>(direction); }
int  Block::GetDurability() const { return durability; }
QString Block::GetNote() const { return note ? *note : ""; }

void Block::Mend() {
    if ( GetDurability() < MAX_DURABILITY ) {
        ++durability;
    }
}

int Block::Weight() const {
    switch ( Sub() ) {
    default:        return WEIGHT_WATER;
    case SUB_CLOUD:
    case AIR:       return WEIGHT_AIR;
    case STONE:     return WEIGHT_STONE;
    case SOIL:      return WEIGHT_SAND+WEIGHT_WATER;
    case GREENERY:  return WEIGHT_GREENERY;
    case NULLSTONE: return WEIGHT_NULLSTONE;
    case SAND:      return WEIGHT_SAND;
    case GLASS:     return WEIGHT_GLASS;
    case WOOD:      return WEIGHT_WATER-1;
    case IRON:      return WEIGHT_IRON;
    case PAPER:
    case ROSE:
    case HAZELNUT:  return WEIGHT_MINIMAL;
    case MOSS_STONE:
    case A_MEAT:
    case H_MEAT:    return WEIGHT_WATER-10;
    case SKY:
    case STAR:
    case SUN_MOON:
    case FIRE:
    case DIFFERENT: return WEIGHT_WATER;
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
        block.GetDurability()==GetDurability() &&
        block.GetDir()==GetDir() &&
        ( (!note && !block.note) ||
            (note && block.note && *block.note==*note) ) );
}

void Block::SaveAttributes(QDataStream &) const {}

void Block::SaveToFile(QDataStream & out) {
    if ( this == block_manager.NormalBlock(sub) ) {
        SaveNormalToFile(out);
    } else {
        out << sub << quint8(BlockManager::KindFromId(id)) <<
            ( ( ( ( durability
            <<= 3 ) |= direction )
            <<= 1 ) |= bool(note) );
        if ( Q_UNLIKELY(note) ) {
            out << *note;
        }
        SaveAttributes(out);
    }
}

void Block::SaveNormalToFile(QDataStream & out) const {
    out << quint8( 0x80 | sub );
}

void Block::RestoreDurabilityAfterSave() { durability >>= 4; }

Block::Block(const int subst, const int i, const int transp) :
        note(nullptr),
        durability(MAX_DURABILITY),
        transparent(Transparency(transp, subst)),
        sub(subst),
        id(i),
        direction(UP)
{}

Block::Block(QDataStream & str, const int subst, const int i, const int transp)
    :
        transparent(Transparency(transp, subst)),
        sub(subst),
        id(i)
{
    // use durability as buffer, set actual value in the end:
    str >> durability;
    if ( Q_UNLIKELY(durability & 1) ) {
        str >> *(note = new QString);
    } else {
        note = nullptr;
    }
    direction = ( durability >>= 1 ) & 0x7;
    durability >>= 3;
}

Block::~Block() { delete note; }
