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
#include "blocks/Inventory.h"
#include "blocks/blocks.h"
#include "blocks/Containers.h"

#include "BlockFactory.h"
#include "World.h"
#include "TrManager.h"

#include <QDataStream>

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

bool Block::IsNormal() const { return BlockFactory::IsNormal(this); }

void Block::Damage(const int dmg, const int dmg_kind) {
    Q_ASSERT(not IsNormal());

    struct _Property {
        int immunity, vulnerability, destruction;
    };
    typedef const _Property Property;
    static Property stone = { DAMAGE_CUT | DAMAGE_HEAT | DAMAGE_PUSH_ANYWHERE
                            , DAMAGE_MINE
                            , DAMAGE_NO };
    static Property nullstone = { DAMAGE_ANY, DAMAGE_NO, DAMAGE_NO };
    static Property meat  = { DAMAGE_PUSH_ANYWHERE, DAMAGE_THRUST, DAMAGE_NO };
    static Property soil  = { DAMAGE_NO, DAMAGE_DIG, DAMAGE_NO };
    static Property green = { DAMAGE_NO, DAMAGE_NO, DAMAGE_ANY };
    static Property iron  = { DAMAGE_PUSH_ANYWHERE, DAMAGE_NO, DAMAGE_NO };
    static Property adamantine = { DAMAGE_ANY & ~DAMAGE_TIME & ~DAMAGE_FREEZE
                                 , DAMAGE_NO
                                 , DAMAGE_NO };
    static Property paper = { DAMAGE_NO, DAMAGE_HEAT, DAMAGE_NO };

    static const Property properties[SUB_COUNT] = {
        stone,                                              // STONE
        stone,                                              // MOSS_STONE
        nullstone,                                          // NULLSTONE
        nullstone,                                          // SKY
        nullstone,                                          // STAR
        {DAMAGE_ACID, DAMAGE_NO, DAMAGE_NO},                // DIAMOND
        soil,                                               // SOIL
        meat,                                               // H_MEAT
        meat,                                               // A_MEAT
        {DAMAGE_ACID | DAMAGE_HEAT, DAMAGE_NO,
            DAMAGE_ANY & ~(DAMAGE_ACID | DAMAGE_HEAT)},     // GLASS
        {DAMAGE_NO, DAMAGE_CUT, DAMAGE_NO},                 // WOOD
        {DAMAGE_ANY & ~DAMAGE_TIME & ~DAMAGE_ACID,
            DAMAGE_NO, DAMAGE_NO},                          // DIFFERENT
        iron,                                               // IRON
        {DAMAGE_ANY & ~DAMAGE_TIME & ~DAMAGE_HEAT,
            DAMAGE_NO, DAMAGE_NO},                          // WATER
        green,                                              // GREENERY
        soil,                                               // SAND
        green,                                              // SUB_NUT
        green,                                              // ROSE
        soil,                                               // CLAY
        nullstone,                                          // AIR
        paper,                                              // PAPER
        iron,                                               // GOLD
        {DAMAGE_NO, DAMAGE_NO, DAMAGE_NO},                  // BONE
        iron,                                               // STEEL
        adamantine,                                         // ADAMANTINE
        adamantine,                                         // FIRE
        paper,                                              // COAL
        paper,                                              // EXPLOSIVE
        {DAMAGE_ACID, DAMAGE_NO, DAMAGE_NO},                // ACID
        green,                                              // SUB_CLOUD
        green,                                              // SUB_DUST
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
                new Block(static_cast<subs>(sub)));
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
        return new Ladder(STONE);
    } // no break;
    default: return DropInto(delete_block);
    }
}

Block* Block::DropInto(bool* const delete_block) {
    Block* const pile = new Box(DIFFERENT);
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
    Q_ASSERT(not IsNormal());

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

void Block::Break() {
    Q_ASSERT(not IsNormal());
    durability = 0;
}

void Block::Mend(const int plus) {
    Q_ASSERT(not IsNormal());
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

bool Block::SetDir(const int dir) {
    Q_ASSERT(not IsNormal());

    if (GetSubGroup(Sub()) != GROUP_AIR) {
        direction = dir;
        return true;
    } else {
        return false;
    }
}

bool Block::operator==(const Block& block) const {
    return ( block.Kind() == Kind()
        && block.Sub()    == Sub()
        && block.GetDurability() == GetDurability()
        && block.GetDir()        == GetDir()
             && block.GetNote()       == GetNote() );
}

void Block::operator delete(void* const ptr, std::size_t) {
    Block* const block = reinterpret_cast<Block*>(ptr);
    if ( not BlockFactory::IsNormal(block) ) {
        ::operator delete(ptr);
    }
}

void Block::SaveAttributes(QDataStream&) const {}

void Block::SaveToFile(QDataStream& out) const {
    qint16 buffer = durability;
    out << substance << blockKind <<
        qint16( ( ( ( buffer
        <<= 3 ) |= direction )
        <<= 1 ) |= (noteId != 0) );
    if ( Q_UNLIKELY(noteId != 0) ) {
        out << noteId;
    }
    SaveAttributes(out);
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
