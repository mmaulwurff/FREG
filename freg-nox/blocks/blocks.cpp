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

#include <QTextStream>
#include <QFile>
#include "blocks.h"
#include "world.h"
#include "Shred.h"
#include "CraftManager.h"
#include "BlockManager.h"
#include "Xyz.h"

// Block::
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
            case CRUSH: case DAMAGE_HANDS:
            case CUT:   return;
            case MINE:  mult = 2; break;
        } break;
        case GREENERY:
        case GLASS: durability = 0; return;
        case WOOD: switch ( dmg_kind ) {
            default:  mult = 1; break;
            case CUT: mult = 2; break;
            case DAMAGE_HANDS: return;
        } break;
        case SAND:
        case A_MEAT:
        case H_MEAT: ++(mult = (THRUST==dmg_kind)); break;
        case SOIL:   ++(mult = (DIG   ==dmg_kind)); break;
        case FIRE: mult = (FREEZE==dmg_kind || TIME==dmg_kind); break;
        }
        durability -= mult*dmg;
    }

    Block * Block::DropAfterDamage() const {
        return GLASS==Sub() ?
            0 : BLOCK==Kind() ?
                block_manager.NormalBlock(Sub()) :
                block_manager.NewBlock(Kind(), Sub());
    }

    int  Block::PushResult(const int) const {
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
    uchar Block::LightRadius() const { return (FIRE == Sub()) ? 5 : 0; }
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
    int  Block::GetDir() const { return direction; }
    int  Block::Sub() const { return sub; }
    int  Block::Transparent() const { return transparent; }
    short Block::GetDurability() const { return durability; }
    QString Block::GetNote() const { return note ? *note : ""; }

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
        case AIR:       return WEIGHT_AIR;
        default:        return WEIGHT_WATER;
        }
    }

    void Block::SetDir(const int dir) {
        if ( BLOCK!=Kind() || WOOD==Sub() ) {
            direction = dir;
        }
    }

    bool Block::operator==(const Block & block) const {
        return ( block.GetId()==GetId() &&
            block.GetDir()==GetDir() &&
            block.GetDurability()==GetDurability() &&
            ( (!note && !block.note) ||
                (note && block.note && *block.note==*note) ) );
    }
    bool Block::operator!=(const Block & block) const {
        return !(*this == block);
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
// Plate::
    QString Plate::FullName() const {
        switch ( Sub() ) {
        case WOOD:  return QObject::tr("Wooden board");
        case IRON:  return QObject::tr("Iron plate");
        case STONE: return QObject::tr("Stone slab");
        default:
            fprintf(stderr, "Plate::FullName: unlisted sub: %d",
                Sub());
            return "Strange plate";
        }
    }

    quint8 Plate::Kind() const { return PLATE; }
    int Plate::PushResult(const int) const { return JUMP; }
    ushort Plate::Weight() const { return Block::Weight()/4; }

    Plate::Plate(const int sub, const quint16 id) :
            Block(sub, id, NONSTANDARD)
    {}
    Plate::Plate(QDataStream & str, const int sub, const quint16 id) :
            Block(str, sub, id, NONSTANDARD)
    {}
// Ladder::
    QString Ladder::FullName() const {
        switch ( Sub() ) {
        case WOOD:  return QObject::tr("Ladder");
        case STONE: return QObject::tr("Rock with ledges");
        case GREENERY: return QObject::tr("Liana");
        default:
            fprintf(stderr, "Ladder::FullName: unlisted sub: %d\n",
                Sub());
            return "Strange ladder";
    }
    }

    int  Ladder::PushResult(const int) const { return MOVE_UP; }
    bool Ladder::Catchable() const { return true; }
    ushort Ladder::Weight() const { return Block::Weight()*3; }
    quint8 Ladder::Kind() const { return LADDER; }

    Block * Ladder::DropAfterDamage() const {
        return ( STONE==Sub() ) ?
            block_manager.NormalBlock(STONE) :
            block_manager.NewBlock(LADDER, Sub());
    }

    Ladder::Ladder(const int sub, const quint16 id) :
            Block(sub, id, NONSTANDARD)
    {}
    Ladder::Ladder(QDataStream & str, const int sub, const quint16 id) :
            Block(str, sub, id, NONSTANDARD)
    {}
// Animal::
    INNER_ACTIONS Animal::ActInner() {
        if ( satiation <= 0 ) {
            Damage(5, HUNGER);
        } else {
            --satiation;
        }
        if ( durability < MAX_DURABILITY ) {
            ++durability;
        }
        emit Updated();
        return INNER_ACTION_NONE;
    }

    void Animal::DoRareAction() {
        if ( not IsSubAround(AIR) ) {
            if ( breath <= 0 ) {
                Damage(10, BREATH);
            } else {
                --breath;
            }
        } else if ( breath < MAX_BREATH ) {
            ++breath;
        }
        emit Updated();
    }
    int Animal::ShouldAct() const { return FREQUENT_ANIMAL | RARE; }

    ushort Animal::Breath() const { return breath; }
    ushort Animal::Satiation() const { return satiation; }
    Animal * Animal::IsAnimal() { return this; }
    int Animal::DamageKind() const { return BITE; }

    bool Animal::Eat(const int sub) {
        const int value = NutritionalValue(sub);
        if ( value ) {
            satiation += value;
            ReceiveSignal(tr("Yum!"));
            if ( SECONDS_IN_DAY < satiation ) {
                satiation=1.1*SECONDS_IN_DAY;
                ReceiveSignal(tr("You have gorged yourself!"));
            }
            return true;
        } else {
            ReceiveSignal(tr("You cannot eat this."));
            return false;
        }
    }

    void Animal::SaveAttributes(QDataStream & out) const {
        Active::SaveAttributes(out);
        out << breath << satiation;
    }

    void Animal::EatGrass() {
        for (ushort x=X()-1; x<=X()+1; ++x)
        for (ushort y=Y()-1; y<=Y()+1; ++y) {
            if ( world->InBounds(x, y) &&
                    GREENERY == world->GetBlock(x, y, Z())->Sub() )
            {
                world->Damage(x, y, Z(), DamageLevel(), DamageKind());
                if ( world->GetBlock(x, y, Z())->GetDurability() <= 0 ) {
                    world->DestroyAndReplace(x, y, Z());
                }
                Eat(GREENERY);
                return;
            }
        }
    }

    Block * Animal::DropAfterDamage() const {
        return block_manager.NewBlock(WEAPON, BONE);
    }

    Animal::Animal(const int sub, const quint16 id) :
            Active(sub, id, NONSTANDARD),
            breath(MAX_BREATH),
            satiation(SECONDS_IN_DAY)
    {}
    Animal::Animal(QDataStream & str, const int sub, const quint16 id) :
            Active(str, sub, id, NONSTANDARD)
    {
        str >> breath >> satiation;
    }
// Inventory::
    bool   Inventory::Access() const { return true; }
    ushort Inventory::Start() const { return 0; }
    ushort Inventory::Size() const { return size; }
    Inventory * Inventory::HasInventory() { return this; }

    bool Inventory::Drop(const ushort src, ushort dest, ushort num,
            Inventory * const inv_to)
    {
        dest = qMax(inv_to->Start(), dest);
        bool ok_flag = false;
        for ( ; num; --num) {
            if ( src < Size()
                    && dest < inv_to->Size()
                    && !inventory[src].isEmpty()
                    && inv_to->Get(inventory[src].top(),
                        dest) )
            {
                ok_flag = true;
                Pull(src);
            }
        }
        return ok_flag;
    }

    bool Inventory::GetAll(Inventory * const from) {
        bool flag = false;
        for (ushort i=0; i<from->Size(); ++i) {
            if ( from->Drop(i, 0, from->Number(i), this) ) {
                flag = true;
            }
        }
        return flag;
    }

    void Inventory::Pull(const ushort num) {
        if ( not inventory[num].isEmpty() ) {
            inventory[num].pop();
        }
    }

    void Inventory::SaveAttributes(QDataStream & out) const {
        for (ushort i=0; i<Size(); ++i) {
            out << Number(i);
            for (ushort j=0; j<Number(i); ++j) {
                inventory[i].top()->SaveToFile(out);
            }
        }
    }

    bool Inventory::Get(Block * const block, const ushort start) {
        if ( block == nullptr ) return true;
        if ( block->Kind() == LIQUID ) {
            for (int i=qMax(Start(), start); i<Size(); ++i) {
                if ( Number(i)==1 && ShowBlock(i) ) {
                    Inventory * const inner = ShowBlock(i)->HasInventory();
                    if ( inner && inner->Get(block) ) {
                        return true;
                    }
                }
            }
            return false;
        } // else:
        for (ushort i=qMax(Start(), start); i<Size(); ++i) {
            if ( GetExact(block, i) ) {
                return true;
            }
        }
        return false;
    }

    bool Inventory::GetExact(Block * const block, const ushort num) {
        if ( block ) {
            if ( inventory[num].isEmpty() ) {
                inventory[num].push(block);
            } else if ( *block == *inventory[num].top()
                    && Number(num) < MAX_STACK_SIZE )
            {
                Inventory * const inner = inventory[num].top()->HasInventory();
                if ( inner==nullptr || inner->IsEmpty() ) {
                    inventory[num].push(block);
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
        return true;
    }

    void Inventory::MoveInside(const ushort num_from, const ushort num_to,
            const ushort num)
    {
        for (ushort i=0; i<num; ++i) {
            if ( GetExact(ShowBlock(num_from), num_to) ) {
                Pull(num_from);
            }
        }
    }

    bool Inventory::InscribeInv(const ushort num, const QString str) {
        const int number = Number(num);
        if ( number == 0 ) {
            ReceiveSignal(QObject::tr("Nothing here."));
            return false;
        }
        const int sub = inventory[num].top()->Sub();
        if ( inventory[num].top() == block_manager.NormalBlock(sub) ) {
            for (ushort i=0; i<number; ++i) {
                inventory[num].replace(i, block_manager.NormalBlock(sub));
            }
        }
        for (ushort i=0; i<number; ++i) {
            if ( !inventory[num].at(i)->Inscribe(str) ) {
                ReceiveSignal(QObject::tr("Cannot inscribe this."));
                return false;
            }
        }
        ReceiveSignal(QObject::tr("Inscribed."));
        return true;
    }

    QString Inventory::InvFullName(const ushort num) const {
        return inventory[num].isEmpty() ?
            "" : inventory[num].top()->FullName();
    }

    QString Inventory::NumStr(const ushort num) const {
        return QString(" (%1x)").arg(Number(num));
    }

    ushort Inventory::GetInvWeight(const ushort i) const {
        return inventory[i].isEmpty() ?
            0 : inventory[i].top()->Weight()*Number(i);
    }

    int Inventory::GetInvSub(const ushort i) const {
        return inventory[i].isEmpty() ?
            AIR : inventory[i].top()->Sub();
    }

    int Inventory::GetInvKind(const ushort i) const {
        return inventory[i].isEmpty() ?
            BLOCK : int(inventory[i].top()->Kind());
    }

    QString Inventory::GetInvNote(const ushort num) const {
        return inventory[num].top()->GetNote();
    }

    ushort Inventory::Weight() const {
        ushort sum = 0;
        for (ushort i=0; i<Size(); ++i) {
            sum += GetInvWeight(i);
        }
        return sum;
    }

    Block * Inventory::ShowBlock(const ushort slot) const {
        return ( slot > Size() || Number(slot)==0 ) ?
            nullptr : inventory[slot].top();
    }

    Block * Inventory::ShowBlock(const ushort slot, const ushort num) const {
        return ( slot > Size() || num+1 > Number(slot) ) ?
            nullptr : inventory[slot].at(num);
    }

    bool Inventory::IsEmpty() const {
        for (ushort i=Start(); i<Size(); ++i) {
            if ( not inventory[i].isEmpty() ) {
                return false;
            }
        }
        return true;
    }

    void Inventory::Push(Block * const who) {
        Inventory * const inv = who->HasInventory();
        if ( inv ) {
            inv->GetAll(this);
        }
    }

    quint8 Inventory::Number(const ushort i) const {
        return inventory[i].size();
    }

    bool Inventory::MiniCraft(const ushort num) {
        if ( Number(num) == 0 ) {
            ReceiveSignal(QObject::tr("Nothing here."));
            return false;
        } // else:
        const CraftItem * const crafted = craft_manager.MiniCraft(
            Number(num), BlockManager::MakeId(GetInvKind(num),GetInvSub(num)));
        if ( crafted ) {
            while ( not inventory[num].isEmpty() ) {
                Block * const to_delete = ShowBlock(num);
                Pull(num);
                block_manager.DeleteBlock(to_delete);
            }
            for (int i=0; i<crafted->num; ++i) {
                GetExact(block_manager.NewBlock(
                    BlockManager::KindFromId(crafted->id),
                    BlockManager:: SubFromId(crafted->id) ), num);
            }
            ReceiveSignal(QObject::tr("Craft successful."));
            delete crafted;
            return true;
        } else {
            ReceiveSignal(QObject::tr("You don't know how to craft this."));
            return false;
        }
    }

    Inventory::Inventory(const ushort sz) :
            size(sz),
            inventory(new QStack<Block *>[sz])
    {}
    Inventory::Inventory(QDataStream & str, const ushort sz) :
            Inventory(sz)
    {
        for (ushort i=0; i<Size(); ++i) {
            quint8 num;
            str >> num;
            while ( num-- ) {
                inventory[i].push(block_manager.
                    BlockFromFile(str));
            }
        }
    }
    Inventory::Inventory(const Inventory & inv) :
            Inventory(inv.Size())
    {}
    Inventory::~Inventory() {
        for (ushort i=0; i<Size(); ++i) {
            while ( !inventory[i].isEmpty() ) {
                block_manager.DeleteBlock(inventory[i].pop());
            }
        }
        delete [] inventory;
    }
// Chest::
    quint8 Chest::Kind() const { return CHEST; }
    int Chest::Sub() const { return Block::Sub(); }
    Inventory * Chest::HasInventory() { return Inventory::HasInventory(); }
    usage_types Chest::Use(Block *) { return USAGE_TYPE_OPEN; }
    void Chest::ReceiveSignal(const QString str) { Block::ReceiveSignal(str); }

    QString Chest::FullName() const {
        switch ( Sub() ) {
        case WOOD:  return QObject::tr("Wooden chest");
        case STONE: return QObject::tr("Stone chest");
        default:
            fprintf(stderr, "Chest::FullName: unlisted sub: %d\n", Sub());
            return QObject::tr("Chest");
        }
    }

    void Chest::Push(const int, Block * const who) { Inventory::Push(who); }

    ushort Chest::Weight() const {
        return Block::Weight()*4 + Inventory::Weight();
    }

    void Chest::SaveAttributes(QDataStream & out) const {
        Inventory::SaveAttributes(out);
    }

    Chest::Chest(const int sub, const quint16 id, const ushort size) :
            Block(sub, id),
            Inventory(size)
    {}
    Chest::Chest(QDataStream & str, const int sub, const quint16 id,
            const ushort size)
        :
            Block(str, sub, id),
            Inventory(str, size)
    {}
// Pile::
    void Pile::Push(const int, Block * const who) {
        Inventory::Push(who);
        if ( IsEmpty() ) {
            GetWorld()->DestroyAndReplace(X(), Y(), Z());
        }
    }

    void Pile::DoRareAction() {
        Inventory * const inv =
            GetWorld()->GetBlock(X(), Y(), Z()-1)->HasInventory();
        if ( inv ) {
            inv->GetAll(this);
        }
        if ( IsEmpty() ) {
            Damage(GetDurability(), TIME);
        }
    }

    int Pile::ShouldAct() const { return RARE; }
    quint8 Pile::Kind() const { return PILE; }
    int Pile::Sub() const { return Block::Sub(); }
    Inventory * Pile::HasInventory() { return Inventory::HasInventory(); }
    usage_types Pile::Use(Block *) { return USAGE_TYPE_OPEN; }
    ushort Pile::Weight() const { return Inventory::Weight(); }
    Block * Pile::DropAfterDamage() const { return 0; }

    void Pile::ReceiveSignal(const QString str) { Active::ReceiveSignal(str); }

    QString Pile::FullName() const {
        switch ( Sub() ) {
        case DIFFERENT: return tr("Pile");
        default:
            fprintf(stderr, "Pile::FullName: unlisted sub: %d\n", Sub());
            return tr("Unknown pile");
        }
    }

    void Pile::SaveAttributes(QDataStream & out) const {
        Active::SaveAttributes(out);
        Inventory::SaveAttributes(out);
    }

    Pile::Pile(const int sub, const quint16 id) :
            Active(sub, id, NONSTANDARD),
            Inventory(INV_SIZE)
    {}
    Pile::Pile(QDataStream & str, const int sub, const quint16 id) :
            Active(str, sub, id, NONSTANDARD),
            Inventory(str, INV_SIZE)
    {}
// Liquid::
    void Liquid::DoRareAction() {
        World * const world = GetWorld();
        // IDEA: turn off water drying up in ocean
        if ( WATER == Sub() && not IsSubAround(WATER) ) {
            Damage(1, HEAT);
        }
        switch ( qrand()%20 ) {
        case 0: world->Move(X(), Y(), Z(), NORTH); break;
        case 1: world->Move(X(), Y(), Z(), EAST);  break;
        case 2: world->Move(X(), Y(), Z(), SOUTH); break;
        case 3: world->Move(X(), Y(), Z(), WEST);  break;
        }
    }

    int Liquid::ShouldAct() const  { return RARE; }
    int Liquid::PushResult(const int) const { return ENVIRONMENT; }
    quint8 Liquid::Kind() const { return LIQUID; }
    int Liquid::Temperature() const { return ( WATER==Sub() ) ? 0 : 1000; }
    uchar Liquid::LightRadius() const { return ( WATER==Sub() ) ? 0 : 3; }
    Block * Liquid::DropAfterDamage() const { return 0; }

    QString Liquid::FullName() const {
        switch ( Sub() ) {
        case WATER: return tr("Liquid");
        case STONE: return tr("Lava");
        default:
            fprintf(stderr, "Liquid::FullName(): sub (?): %d\n", Sub());
            return "Unknown liquid";
        }
    }

    Liquid::Liquid(const int sub, const quint16 id) :
            Active(sub, id)
    {}
    Liquid::Liquid(QDataStream & str, const int sub, const quint16 id) :
            Active(str, sub, id)
    {}
// Grass::
    void Grass::DoRareAction() {
        World * const world = GetWorld();
        if ( FIRE == Sub() ) {
            const Xyz coords[] = {
                Xyz( X()-1, Y(),   Z()   ),
                Xyz( X()+1, Y(),   Z()   ),
                Xyz( X(),   Y()-1, Z()   ),
                Xyz( X(),   Y()+1, Z()   ),
                Xyz( X(),   Y(),   Z()-1 ),
                Xyz( X(),   Y(),   Z()+1 ) };
            for (const Xyz xyz : coords) {
                if ( not world->InBounds(xyz.GetX(), xyz.GetY()) ) {
                    continue;
                }
                world->Damage(xyz.GetX(), xyz.GetY(), xyz.GetZ(), 5, HEAT);
                if ( world->GetBlock(xyz.GetX(), xyz.GetY(), xyz.GetZ())->
                        GetDurability() <= 0 )
                {
                    world->DestroyAndReplace(
                        xyz.GetX(), xyz.GetY(), xyz.GetZ());
                }
            }
            if ( qrand()%10 || IsSubAround(WATER) ) {
                Damage(2, FREEZE);
            }
        }
        if ( not IsBase(Sub(), world->GetBlock(X(), Y(), Z()-1)->Sub()) ) {
            world->DestroyAndReplace(X(), Y(), Z());
        }
        short i=X(), j=Y();
        // increase this if grass grows too fast
        switch ( qrand() % (FIRE==Sub() ? 4 : SECONDS_IN_HOUR*2) ) {
        case 0: ++i; break;
        case 1: --i; break;
        case 2: ++j; break;
        case 3: --j; break;
        default: return;
        }
        if ( not world->InBounds(i, j) ) return;
        const quint8 sub_near = world->GetBlock(i, j, Z())->Sub();
        if ( world->Enlightened(i, j, Z()) || FIRE == Sub() ) {
            if ( AIR == sub_near
                    && IsBase(Sub(), world->GetBlock(i, j, Z()-1)->Sub() ) )
            {
                world->Build(block_manager.NewBlock(GRASS, Sub()), i, j, Z());
            } else if ( IsBase(Sub(), sub_near)
                    && AIR == world->GetBlock(i, j, Z()+1)->Sub() )
            {
                world->Build(block_manager.NewBlock(GRASS, Sub()), i,j, Z()+1);
            }
        }
    }

    bool Grass::IsBase(const quint8 own_sub, const quint8 ground) {
        return ( GREENERY==own_sub && SOIL==ground )
            || ( FIRE==own_sub && (
                WOOD==ground
                || GREENERY==ground
                || H_MEAT==ground
                || A_MEAT==ground
                || HAZELNUT==ground
                || ROSE==ground
                || PAPER==ground ) );
    }

    QString Grass::FullName() const {
        switch ( Sub() ) {
        case GREENERY: return tr("Grass");
        case FIRE:     return tr("Fire");
        default:
            fprintf(stderr, "Grass::FullName(): sub (?): %d\n", Sub());
            return "Unknown plant";
        }
    }

    int  Grass::ShouldAct() const  { return RARE; }
    void Grass::Push(const int, Block * const) {
        GetWorld()->DestroyAndReplace(X(), Y(), Z());
    }
    bool Grass::ShouldFall() const { return false; }
    quint8 Grass::Kind() const { return GRASS; }
    Block * Grass::DropAfterDamage() const { return 0; }

    Grass::Grass(const int sub, const quint16 id) :
            Active(sub, id)
    {}
    Grass::Grass(QDataStream & str, const int sub, const quint16 id) :
            Active(str, sub, id)
    {}
// Bush::
    int  Bush::Sub() const { return Block::Sub(); }
    bool Bush::ShouldFall() const { return false; }
    int  Bush::ShouldAct() const  { return RARE; }
    void Bush::ReceiveSignal(const QString str) { Active::ReceiveSignal(str); }
    QString Bush::FullName() const { return tr("Bush"); }
    quint8  Bush::Kind() const { return BUSH; }
    ushort  Bush::Weight() const { return Inventory::Weight()+Block::Weight(); }
    usage_types Bush::Use(Block *) { return USAGE_TYPE_OPEN; }
    Inventory * Bush::HasInventory() { return Inventory::HasInventory(); }

    void Bush::Damage(const ushort dmg, const int dmg_kind) {
        durability -= ( CUT==dmg_kind ? dmg*2 : dmg );
    }

    void Bush::DoRareAction() {
        if ( 0 == qrand()%(SECONDS_IN_HOUR*4) ) {
            Get(block_manager.NormalBlock(HAZELNUT));
        }
    }

    int Bush::PushResult(int const) const { return NOT_MOVABLE; }
    void Bush::Push(const int, Block * const who) { Inventory::Push(who); }

    Block * Bush::DropAfterDamage() const {
        Block * const pile = block_manager.NewBlock(PILE, DIFFERENT);
        pile->HasInventory()->Get(block_manager.NewBlock(WEAPON, WOOD));
        pile->HasInventory()->Get(block_manager.NormalBlock(HAZELNUT));
        return pile;
    }

    void Bush::SaveAttributes(QDataStream & out) const {
        Active::SaveAttributes(out);
        Inventory::SaveAttributes(out);
    }

    Bush::Bush(const int sub, const quint16 id) :
            Active(sub, id),
            Inventory(BUSH_SIZE)
    {}
    Bush::Bush(QDataStream & str, const int sub, const quint16 id) :
            Active(str, sub, id),
            Inventory(str, BUSH_SIZE)
    {}
// Rabbit::
    short Rabbit::Attractive(const int sub) const {
        switch ( sub ) {
        case H_MEAT:   return -16;
        case A_MEAT:   return -1;
        case GREENERY: return  1;
        case SAND:     return -1;
        default:       return  0;
        }
    }

    void Rabbit::DoRareAction() {
        Animal::DoRareAction();
        // eat sometimes
        World * const world = GetWorld();
        if ( SECONDS_IN_DAY/2 > Satiation() ) {
            EatGrass();
        }
        // random movement
        switch ( qrand()%60 ) {
        case 0: SetDir(NORTH); break;
        case 1: SetDir(SOUTH); break;
        case 2: SetDir(EAST);  break;
        case 3: SetDir(WEST);  break;
        default: return;
        }
        world->Move(X(), Y(), Z(), GetDir());
    }

    Block * Rabbit::DropAfterDamage() const {
        Block * const pile = block_manager.NewBlock(PILE, DIFFERENT);
        Inventory * const pile_inv = pile->HasInventory();
        pile_inv->Get(block_manager.NormalBlock(A_MEAT));
        pile_inv->Get(Animal::DropAfterDamage());
        return pile;
    }

    QString Rabbit::FullName() const { return tr("Herbivore"); }
    quint8 Rabbit::Kind() const { return RABBIT; }
    void Rabbit::DoFrequentAction() {
        if ( Gravitate(4, 1, 3, 4) ) {
            if ( qrand()%2 ) {
                world->Jump(X(), Y(), Z(), GetDir());
            } else {
                world->Move(X(), Y(), Z(), GetDir());
            }
        }
    }

    quint16 Rabbit::NutritionalValue(const quint8 sub) const {
        return ( GREENERY == sub ) ? SECONDS_IN_HOUR*4 : 0;
    }

    Rabbit::Rabbit(const int sub, const quint16 id) :
            Animal(sub, id)
    {}
    Rabbit::Rabbit(QDataStream & str, const int sub, const quint16 id) :
            Animal(str, sub, id)
    {}
// Workbench::
    void Workbench::Craft() {
        for (int i=0; i<Start(); ++i) { // remove previous products
            while ( Number(i) ) {
                Block * const to_pull = ShowBlock(i);
                Pull(i);
                block_manager.DeleteBlock(to_pull);
            }
        }
        int materials_number = 0;
        for (int i=Start(); i<Size(); ++i) {
            if ( Number(i) ) {
                ++materials_number;
            }
        }
        CraftList list(materials_number, 0);
        for (int i=Start(); i<Size(); ++i) {
            if ( Number(i) ) {
                list << new CraftItem({Number(i), ShowBlock(i)->GetId()});
            }
        }
        CraftList * products = craft_manager.Craft(&list, Sub());
        if ( products ) {
            for (int i=0; i<products->GetSize(); ++i) {
                for (int n=0; n<products->GetItem(i)->num; ++n) {
                    quint16 id = products->GetItem(i)->id;
                    GetExact(block_manager.NewBlock(
                        block_manager.KindFromId(id),
                        block_manager. SubFromId(id)), i);
                }
            }
            delete products;
        }
    }

    bool Workbench::Drop(const ushort src, const ushort dest,
            const ushort num, Inventory * const inv_to)
    {
        if ( inv_to == nullptr
                || src  >= Size()
                || dest >= inv_to->Size()
                || Number(src) == 0 )
        {
            return false;
        }
        for (ushort i=0; i<num; ++i) {
            if ( not inv_to->Get(ShowBlock(src), dest) ) return false;
            Pull(src);
            if ( src < Start() ) {
                // remove materials:
                for (ushort i=Start(); i<Size(); ++i) {
                    while ( Number(i) ) {
                        Block * const to_pull = ShowBlock(i);
                        Pull(i);
                        block_manager.DeleteBlock(to_pull);
                    }
                }
            } else {
                Craft();
            }
        }
        return true;
    }

    QString Workbench::FullName() const {
        switch ( Sub() ) {
        case WOOD: return QObject::tr("Workbench");
        case IRON: return QObject::tr("Iron anvil");
        default:
            fprintf(stderr, "Workbench::FullName: sub (?): %d\n", Sub());
            return "Strange workbench";
        }
    }

    quint8 Workbench::Kind() const { return WORKBENCH; }
    ushort Workbench::Start() const { return 2; }

    void Workbench::ReceiveSignal(const QString str) {
        Block::ReceiveSignal(str);
    }

    bool Workbench::Get(Block * const block, const ushort start) {
        if ( Inventory::Get(block, start) ) {
            Craft();
            return true;
        } else {
            return false;
        }
    }

    bool Workbench::GetAll(Inventory * const from) {
        if ( Inventory::GetAll(from) ) {
            Craft();
            return true;
        } else {
            return false;
        }
    }

    Workbench::Workbench(const int sub, const quint16 id) :
            Chest(sub, id, WORKBENCH_SIZE)
    {}
    Workbench::Workbench(QDataStream & str, const int sub, const quint16 id) :
            Chest(str, sub, id, WORKBENCH_SIZE)
    {}
// Door::
    int Door::PushResult(const int) const {
        return movable ? MOVABLE : NOT_MOVABLE;
    }

    void Door::Push(const int, Block * const who) {
        if ( not shifted
                && not locked
                && World::Anti(GetDir())!=who->GetDir() )
        {
            movable = true;
            shifted = GetWorld()->Move(X(), Y(), Z(), GetDir());
            movable = false;
        }
    }

    void Door::DoFrequentAction() {
        if ( shifted ) {
            movable = true;
            ushort x, y, z;
            GetWorld()->Focus(X(), Y(), Z(), x, y, z, World::Anti(GetDir()));
            if ( ENVIRONMENT == GetWorld()->GetBlock(x, y, z)->
                    PushResult(NOWHERE) )
            {
                shifted = !GetWorld()->Move(X(), Y(), Z(),
                    World::Anti(GetDir()));
            }
            movable = false;
        }
    }

    int  Door::ShouldAct() const { return FREQUENT_MECH; }
    bool Door::ShouldFall() const { return false; }
    quint8 Door::Kind() const { return locked ? LOCKED_DOOR : DOOR; }

    QString Door::FullName() const {
        QString sub_string;
        switch ( Sub() ) {
        case WOOD:  sub_string = tr(" of wood");  break;
        case STONE: sub_string = tr(" of stone"); break;
        case GLASS: sub_string = tr(" of glass"); break;
        case IRON:  sub_string = tr(" of iron");  break;
        default:
            sub_string = tr(" of something");
            fprintf(stderr, "Door::FullName: unlisted sub: %d\n", Sub());
        }
        return locked ? tr("Locked door") : tr("Door") + sub_string;
    }

    usage_types Door::Use(Block *) {
        locked = !locked;
        return USAGE_TYPE_NO;
    }

    void Door::SaveAttributes(QDataStream & out) const {
        Active::SaveAttributes(out);
        out << shifted << locked;
    }

    Door::Door(const int sub, const quint16 id) :
            Active(sub, id, ( STONE==sub ) ?
                BLOCK_OPAQUE : NONSTANDARD),
            shifted(false),
            locked(false),
            movable(false)
    {}
    Door::Door(QDataStream & str, const int sub, const quint16 id) :
            Active(str, sub, id, ( STONE==sub ) ?
                BLOCK_OPAQUE : NONSTANDARD),
            movable(false)
    {
        str >> shifted >> locked;
    }
// Clock::
    usage_types Clock::Use(Block * const who) {
        if ( who ) {
            const Active * const active = who->ActiveBlock();
            if ( active ) {
                who->ReceiveSignal(GetWorld()->TimeOfDayStr());
            }
        } else {
            SendSignalAround(world->TimeOfDayStr());
        }
        return USAGE_TYPE_NO;
    }

    QString Clock::FullName() const {
        switch ( Sub() ) {
        case IRON: return QObject::tr("Iron clock");
        default:
            fprintf(stderr, "Clock::FullName: unlisted sub: %d\n", Sub());
            return "Strange clock";
        }
    }

    quint8 Clock::Kind() const { return CLOCK; }
    ushort Clock::Weight() const { return Block::Weight()/10; }
    bool Clock::ShouldFall() const { return false; }
    void Clock::Push(const int, Block * const) { Use(); }
    int Clock::PushResult(const int) const { return NOT_MOVABLE; }
    int Clock::ShouldAct() const  { return RARE; }

    void Clock::DoRareAction() {
        if ( alarmTime == GetWorld()->TimeOfDay()
                || ActInner() == INNER_ACTION_MESSAGE )
        {
            Use();
        }
    }

    INNER_ACTIONS Clock::ActInner() {
        if ( timerTime > 0 )  {
            --timerTime;
            note->setNum(timerTime);
        } else if ( timerTime == 0 ) {
            *note = QObject::tr("Timer fired. %1").
                arg(GetWorld()->TimeOfDayStr());
            timerTime = -1;
            return INNER_ACTION_MESSAGE;
        }
        return INNER_ACTION_NONE;
    }

    bool Clock::Inscribe(const QString str) {
        Block::Inscribe(str);
        char c;
        QTextStream txt_stream(note);
        txt_stream >> c;
        if ( 'a'==c ) {
            ushort alarm_hour;
            txt_stream >> alarm_hour;
            txt_stream >> alarmTime;
            alarmTime+=alarm_hour*60;
            timerTime=-1;
        } else if ( 't'==c ) {
            txt_stream >> timerTime;
            alarmTime=-1;
        } else {
            alarmTime=timerTime=-1;
        }
        return true;
    }

    Clock::Clock(const int sub, const quint16 id) :
            Active(sub, id, NONSTANDARD),
            alarmTime(-1),
            timerTime(-1)
    {}
    Clock::Clock (QDataStream & str, const int sub, const quint16 id) :
            Active(str, sub, id, NONSTANDARD),
            alarmTime(-1),
            timerTime(-1)
    {
        if ( note ) {
            Inscribe(*note);
        }
    }
// Creator::
    quint8 Creator::Kind() const { return CREATOR; }
    int Creator::Sub() const { return Block::Sub(); }
    QString Creator::FullName() const { return tr("Creative block"); }
    int Creator::DamageKind() const { return TIME; }
    ushort Creator::DamageLevel() const { return MAX_DURABILITY; }
    Inventory * Creator::HasInventory() { return Inventory::HasInventory(); }

    void Creator::ReceiveSignal(const QString str) {
        Active::ReceiveSignal(str);
    }

    void Creator::SaveAttributes(QDataStream & out) const {
        Active::SaveAttributes(out);
        Inventory::SaveAttributes(out);
    }

    Creator::Creator(const int sub, const quint16 id) :
            Active(sub, id, NONSTANDARD),
            Inventory(INV_SIZE)
    {}
    Creator::Creator(QDataStream & str, const int sub, const quint16 id) :
            Active(str, sub, id, NONSTANDARD),
            Inventory(str, INV_SIZE)
    {}
// Text::
    quint8 Text::Kind() const { return TEXT; }
    QString Text::FullName() const {
        switch ( Sub() ) {
        case PAPER: return QObject::tr("Paper page");
        case GLASS: return QObject::tr("Screen");
        default:
            fprintf(stderr, "Text::FullName: sub ?: %d\n", Sub());
            return QObject::tr("Strange text");
        }
    }

    usage_types Text::Use(Block * const who) {
        if ( note ) {
            return USAGE_TYPE_READ;
        } else {
            who->ReceiveSignal(QObject::tr(
                "Nothing is written on this page."));
            return USAGE_TYPE_NO;
        }
    }

    bool Text::Inscribe(const QString str) {
        if ( '.' != str.at(0) && (note == nullptr || GLASS == Sub()) ) {
            Block::Inscribe(str);
            return true;
        } else {
            return false;
        }
    }

    Text::Text(const int sub, const quint16 id) :
            Block(sub, id, NONSTANDARD)
    {}
    Text::Text(QDataStream & str, const int sub, const quint16 id) :
            Block(str, sub, id, NONSTANDARD)
    {}
// Map::
    quint8 Map::Kind() const { return MAP; }
    QString Map::FullName() const { return QObject::tr("Map"); }

    usage_types Map::Use(Block * const who) {
        if ( note == nullptr ) {
            if ( who ) {
                who->ReceiveSignal(QObject::tr(
                    "Set title to this map first."));
            }
            return USAGE_TYPE_NO;
        } else if ( who && who->ActiveBlock() ) {
            const Active * const active = who->ActiveBlock();
            QFile map_file(active->GetWorld()->
                WorldName() + "/texts/" + *note + ".txt");
            if ( not map_file.open(QIODevice::ReadWrite | QIODevice::Text) ) {
                return USAGE_TYPE_READ;
            }
            const Shred * const shred = active->GetShred();
            const long  lati = shred->Latitude();
            const long longi = shred->Longitude();
            static const ushort FILE_SIZE_CHARS = 31;
            if ( 0 == map_file.size() ) { // new map
                char header[FILE_SIZE_CHARS+1];
                memset(header, '-', FILE_SIZE_CHARS);
                header[0] = header[FILE_SIZE_CHARS/2] =
                    header[FILE_SIZE_CHARS-1] = '+';
                char body[FILE_SIZE_CHARS+1];
                memset(body, ' ', FILE_SIZE_CHARS);
                body[0] = body[FILE_SIZE_CHARS-1] = '|';
                body[FILE_SIZE_CHARS] = header[FILE_SIZE_CHARS] = '\n';
                map_file.write(header, FILE_SIZE_CHARS+1);
                for (ushort i=0; i<FILE_SIZE_CHARS-2; ++i) {
                    map_file.write(body,FILE_SIZE_CHARS+1);
                }
                map_file.write(header, FILE_SIZE_CHARS+1);

                map_file.seek( FILE_SIZE_CHARS/2 * (FILE_SIZE_CHARS+1) );
                map_file.putChar('+');
                map_file.seek(FILE_SIZE_CHARS/2*
                    (FILE_SIZE_CHARS+1)+FILE_SIZE_CHARS-1);
                map_file.putChar('+');

                longiStart = longi;
                latiStart  = lati;
            }
            if ( ( qAbs(lati-latiStart) > FILE_SIZE_CHARS/2 )
                    || ( qAbs(longi-longiStart) > FILE_SIZE_CHARS/2 ) )
            {
                return USAGE_TYPE_READ;
            }
            if ( savedChar ) {
                map_file.seek(savedShift);
                map_file.putChar(savedChar);
            }
            map_file.seek(savedShift=(FILE_SIZE_CHARS+1)*
                (longi-longiStart+FILE_SIZE_CHARS/2)+
                 lati -latiStart +FILE_SIZE_CHARS/2);
            map_file.putChar('@');
            savedChar=active->GetWorld()->TypeOfShred(longi, lati);
            map_file.seek((FILE_SIZE_CHARS+1)*FILE_SIZE_CHARS-1);
            map_file.write("\n @ = ");
            map_file.putChar(savedChar);
            map_file.putChar('\n');
        }
        return USAGE_TYPE_READ;
    } // usage_types Map::Use(Block * who)

    void Map::SaveAttributes(QDataStream & out) const {
        out << longiStart << latiStart << savedShift << savedChar;
    }

    Map::Map(const int sub, const quint16 id) :
            Text(sub, id),
            longiStart(),
            latiStart(),
            savedShift(),
            savedChar(0)
    {}
    Map::Map(QDataStream & str, const int sub, const quint16 id) :
            Text(str, sub, id)
    {
        str >> longiStart >> latiStart >> savedShift >> savedChar;
    }
// Bell::
    quint8 Bell::Kind() const { return BELL; }
    QString Bell::FullName() const { return QObject::tr("Bell"); }

    usage_types Bell::Use(Block  * const) {
        SendSignalAround(DING);
        return USAGE_TYPE_NO;
    }

    void Bell::ReceiveSignal(const QString str) {
        if ( DING != str ) {
            SendSignalAround(DING);
        }
    }

    Bell::Bell(const int sub, const quint16 id) :
            Active(sub, id)
    {}
    Bell::Bell(QDataStream & str, const int sub, const quint16 id) :
            Active(str, sub, id)
    {}
// Predator::
    Predator::Predator(const int sub, const quint16 id) :
            Animal(sub, id)
    {}
    Predator::Predator(QDataStream & str, const int sub, const quint16 id)
            : Animal(str, sub, id)
    {}

    ushort Predator::DamageLevel() const { return 10; }
    quint8 Predator::Kind() const { return PREDATOR; }
    QString Predator::FullName() const { return "Predator"; }
    quint16 Predator::NutritionalValue(quint8 sub) const {
        return Attractive(sub)*SECONDS_IN_HOUR;
    }

    void Predator::DoFrequentAction() {
        if ( Gravitate(5, 1, 2, 0) ) {
            world->Move(X(), Y(), Z(), GetDir());
        }
    }
    void Predator::DoRareAction() {
        const Xyz coords[] = {
            Xyz(X()-1, Y(), Z()),
            Xyz(X()+1, Y(), Z()),
            Xyz(X(), Y()-1, Z()),
            Xyz(X(), Y()+1, Z()),
            Xyz(X(), Y(), Z()-1),
            Xyz(X(), Y(), Z()+1) };
        World * const world = GetWorld();
        for (const Xyz xyz : coords) {
            const ushort x = xyz.GetX();
            const ushort y = xyz.GetY();
            const ushort z = xyz.GetZ();
            Block * const block = world->GetBlock(x, y, z);
            if ( Attractive(block->Sub()) ) {
                world->GetBlock(x, y, z)->ReceiveSignal(
                    tr("Predator bites you!"));
                world->Damage(x, y, z, DamageLevel(), DamageKind());
                Eat(block->Sub());
            }
        }
        if ( SECONDS_IN_DAY/4 > Satiation() ) {
            EatGrass();
        }
        Animal::DoRareAction();
    }

    short Predator::Attractive(const int sub) const {
        switch ( sub ) {
        default: return 0;
        case A_MEAT:
        case H_MEAT: return 10;
        case GREENERY: return 1;
        }
    }
