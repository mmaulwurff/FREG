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

#include "blocks/Block.h"
#include "BlockFactory.h"
#include "World.h"
#include "Inventory.h"

dirs Block::MakeDirFromDamage(const int dmg_kind) {
    Q_ASSERT(dmg_kind >= DAMAGE_PUSH_UP);
    return static_cast<dirs>(dmg_kind - DAMAGE_PUSH_UP);
}

QString Block::FullName() const {
    if ( Kind() ==  BLOCK ) {
        switch ( Sub() ) {
        default:    return TrManager::SubNameUpper(Sub());
        case WATER: return QObject::tr("Ice");
        case SAND:  return QObject::tr("Sandstone");
        case CLAY:  return QObject::tr("Clay brick");
        case ACID:  return QObject::tr("Acid concentrate");
        case IRON:
        case GOLD:
        case COAL: break;
        }
    }
    return QStringLiteral("%1 (%2)").
        arg(TrManager::KindName(Kind())).
        arg(TrManager::SubName(Sub()));
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
    // Fragile or undestructible substances:
    switch ( Sub() ) {
    case SUB_DUST:
    case GREENERY:
    case ROSE:
    case SUB_NUT: durability = 0; return;
    case NULLSTONE:
    case STAR:
    case AIR:
    case SKY: return;
    }
    // Special damage kinds:
    switch ( dmg_kind ) {
    case DAMAGE_PUSH_UP: switch ( Sub() ) {
        default: return;
        case A_MEAT:
        case H_MEAT: durability -= dmg * ( dmg > 10 ); return;
        case GLASS:  durability -= dmg * (MAX_DURABILITY+9) / 10; return;
        } break;
    case DAMAGE_HANDS: switch ( Sub() ) {
        default: return;
        case SOIL:
        case A_MEAT:
        case H_MEAT: break;
        } break;
    case DAMAGE_ULTIMATE: durability = 0; return;
    case DAMAGE_INVENTORY_ACTION:
    case DAMAGE_NO: return;
    case DAMAGE_TIME: durability -= dmg; return;
    case DAMAGE_ACID:
        switch ( Sub() ) {
        default: durability -= 2 * dmg; return;
        case DIFFERENT: durability = 0; return;
        case ACID:
        case ADAMANTINE:
        case GLASS: return;
        } break;
    }
    // Common substances and damage kinds:
    int mult = 1; // default
    switch ( Sub() ) {
    case DIFFERENT: return;
    case MOSS_STONE:
    case STONE: switch ( dmg_kind ) {
        case DAMAGE_HEAT:
        case DAMAGE_CUT: return;
        case DAMAGE_MINE: mult = 2; break;
        } break;
    case WOOD: mult += ( DAMAGE_CUT == dmg_kind ); break;
    case A_MEAT:
    case H_MEAT: switch ( dmg_kind ) {
        case DAMAGE_HEAT:
        case DAMAGE_THRUST: mult = 2; break;
        } break;
    case SAND:
    case SOIL:  mult += ( DAMAGE_DIG    == dmg_kind ); break;
    case ADAMANTINE:
    case FIRE:  mult  = ( DAMAGE_FREEZE == dmg_kind ); break;
    case WATER: mult  = ( DAMAGE_HEAT   == dmg_kind ); break;
    case GLASS: durability *= ( DAMAGE_HEAT == dmg_kind ); return;
    }
    durability -= mult * dmg;
} // Block::Damage(const ind dmg, const int dmg_kind)

Block * Block::DropAfterDamage(bool * const delete_block) {
    switch ( Sub() ) {
    case GREENERY:
    case SUB_DUST:
    case GLASS:
    case AIR: return BlockFactory::Normal(AIR);
    case STONE: if ( BLOCK==Kind() ) {
        return BlockFactory::NewBlock(LADDER, STONE);
    } // no break;
    default: {
        Block * const pile = BlockFactory::NewBlock(BOX, DIFFERENT);
        pile->HasInventory()->Get(this);
        *delete_block = false;
        return pile;
    }
    }
}

push_reaction Block::PushResult(dirs) const {
    return ( AIR==Sub() ) ? ENVIRONMENT : NOT_MOVABLE;
}

bool Block::Catchable() const { return false; }
void Block::Move(dirs) {}
int  Block::DamageKind() const { return DAMAGE_CRUSH; }
int  Block::DamageLevel() const { return 1; }
int  Block::LightRadius() const { return 0; }
void Block::ReceiveSignal(QString) {}
usage_types Block::Use(Active *) { return USAGE_TYPE_NO; }
usage_types Block::UseOnShredMove(Active *) { return USAGE_TYPE_NO; }

wearable Block::Wearable() const {
    switch ( Sub() ) {
    case GREENERY: return WEARABLE_OTHER;
    default: return WEARABLE_NOWHERE;
    }
}

bool Block::Inscribe(const QString str) {
    if ( Sub() == AIR ) return false;
    noteId = ( noteId == 0 ) ? // new note
        World::GetWorld()->SetNote(str.left(MAX_NOTE_LENGTH)) :
        World::GetWorld()->ChangeNote(str.left(MAX_NOTE_LENGTH), noteId);
    return true;
}

Inventory * Block::HasInventory() { return nullptr; }
Animal    * Block::IsAnimal()     { return nullptr; }
Active    * Block::ActiveBlock()  { return nullptr; }
Falling   * Block::ShouldFall()   { return nullptr; }

void Block::Restore() { durability = MAX_DURABILITY; }
void Block::Break()   { durability = 0; }

void Block::Mend(const int plus) {
    durability = qMin(MAX_DURABILITY, durability+plus);
}

QString Block::GetNote() const {
    return noteId ? World::GetWorld()->GetNote(noteId) : QString();
}

int Block::Weight() const {
    switch ( Sub() ) {
    default:        return WEIGHT_WATER;
    case SUB_CLOUD:
    case AIR:       return WEIGHT_AIR;
    case STONE:     return WEIGHT_STONE;
    case SOIL:      return WEIGHT_SAND + WEIGHT_WATER;
    case GREENERY:  return WEIGHT_GREENERY;
    case NULLSTONE: return WEIGHT_NULLSTONE;
    case SAND:      return WEIGHT_SAND;
    case GLASS:     return WEIGHT_GLASS;
    case WOOD:      return WEIGHT_WATER - 1;
    case IRON:      return WEIGHT_IRON;
    case PAPER:
    case ROSE:
    case SUB_NUT:   return WEIGHT_MINIMAL;
    case MOSS_STONE:
    case A_MEAT:
    case H_MEAT:    return WEIGHT_WATER + 10;
    case SKY:
    case STAR:
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

sub_groups Block::GetSubGroup(const int sub) {
    switch ( sub ) {
    default:     return GROUP_NONE;
    case AIR:
    case SKY:
    case STAR:   return GROUP_AIR;

    case H_MEAT:
    case A_MEAT: return GROUP_MEAT;

    case STONE:
    case DIAMOND:
    case WOOD:
    case BONE:   return GROUP_HANDY;

    case IRON:
    case GOLD:
    case ADAMANTINE:
    case STEEL:  return GROUP_METAL;
    }
}

bool Block::operator==(const Block & block) const {
    return ( block.Kind() == Kind()
        && block.Sub()    == Sub()
        && block.GetDurability() == GetDurability()
        && block.GetDir()        == GetDir()
        && block.GetNote()       == GetNote() );
}

void Block::SaveAttributes(QDataStream &) const {}

void Block::SaveToFile(QDataStream & out) {
    if ( this == BlockFactory::Normal(sub) ) {
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
        Block(kind_, sub_, transp)
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
