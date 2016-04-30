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

#include "blocks/Containers.h"
#include "World.h"
#include "Shred.h"
#include "BlockFactory.h"
#include "TrManager.h"

#include <QDataStream>

// Container::
    void Container::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind >= DAMAGE_PUSH_UP ) {
            Push(X(), Y(), Z(), dmg_kind);
        } else {
            Block::Damage(dmg, dmg_kind);
        }
    }

    int Container::ShouldAct() const {
        return ( Sub() == IRON || Sub() == WATER ) ?
            FREQUENT_NEVER : FREQUENT_RARE;
    }

    void Container::ReceiveSignal(const QString&) {}
    usage_types Container::Use(Active*) { return USAGE_TYPE_OPEN; }
    push_reaction Container::PushResult(dirs) const { return NOT_MOVABLE; }
    inner_actions Container::ActInner() { return INNER_ACTION_ONLY; }

    Block* Container::DropAfterDamage(bool* const delete_block) {
        Block* const pile = new Box(DIFFERENT);
        Inventory* const pile_inv = pile->HasInventory();
        GetAll(pile_inv);
        *delete_block = not pile_inv->Get(this);
        return pile;
    }

    int Container::Weight() const {
        return Block::Weight()*4 + Inventory::Weight();
    }

    QString Container::FullName() const {
        TrString ironName  = QObject::tr("Locker");
        TrString waterName = QObject::tr("Fridge");

        switch ( Sub() ) {
        case IRON:      return ironName;
        case WATER:     return waterName;
        default:        return Block::FullName();
        }
    }

    void Container::SaveAttributes(QDataStream& out) const {
        Inventory::SaveAttributes(out);
    }

    Container::Container(const kinds kind, const subs sub, const int size)
        : Active(kind, sub)
        , Inventory(size)
    {}

    Container::Container(QDataStream& str, const kinds kind, const subs sub,
                         const int size)
        : Active(str, kind, sub)
        , Inventory(str, size)
    {}

// Box::
    Box::Box(const kinds kind, const subs sub)
        : Falling(kind, sub)
        , Inventory(INV_SIZE)
    {}

    Box::Box(QDataStream& str, const kinds kind, const subs sub)
        : Falling(str, kind, sub)
        , Inventory(str, INV_SIZE)
    {}

    void Box::SaveAttributes(QDataStream& str) const {
        Falling::SaveAttributes(str);
        Inventory::SaveAttributes(str);
    }

    int  Box::ShouldAct() const { return FREQUENT_RARE; }
    void Box::ReceiveSignal(const QString& str) { Active::ReceiveSignal(str); }
    inner_actions Box::ActInner() { return INNER_ACTION_NONE; }

    void Box::DoRareAction() {
        if ( GROUP_MEAT == GetSubGroup(Sub()) ) {
            Damage(MAX_DURABILITY/World::SECONDS_IN_DAY, DAMAGE_TIME);
            if ( GetDurability() <= 0 ) {
                World::GetWorld()->DestroyAndReplace(X(), Y(), Z());
            }
        } else if ( Sub() == DIFFERENT ) {
            World* const world = World::GetWorld();
            Inventory* const inv =
                world->GetBlock(X(), Y(), Z()-1)->HasInventory();
            if ( inv ) {
                inv->GetAll(this);
            }
            if ( IsEmpty() ) {
                world->DestroyAndReplace(X(), Y(), Z());
            }
        }
    }

    Block* Box::DropAfterDamage(bool* const delete_block) {
        *delete_block = true;
        return BlockFactory::Normal(AIR);
    }

    void Box::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind >= DAMAGE_PUSH_UP ) {
            Push(X(), Y(), Z(), dmg_kind);
            if ( Sub()==DIFFERENT && IsEmpty() ) {
                Break();
            }
        } else {
            Block::Damage(dmg, dmg_kind);
        }
    }

    QString Box::FullName() const {
        TrString differentName = QObject::tr("Pile");
        TrString meatName      = QObject::tr("Corpse (%1)");

        switch ( Sub() ) {
        default:        return Block::FullName();
        case DIFFERENT: return differentName;
        case H_MEAT:
        case A_MEAT:    return meatName.arg(TrManager::SubName(Sub()));
        }
    }

    usage_types Box::Use(Active*) {
        if ( GROUP_MEAT == GetSubGroup(Sub()) ) {
            World::GetWorld()->DestroyAndReplace(X(), Y(), Z());
            return USAGE_TYPE_NO;
        } else {
            return USAGE_TYPE_OPEN;
        }
    }

// Workbench::
    void Workbench::Craft() {
        ///@todo implement craft
    }

    bool Workbench::Drop(const int src, const int dest,
            const int num, Inventory* const inv_to)
    {
        if ( inv_to == nullptr
                || src  >= GetSize()
                || dest >= inv_to->GetSize()
                || IsEmpty(src) )
        {
            return false;
        }
        for (int i=0; i<num; ++i) {
            if ( not inv_to->Get(ShowBlock(src), dest) ) return false;
            Pull(src);
            if ( src < Start() ) {
                // remove materials:
                for (int slot=Start(); slot<GetSize(); ++slot) {
                    while ( not IsEmpty(slot) ) {
                        Block* const to_pull = ShowBlock(slot);
                        Pull(slot);
                        delete to_pull;
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
        case WOOD: return TrManager::KindName(WORKBENCH);
        default:   return Block::FullName();
        }
    }

    QString Workbench::InvFullName(const int slot_number) const {
        TrString productString  = QObject::tr("-product-");
        TrString materialString = QObject::tr("-material-");

        return IsEmpty(slot_number) ?
            (( slot_number < Start() ) ? productString : materialString) :
            Inventory::InvFullName(slot_number);
    }

    int Workbench::Start() const { return 2; }

    bool Workbench::Get(Block* const block, const int start) {
        if ( Inventory::Get(block, start) ) {
            Craft();
            return true;
        } else {
            return false;
        }
    }

    bool Workbench::GetAll(Inventory* const from) {
        if ( Inventory::GetAll(from) ) {
            Craft();
            return true;
        } else {
            return false;
        }
    }

    Workbench::Workbench(const kinds kind, const subs sub)
        : Container(kind, sub, WORKBENCH_SIZE)
    {}

    Workbench::Workbench(QDataStream& str, const kinds kind, const subs sub)
        : Container(str, kind, sub, WORKBENCH_SIZE)
    {}

// Converter
    Converter::Converter(const kinds kind, const subs sub)
        : Container(kind, sub, WORKBENCH_SIZE)
        , isOn(false)
        , fuelLevel(0)
        , lightRadius(0)
        , damageKindOn()
        , damageKindOff()
    {
        InitDamageKinds();
    }

    Converter::Converter(QDataStream& str, const kinds kind, const subs sub)
        : Container(str, kind, sub, WORKBENCH_SIZE)
        , isOn()
        , fuelLevel()
        , lightRadius()
        , damageKindOn()
        , damageKindOff()
    {
        str >> isOn >> fuelLevel;
        lightRadius = isOn ? CONVERTER_LIGHT_RADIUS : 0;
        InitDamageKinds();
    }

    void Converter::SaveAttributes(QDataStream& out) const {
        Container::SaveAttributes(out);
        out << isOn << fuelLevel;
    }

    int Converter::ShouldAct() const { return FREQUENT_RARE; }
    int Converter::LightRadius() const { return lightRadius; }

    QString Converter::InvFullName(const int slot_number) const {
        TrString fuelString = QObject::tr("-fuel-");
        return IsEmpty(slot_number) ?
            fuelString : Inventory::InvFullName(slot_number);
    }

    int Converter::DamageKind() const {
        return (fuelLevel > 0) ? damageKindOn : 0;
    }

    void Converter::DoRareAction() {
        if ( isOn && fuelLevel < World::SECONDS_IN_DAY/DamageLevel() ) {
            for (int i=GetSize()-1; i>=0; --i) {
                Block* const block = ShowBlock(i);
                if ( block ) {
                    const int add = ConvertRatio(block->Sub());
                    if ( add > 0 ) {
                        fuelLevel += add;
                        Pull(i);
                        delete block;
                        break;
                    }
                }
            }
        }
        World* const world = World::GetWorld();
        if ( fuelLevel <= 0
                || ( Sub() == STONE
                    && world->GetBlock(X(), Y(), Z()+1)->Sub() == WATER ) )
        {
            Damage(1, damageKindOff);
        } else {
            if (world->Damage(X(), Y(), Z()+1, DamageLevel(), damageKindOn)<=0)
            {
                world->DestroyAndReplace(X(), Y(), Z()+1);
            }
            fuelLevel -= DamageLevel();
        }
    }

    QString Converter::FullName() const {
        TrString stoneName    = QObject::tr("Furnace");
        TrString chargeString = QObject::tr(" (charge for %1 s)");

        QString name;
        switch ( Sub() ) {
            default:    name = Block::FullName(); break;
            case STONE: name = stoneName; break;
        }
        return name + chargeString.arg(fuelLevel/DamageLevel());
    }

    void Converter::InitDamageKinds() {
        switch ( Sub() ) {
        default:
            qDebug("%s: sub ?: %d.", Q_FUNC_INFO, Sub());
            // no break;
        case STONE:
            damageKindOn  = DAMAGE_HEAT;
            damageKindOff = DAMAGE_FREEZE;
            break;
        }
    }

    void Converter::Damage(const int dmg, const int dmg_kind) {
        World* const world = World::GetWorld();
        if ( dmg_kind == damageKindOn ) {
            if ( not isOn ) {
                isOn = true;
                world->GetShred(X(), Y())->AddShining(this);
                world->Shine(GetXyz(), (lightRadius=CONVERTER_LIGHT_RADIUS));
            } else {
                fuelLevel += dmg;
            }
        } else if ( dmg_kind == damageKindOff ) {
            isOn = false;
            fuelLevel = 0;
            lightRadius = 0;
            world->GetShred(X(), Y())->RemShining(this);
        } else {
            Block::Damage(dmg, dmg_kind);
        }
    }

    int Converter::ConvertRatio(const int sub) const {
        switch ( Sub() ) {
        default: return 0;
        case STONE: switch ( sub ) {
            default:       return  0;
            case WOOD:     return 50;
            case GREENERY: return 10;
            }
        }
    }
