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
#include "TrManager.h"
#include <QDataStream>
#include <memory>

#define TEST_DAMAGE 0
#if TEST_DAMAGE == 1
#include <QFile>
#include <QTextStream>
#endif

const int Block::MAX_DURABILITY = 1024;

dirs Block::MakeDirFromDamage(const int dmg_kind) {
    Q_ASSERT(dmg_kind >= DAMAGE_PUSH_UP);
    return static_cast<dirs>(dmg_kind - DAMAGE_PUSH_UP);
}

QString Block::FullName() const {
    if ( Kind() ==  BLOCK ) {
        TrString waterNameString = QObject::tr("Ice");
        TrString sandNameString  = QObject::tr("Sandstone");
        TrString clayNameString  = QObject::tr("Clay brick");
        TrString acidNameString  = QObject::tr("Acid concentrate");

        switch ( Sub() ) {
        default:    return TrManager::SubNameUpper(Sub());
        case WATER: return waterNameString;
        case SAND:  return sandNameString;
        case CLAY:  return clayNameString;
        case ACID:  return acidNameString;
        case IRON:
        case GOLD:
        case COAL: break;
        }
    }
    return Str("%1 (%2)").
        arg(TrManager::KindName(Kind())).
        arg(TrManager::SubName(Sub()));
}

Q_DECL_RELAXED_CONSTEXPR int Block::Transparency(const int sub) {
    switch ( sub ) {
    default:    return BLOCK_OPAQUE;
    case WATER:
    case GREENERY:
    case ACID:
    case SUB_CLOUD:
    case DIAMOND:
    case GLASS: return BLOCK_TRANSPARENT;
    case AIR:   return INVISIBLE;
    }
}

void Block::Damage(const int dmg, const int dmg_kind) {
    static const struct Property {
        const int immunity, vulnerability, destruction;
    } properties[SUB_COUNT] = {
        {DAMAGE_CUT | DAMAGE_HEAT | DAMAGE_PUSH_ANYWHERE,   // STONE
            DAMAGE_MINE, DAMAGE_NO},
        properties[STONE],                                  // MOSS_STONE
        {DAMAGE_ANY, DAMAGE_NO, DAMAGE_NO},                 // NULLSTONE
        properties[NULLSTONE],                              // SKY
        properties[NULLSTONE],                              // STAR
        {DAMAGE_ACID, DAMAGE_NO, DAMAGE_NO},                // DIAMOND
        {DAMAGE_NO, DAMAGE_DIG, DAMAGE_NO},                 // SOIL
        {DAMAGE_PUSH_ANYWHERE, DAMAGE_THRUST, DAMAGE_NO},   // H_MEAT
        properties[H_MEAT],                                 // A_MEAT
        {DAMAGE_ACID | DAMAGE_HEAT, DAMAGE_NO,
            DAMAGE_ANY & ~(DAMAGE_ACID | DAMAGE_HEAT)},     // GLASS
        {DAMAGE_NO, DAMAGE_CUT, DAMAGE_NO},                 // WOOD
        {DAMAGE_ANY & ~DAMAGE_TIME & ~DAMAGE_ACID,
            DAMAGE_NO, DAMAGE_NO},                          // DIFFERENT
        {DAMAGE_PUSH_ANYWHERE, DAMAGE_NO, DAMAGE_NO},       // IRON
        {DAMAGE_ANY & ~DAMAGE_TIME & ~DAMAGE_HEAT,
            DAMAGE_NO, DAMAGE_NO},                          // WATER
        {DAMAGE_NO, DAMAGE_NO, DAMAGE_ANY},                 // GREENERY
        properties[SOIL],                                   // SAND
        properties[GREENERY],                               // SUB_NUT
        properties[GREENERY],                               // ROSE
        properties[SOIL],                                   // CLAY
        properties[NULLSTONE],                              // AIR
        {DAMAGE_NO, DAMAGE_HEAT, DAMAGE_NO},                // PAPER
        properties[IRON],                                   // GOLD
        {DAMAGE_NO, DAMAGE_NO, DAMAGE_NO},                  // BONE
        properties[IRON],                                   // STEEL
        {DAMAGE_ANY & ~DAMAGE_TIME & ~DAMAGE_FREEZE,
            DAMAGE_NO, DAMAGE_NO},                          // ADAMANTINE
        properties[ADAMANTINE],                             // FIRE
        properties[PAPER],                                  // COAL
        properties[PAPER],                                  // EXPLOSIVE
        {DAMAGE_ACID, DAMAGE_NO, DAMAGE_NO},                // ACID
        properties[GREENERY],                               // SUB_CLOUD
        properties[GREENERY],                               // SUB_DUST
        {DAMAGE_NO, DAMAGE_NO, DAMAGE_NO}                   // SUB_PLASTIC
    };
    const Property& property = properties[Sub()];
    durability -= bool(dmg_kind &  property.destruction) * durability +
             (1 + bool(dmg_kind &  property.vulnerability) * 4) *
                  bool(dmg_kind & ~property.immunity) * dmg;
} // Block::Damage(const ind dmg, const int dmg_kind)

void Block::TestDamage() {
#if TEST_DAMAGE == 1
    QFile file(Str("damageTest.csv"));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&file);
    stream << ";";
    for (int damage_kind=1; damage_kind<=DAMAGE_PUSH_UP; damage_kind<<=1) {
        stream <<
            TrManager::GetDamageString(static_cast<damage_kinds>(damage_kind))
            << ";";
    }
    stream << endl;

    for (int sub=0; sub<SUB_COUNT; ++sub) {
        stream << TrManager::SubName(sub) << ";";
        for (int damage_kind=1; damage_kind<=DAMAGE_PUSH_UP; damage_kind<<=1) {
            std::unique_ptr<Block> block(
                BlockFactory::NewBlock(BLOCK, static_cast<subs>(sub)));
            block->Damage(1, damage_kind);
            stream << MAX_DURABILITY - block->GetDurability() << ";";
        }
        stream << endl;
    }
#endif
}

Block* Block::DropAfterDamage(bool* const delete_block) {
    switch ( Sub() ) {
    case GREENERY:
    case SUB_DUST:
    case GLASS:
    case AIR: return BlockFactory::Normal(AIR);
    case STONE: if ( BLOCK==Kind() ) {
        return BlockFactory::NewBlock(LADDER, STONE);
    } // no break;
    default: return DropInto(delete_block);
    }
}

Block* Block::DropInto(bool* const delete_block) {
    Block* const pile = BlockFactory::NewBlock(BOX, DIFFERENT);
    Restore();
    pile->HasInventory()->Get(this);
    *delete_block = false;
    return pile;
}

push_reaction Block::PushResult(dirs) const {
    return ( AIR==Sub() ) ? ENVIRONMENT : NOT_MOVABLE;
}

subs  Block::Sub()    const { return static_cast<subs >(substance); }
kinds Block::Kind()   const { return static_cast<kinds>(blockKind); }
dirs  Block::GetDir() const { return static_cast<dirs>(direction); }

bool Block::Catchable() const { return false; }
void Block::Move(dirs) {}
int  Block::DamageKind() const { return DAMAGE_CRUSH; }
int  Block::DamageLevel() const { return 1; }
int  Block::LightRadius() const { return 0; }
void Block::ReceiveSignal(const QString&) {}
usage_types Block::Use(Active*) { return USAGE_TYPE_NO; }
usage_types Block::UseOnShredMove(Active*) { return USAGE_TYPE_NO; }

QString Block::Description() const {
    if ( Kind() == BLOCK ) {
        TrString stoneDescription = QObject::tr("Solid wall.");
        TrString airDescription   = QObject::tr("Is needed for breathing.");

        switch ( Sub() ) {
        case STONE: return stoneDescription;
        case AIR:   return airDescription;
        default: break;
        }
    }

    TrString typicalDescription = QObject::tr("Typical object of %1.");
    return typicalDescription.arg(TrManager::SubName(Sub()));
}

wearable Block::Wearable() const {
    switch ( Sub() ) {
    case GREENERY: return WEARABLE_OTHER;
    default: return WEARABLE_NOWHERE;
    }
}

bool Block::Inscribe(const QString& str) {
    if ( Sub() == AIR ) return false;
    noteId = ( noteId == 0 ) ? // new note
        World::GetWorld()->SetNote(str.left(MAX_NOTE_LENGTH)) :
        World::GetWorld()->ChangeNote(str.left(MAX_NOTE_LENGTH), noteId);
    return true;
}

Animal*    Block::IsAnimal()     { return nullptr; }
Active*    Block::ActiveBlock()  { return nullptr; }
Falling*   Block::ShouldFall()   { return nullptr; }

const Active* Block::ActiveBlockConst() const { return nullptr; }

Inventory* Block::HasInventory() {return BlockFactory::Block2Inventory(this);}

const Inventory* Block::HasConstInventory() const {
    return const_cast<Block*>(this)->HasInventory();
}

void Block::Restore() { durability = MAX_DURABILITY; }
void Block::Break()   { durability = 0; }

void Block::Mend(const int plus) {
    durability = std::min(MAX_DURABILITY, durability+plus);
}

QString Block::GetNote() const {
    return noteId ? World::GetCWorld()->GetNote(noteId) : QString();
}

int Block::Weight() const {
    switch ( Sub() ) {
    default:        return WEIGHT_WATER;
    case SUB_CLOUD:
    case AIR:       return WEIGHT_AIR;
    case STONE:     return WEIGHT_STONE;
    case SOIL:      return WEIGHT_SAND + WEIGHT_WATER;
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

void Block::SetDir(const int dir) { direction = dir; }

bool Block::operator==(const Block& block) const {
    return ( block.Kind() == Kind()
        && block.Sub()    == Sub()
        && block.GetDurability() == GetDurability()
        && block.GetDir()        == GetDir()
        && block.GetNote()       == GetNote() );
}

void Block::SaveAttributes(QDataStream&) const {}

void Block::SaveToFile(QDataStream& out) {
    if ( this == BlockFactory::Normal(substance) ) {
        SaveNormalToFile(out);
    } else {
        out << substance << blockKind <<
            qint16( ( ( ( durability
            <<= 3 ) |= direction )
            <<= 1 ) |= (noteId != 0) );
        if ( Q_UNLIKELY(noteId != 0) ) {
            out << noteId;
        }
        SaveAttributes(out);
    }
}

void Block::SaveNormalToFile(QDataStream& out) const {
    out << quint8( 0x80 | substance );
}

Block::Block(const kinds kind, const subs sub)
    : noteId(0)
    , durability(MAX_DURABILITY)
    , transparent(Transparency(sub))
    , blockKind(kind)
    , substance(sub)
    , direction(UP)
{}

Block::Block(QDataStream& stream, const kinds kind, const subs sub)
    : noteId(0)
    , durability()
    , transparent(Transparency(sub))
    , blockKind(kind)
    , substance(sub)
    , direction()
{
    // use durability as buffer, set actual value in the end:
    stream >> durability;
    if ( Q_UNLIKELY(durability & 1) ) {
        stream >> noteId;
    }
    direction = ( durability >>= 1 ) & 0b0111;
    durability >>= 3;
}

Block::~Block() {}

sub_groups Block::GetSubGroup(const int sub) {
    switch ( sub ) {
    default:     return GROUP_NONE;
    case AIR:
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
