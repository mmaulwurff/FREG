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

#include "blocks/Containers.h"
#include "World.h"
#include "Shred.h"
#include "BlockManager.h"
#include "CraftManager.h"

const int CONVERTER_LIGHT_RADIUS = 2;

// Container::
    void Container::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind >= DAMAGE_PUSH_UP ) {
            int x, y, z;
            World * const world = GetWorld();
            world->Focus( X(), Y(), Z(), &x, &y, &z,
                World::Anti(MakeDirFromDamage(dmg_kind)) );
            Inventory::Push(world->GetBlock(x, y, z));
            if ( Sub()==DIFFERENT && IsEmpty() ) {
                Break();
            }
        } else {
            Block::Damage(dmg, dmg_kind);
        }
    }

    void Container::DoRareAction() {
        switch ( Sub() ) {
        default: break;
        case DIFFERENT: {
                Inventory * const inv =
                    GetWorld()->GetBlock(X(), Y(), Z()-1)->HasInventory();
                if ( inv != nullptr ) {
                    inv->GetAll(this);
                }
                if ( IsEmpty() ) {
                    GetWorld()->DestroyAndReplace(X(), Y(), Z());
                }
            } break;
        case A_MEAT:
        case H_MEAT:
            Damage(MAX_DURABILITY/SECONDS_IN_DAY, DAMAGE_TIME);
            if ( GetDurability() <= 0 ) {
                GetWorld()->DestroyAndReplace(X(), Y(), Z());
            }
            break;
        }
    }

    int Container::ShouldAct() const {
        return ( Sub() == IRON || Sub() == WATER ) ?
            FREQUENT_NEVER : FREQUENT_RARE;
    }

    void Container::ReceiveSignal(QString) {}
    Inventory * Container::HasInventory() { return this; }
    push_reaction Container::PushResult(dirs) const { return NOT_MOVABLE; }
    inner_actions Container::ActInner() { return INNER_ACTION_NONE; }

    usage_types Container::Use(Block *) {
        if ( Sub() == A_MEAT || Sub() == H_MEAT ) {
            GetWorld()->DestroyAndReplace(X(), Y(), Z());
            return USAGE_TYPE_NO;
        } else {
            return USAGE_TYPE_OPEN;
        }
    }

    Block * Container::DropAfterDamage(bool * const delete_block) {
        if ( DIFFERENT == Sub() ) {
            *delete_block = true;
            return block_manager.Normal(AIR);
        } // else:
        Block * const pile = BlockManager::NewBlock(CONTAINER, DIFFERENT);
        Inventory * const pile_inv = pile->HasInventory();
        GetAll(pile_inv);
        *delete_block = not pile_inv->Get(this);
        return pile;
    }

    Active * Container::ActiveBlock() {
        return ( Sub() == IRON || Sub() == WATER) ?
            nullptr : this;
    }

    int Container::Weight() const {
        return Block::Weight()*4 + Inventory::Weight();
    }

    QString Container::FullName() const {
        switch ( Sub() ) {
        case DIFFERENT: return tr("Pile");
        case IRON:      return tr("Locker");
        case WATER:     return tr("Fridge");
        case A_MEAT:
        case H_MEAT:    return tr("Corpse");
        default:        return tr("Chest (%1)").arg(SubName(Sub()));
        }
    }

    void Container::SaveAttributes(QDataStream & out) const {
        Active::SaveAttributes(out);
        Inventory::SaveAttributes(out);
    }

    Container::Container(const int kind, const int sub, const int size) :
            Active(kind, sub, NONSTANDARD),
            Inventory(size)
    {}

    Container::Container(QDataStream & str, const int kind, const int sub,
            const int size)
        :
            Active(str, kind, sub, NONSTANDARD),
            Inventory(str, size)
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
        CraftList list(materials_number);
        for (int i=Start(); i<Size(); ++i) {
            if ( Number(i) ) {
                list << new CraftItem(
                    {Number(i), ShowBlock(i)->Kind(), ShowBlock(i)->Sub()} );
            }
        }
        if ( craft_manager->Craft(&list, Sub()) ) {
            for (int i=0; i<list.size(); ++i) {
                for (int n=0; n<list.at(i)->num; ++n) {
                    const CraftItem * const item = list.at(i);
                    GetExact(BlockManager::NewBlock(item->kind, item->sub), i);
                }
            }
        }
    }

    bool Workbench::Drop(const int src, const int dest,
            const int num, Inventory * const inv_to)
    {
        if ( inv_to == nullptr
                || src  >= Size()
                || dest >= inv_to->Size()
                || Number(src) == 0 )
        {
            return false;
        }
        for (int i=0; i<num; ++i) {
            if ( not inv_to->Get(ShowBlock(src), dest) ) return false;
            Pull(src);
            if ( src < Start() ) {
                // remove materials:
                for (int i=Start(); i<Size(); ++i) {
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
        case WOOD: return tr("Workbench");
        default:   return tr("Anvil (%1)").arg(SubName(Sub()));
        }
    }

    QString Workbench::InvFullName(const int slot_number) const {
        return ( slot_number < Start() ) ?
            tr("-product-") : tr("-material-");
    }

    int Workbench::Start() const { return 2; }

    bool Workbench::Get(Block * const block, const int start) {
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

    Workbench::Workbench(const int kind, const int sub) :
            Container(kind, sub, WORKBENCH_SIZE)
    {}

    Workbench::Workbench(QDataStream & str, const int kind, const int sub) :
            Container(str, kind, sub, WORKBENCH_SIZE)
    {}

// Converter
    Converter::Converter(const int kind, const int sub) :
            Container(kind, sub, WORKBENCH_SIZE),
            isOn(false),
            fuelLevel(0),
            lightRadius(0),
            damageKindOn(),
            damageKindOff()
    {
        InitDamageKinds();
    }

    Converter::Converter(QDataStream & str, const int kind, const int sub) :
            Container(str, kind, sub, WORKBENCH_SIZE),
            isOn(),
            fuelLevel(),
            lightRadius(),
            damageKindOn(),
            damageKindOff()
    {
        str >> isOn >> fuelLevel;
        lightRadius = isOn ? CONVERTER_LIGHT_RADIUS : 0;
        InitDamageKinds();
    }

    void Converter::SaveAttributes(QDataStream & out) const {
        Container::SaveAttributes(out);
        out << isOn << fuelLevel;
    }

    int Converter::ShouldAct() const { return FREQUENT_RARE; }
    int Converter::LightRadius() const { return lightRadius; }
    QString Converter::InvFullName(int) const { return tr("-fuel-"); }

    int Converter::DamageKind() const {
        return (fuelLevel > 0) ? damageKindOn : 0;
    }

    void Converter::DoRareAction() {
        if ( isOn && fuelLevel < SECONDS_IN_DAY/DamageLevel() ) {
            for (int i=Size()-1; i>=0; --i) {
                Block * const block = ShowBlock(i);
                if ( block != nullptr ) {
                    const int add = ConvertRatio(block->Sub());
                    if ( add > 0 ) {
                        fuelLevel += add;
                        Pull(i);
                        block_manager.DeleteBlock(block);
                        break;
                    }
                }
            }
        }
        World * const world = GetWorld();
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
        QString name;
        switch ( Sub() ) {
            default:
            case STONE: name = tr("Furnace"); break;
        }
        return name + tr(" (charge for %1 s)").arg(fuelLevel/DamageLevel());
    }

    void Converter::InitDamageKinds() {
        switch ( Sub() ) {
        default:
            fprintf(stderr, "%s: sub ?: %d.\n", Q_FUNC_INFO, Sub());
            // no break;
        case STONE:
            damageKindOn  = DAMAGE_HEAT;
            damageKindOff = DAMAGE_FREEZE;
            break;
        }
    }

    void Converter::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind == damageKindOn ) {
            if ( not isOn ) {
                isOn = true;
                GetWorld()->GetShred(X(), Y())->AddShining(this);
                GetWorld()->Shine(X(), Y(), Z(),
                    (lightRadius=CONVERTER_LIGHT_RADIUS), true);
            } else {
                fuelLevel += dmg;
            }
        } else if ( dmg_kind == damageKindOff ) {
            isOn = false;
            fuelLevel = 0;
            lightRadius = 0;
            GetWorld()->GetShred(X(), Y())->RemShining(this);
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
