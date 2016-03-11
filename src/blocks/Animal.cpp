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

#include "blocks/Animal.h"
#include "blocks/Inventory.h"
#include "DeferredAction.h"
#include "BlockFactory.h"
#include "World.h"
#include "TrManager.h"
#include "AroundCoordinates.h"
#include "RandomManager.h"
#include "ActiveWatcher.h"

#include <QDataStream>

// Animal:: section
    inner_actions Animal::ActInner() {
        if ( GROUP_MEAT != GetSubGroup(Sub()) ) return INNER_ACTION_NONE;
        if ( satiation <= 0 ) {
            Damage(5, DAMAGE_HUNGER);
        } else {
            --satiation;
            Mend(1);
        }
        return INNER_ACTION_NONE;
    }

    void Animal::ActFrequent() {
        if ( deferredAction ) deferredAction->MakeAction();
    }

    void Animal::DoRareAction() {
        if ( GROUP_MEAT != GetSubGroup(Sub()) ) return;
        EatAround();
        if ( not IsSubAround(AIR) ) {
            if ( breath == 0 ) {
                Damage(10, DAMAGE_BREATH);
            } else {
                --breath;
            }
        } else if ( breath < MAX_BREATH ) {
            ++breath;
        }
        if ( GetDurability() <= 0 ) {
            World::GetWorld()->DestroyAndReplace(X(), Y(), Z());
        } else {
            Updated();
        }
    }

    void Animal::Damage(const int dmg, const int dmg_kind) {
        const int last_durability = GetDurability();
        Falling::Damage(dmg, dmg_kind);
        if ( last_durability != GetDurability() ) {
            TrString hungerString = QObject::tr("You weaken from hunger!");
            TrString burnString   = QObject::tr("You burn!");
            TrString chokeString  = QObject::tr("You choke without air!");
            TrString damageString = QObject::tr("Received damage!");

            switch ( dmg_kind ) {
            case DAMAGE_HUNGER: ReceiveSignal(hungerString); break;
            case DAMAGE_HEAT:   ReceiveSignal(burnString);   break;
            case DAMAGE_BREATH: ReceiveSignal(chokeString);  break;
            default:            ReceiveSignal(damageString); break;
            }
            Updated();
        }
    }

    int Animal::ShouldAct() const { return FREQUENT_SECOND | FREQUENT_RARE; }
    int Animal::DamageKind() const { return DAMAGE_BITE; }
    Animal* Animal::IsAnimal() { return this; }
    QString Animal::FullName() const { return TrManager::KindName(Kind()); }

    bool Animal::Eat(const subs sub) {
        const int value = NutritionalValue(sub);
        if ( value ) {
            satiation += value;
            TrString ateString = QObject::tr("Ate %1.");
            ReceiveSignal(ateString.arg(TrManager::SubName(sub)));
            if ( World::SECONDS_IN_DAY < satiation ) {
                satiation = 1.1 * World::SECONDS_IN_DAY;
                TrString gorged = QObject::tr("You have gorged yourself!");
                ReceiveSignal(gorged);
            }
            Updated();
            return true;
        } else {
            return false;
        }
    }

    void Animal::SaveAttributes(QDataStream& out) const {
        Falling::SaveAttributes(out);
        out << breath << satiation;
    }

    void Animal::EatAround() {
        World* const world = World::GetWorld();
        for (const XyzInt& xyz : AroundCoordinates(X(), Y(), Z())) {
            const subs sub = world->GetBlock(XYZ(xyz))->Sub();
            if ( Attractive(sub) ) {
                world->Damage(XYZ(xyz), DamageLevel(), DamageKind());
                Eat(sub);
            }
        }
    }

    Block* Animal::DropAfterDamage(bool*) {
        Block* const cadaver = BlockFactory::NewBlock(BOX, Sub());
        cadaver->HasInventory()->Get(BlockFactory::NewBlock(WEAPON, BONE));
        return cadaver;
    }

    DeferredAction* Animal::GetDeferredAction() {
        return ( deferredAction == nullptr ) ?
            (deferredAction = new DeferredAction(this)) :
            deferredAction;
    }

    Animal::Animal(const kinds kind, const subs sub)
        : Falling(kind, sub)
        , moved_in_this_turn(false)
        , breath(MAX_BREATH)
        , satiation(World::SECONDS_IN_DAY)
        , deferredAction(nullptr)
    {}

    Animal::Animal(QDataStream& str, const kinds kind, const subs sub)
        : Falling(str, kind, sub)
        , moved_in_this_turn(false)
        , breath()
        , satiation()
        , deferredAction(nullptr)
    {
        str >> breath >> satiation;
    }

    Animal::~Animal() {
        delete deferredAction;
        deferredAction = nullptr;
    }

    void Animal::Updated() const { if (watcher) watcher->Updated(); }
    void Animal::CauseTeleportation() const {
        if (watcher) watcher->CauseTeleportation();
    }

// Rabbit::section
    int Rabbit::Attractive(const int sub) const {
        switch ( sub ) {
        case GREENERY: return Satiation() < World::SECONDS_IN_DAY/2;
        case H_MEAT:   return -16;
        case A_MEAT:   return - 1;
        case SAND:     return - 1;
        default:       return   0;
        }
    }

    void Rabbit::DoRareAction() {
        if ( not moved_in_this_turn ) {
            const int rand = RandomManager::getRandBit8();
            if ( rand < 64 ) {
                World::GetWorld()->Move(X(), Y(), Z(),
                    static_cast<dirs>(DOWN + (rand % 4)));
            } else if ( Gravitate(4, 1, 3, 4) ) {
                if ( rand % 2 ) {
                    World::GetWorld()->Jump(X(), Y(), Z(), GetDir());
                } else {
                    World::GetWorld()->Move(X(), Y(), Z(), GetDir());
                }
            }
        }
        moved_in_this_turn = false; // for next turn
        Animal::DoRareAction();
    }

    void Rabbit::ActFrequent() {
        if ( Gravitate(2, 1, 2, 4) ) {
            if ( RandomManager::getRandBit1() ) {
                World::GetWorld()->Jump(X(), Y(), Z(), GetDir());
            } else {
                World::GetWorld()->Move(X(), Y(), Z(), GetDir());
            }
            moved_in_this_turn = true;
        }
        Animal::ActFrequent();
    }

    int Rabbit::NutritionalValue(const subs sub) const {
        return ( GREENERY == sub ) ? World::SECONDS_IN_HOUR*4 : 0;
    }

// Predator:: section
    int Predator::DamageLevel() const { return 10; }

    int Predator::NutritionalValue(const subs sub) const {
        return Attractive(sub) * World::SECONDS_IN_HOUR;
    }

    void Predator::ActFrequent() {
        if ( Gravitate(5, 1, 2, 0) ) {
            World::GetWorld()->Move(X(), Y(), Z(), GetDir());
        }
    }

    int Predator::Attractive(const int sub) const {
        return ( 2 * Satiation() / World::SECONDS_IN_DAY ) == 0 ?
            0 : ( GROUP_MEAT == GetSubGroup(sub) ) ?
                10 : ( sub == GREENERY ) ?
                    1 : 0;
    }
