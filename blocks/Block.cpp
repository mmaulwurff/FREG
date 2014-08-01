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
#include "World.h"
#include "Inventory.h"

dirs Block::MakeDirFromDamage(const int dmg_kind) {
    Q_ASSERT(dmg_kind >= DAMAGE_PUSH_UP);
    return static_cast<dirs>(dmg_kind - DAMAGE_PUSH_UP);
}

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

void Block::Damage(const int dmg, int dmg_kind) {
    if ( dmg_kind > DAMAGE_PUSH_UP ) {
         dmg_kind = DAMAGE_PUSH_UP;
    }
    switch ( Sub() ) {
    case SUB_DUST:
    case GREENERY:
    case ROSE:
    case HAZELNUT: durability = 0; return;
    case NULLSTONE:
    case STAR:
    case AIR:
    case SKY:
    case SUN_MOON: return;
    }
    switch ( dmg_kind ) {
    case DAMAGE_NO: return;
    case DAMAGE_TIME: durability -= dmg; return;
    case DAMAGE_ACID:
        switch ( Sub() ) {
        default: durability -= 2 * dmg; return;
        case DIFFERENT: durability = 0; return;
        case ACID:
        case ADAMANTINE:
        case IRON:
        case GLASS: return;
        }
    }
    int mult = 1; // default
    switch ( Sub() ) {
    case DIFFERENT: return;
    case MOSS_STONE:
    case STONE: switch ( dmg_kind ) {
        case DAMAGE_HEAT:
        case DAMAGE_HANDS:
        case DAMAGE_PUSH_UP:
        case DAMAGE_CUT: return;
        case DAMAGE_MINE: mult = 2; break;
        } break;
    case WOOD: switch ( dmg_kind ) {
        case DAMAGE_CUT: mult = 2; break;
        case DAMAGE_PUSH_UP:
        case DAMAGE_HANDS: return;
        } break;
    case A_MEAT:
    case H_MEAT:
        mult += (DAMAGE_THRUST==dmg_kind || DAMAGE_HEAT==dmg_kind);
        break;
    case SAND:
    case SOIL:      mult += ( DAMAGE_DIG    == dmg_kind ); break;
    case ADAMANTINE:
    case FIRE:      mult  = ( DAMAGE_FREEZE == dmg_kind ); break;
    case WATER:     mult  = ( DAMAGE_HEAT   == dmg_kind ); break;
    case GLASS: switch ( dmg_kind ) {
        case DAMAGE_PUSH_UP: durability -= dmg*((MAX_DURABILITY+9)/10); return;
        default:             durability  = 0; // no break;
        case DAMAGE_HEAT: return;
        }
    case IRON: switch ( dmg_kind ) {
        case DAMAGE_HANDS:
        case DAMAGE_PUSH_UP: return;
        }
    }
    durability -= mult*dmg;
}

Block * Block::DropAfterDamage(bool * const delete_block) {
    switch ( Sub() ) {
    case SUB_DUST:
    case GLASS:
    case AIR: return block_manager.Normal(AIR);
    case STONE: if ( BLOCK==Kind() ) {
        return BlockManager::NewBlock(LADDER, STONE);
    } // no break;
    default: {
        Block * const pile = BlockManager::NewBlock(CONTAINER, DIFFERENT);
        pile->HasInventory()->Get(this);
        *delete_block = false;
        return pile;
    }
    }
}

push_reaction Block::PushResult(dirs) const {
    return ( AIR==Sub() ) ? ENVIRONMENT : NOT_MOVABLE;
}

int  Block::GetId() const { return BlockManager::MakeId(Kind(), Sub()); }
bool Block::Catchable() const { return false; }
void Block::Move(dirs) {}
int  Block::Wearable() const { return WEARABLE_NOWHERE; }
int  Block::DamageKind() const { return DAMAGE_CRUSH; }
int  Block::DamageLevel() const { return 1; }
int  Block::LightRadius() const { return 0; }
void Block::ReceiveSignal(QString) {}
usage_types Block::Use(Block *) { return USAGE_TYPE_NO; }

bool Block::Inscribe(const QString str) {
    if ( Sub() == AIR ) return false;
    noteId = ( noteId == 0 ) ? // new note
        world->SetNote(str.left(MAX_NOTE_LENGTH)) :
        world->ChangeNote(str.left(MAX_NOTE_LENGTH), noteId);
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

QString Block::GetNote() const {
    return (noteId == 0) ? QString() : world->GetNote(noteId);
}

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
    case SUB_DUST:  return WEIGHT_MINIMAL;
    }
}

void Block::SetDir(const int dir) {
    if ( BLOCK!=Kind() || WOOD==Sub() ) {
        direction = dir;
    }
}

bool Block::operator!=(const Block & block) const { return !(*this == block); }

bool Block::operator==(const Block & block) const {
    return ( block.Kind() == Kind()
        && block.Sub() == Sub()
        && block.GetDurability() == GetDurability()
        && block.GetDir() == GetDir()
        && ( (block.noteId == 0 && noteId == 0)
            || world->GetNote(block.noteId) == world->GetNote(noteId) ) );
}

void Block::SaveAttributes(QDataStream &) const {}

void Block::SaveToFile(QDataStream & out) {
    if ( this == block_manager.Normal(sub) ) {
        SaveNormalToFile(out);
    } else {
        out << sub << kind <<
            ( ( ( ( durability
            <<= 3 ) |= direction )
            <<= 1 ) |= (noteId != 0) );
        if ( Q_UNLIKELY(noteId != 0) ) {
            out << noteId;
        }
        SaveAttributes(out);
    }
}

void Block::SaveNormalToFile(QDataStream & out) const {
    out << quint8( 0x80 | sub );
}

void Block::RestoreDurabilityAfterSave() { durability >>= 4; }

Block::Block(const int kind_, const int sub_, const int transp) :
        noteId(0),
        durability(MAX_DURABILITY),
        transparent(Transparency(transp, sub_)),
        kind(kind_),
        sub(sub_),
        direction(UP)
{}

Block::Block(QDataStream & str, const int kind_, const int sub_,
        const int transp)
    :
        noteId(),
        durability(),
        transparent(Transparency(transp, sub_)),
        kind(kind_),
        sub(sub_),
        direction()
{
    // use durability as buffer, set actual value in the end:
    str >> durability;
    if ( Q_UNLIKELY(durability & 1) ) {
        str >> noteId;
    }
    direction = ( durability >>= 1 ) & 0x7;
    durability >>= 3;
}

Block::~Block() {}
