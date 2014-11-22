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

#include "blocks/Animal.h"
#include "blocks/Inventory.h"
#include "DeferredAction.h"
#include "BlockManager.h"
#include "World.h"

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
        if ( deferredAction != nullptr ) {
            deferredAction->MakeAction();
        }
    }

    void Animal::DoRareAction() {
        if ( GROUP_MEAT != GetSubGroup(Sub()) ) return;
        EatAround();
        if ( not IsSubAround(AIR) ) {
            if ( breath <= 0 ) {
                Damage(10, DAMAGE_BREATH);
            } else {
                --breath;
            }
        } else if ( breath < MAX_BREATH ) {
            ++breath;
        }
        if ( GetDurability() <= 0 ) {
            GetWorld()->DestroyAndReplace(X(), Y(), Z());
        } else {
            emit Updated();
        }
    }

    int Animal::ShouldAct() const { return FREQUENT_SECOND | FREQUENT_RARE; }
    int Animal::DamageKind() const { return DAMAGE_BITE; }
    Animal * Animal::IsAnimal() { return this; }

    bool Animal::Eat(const subs sub) {
        const int value = NutritionalValue(sub);
        if ( value ) {
            satiation += value;
            ReceiveSignal(tr("Ate %1.").arg(tr_manager->SubName(sub)));
            if ( SECONDS_IN_DAY < satiation ) {
                satiation = 1.1 * SECONDS_IN_DAY;
                ReceiveSignal(tr("You have gorged yourself!"));
            }
            emit Updated();
            return true;
        } else {
            return false;
        }
    }

    void Animal::SaveAttributes(QDataStream & out) const {
        Falling::SaveAttributes(out);
        out << breath << satiation;
    }

    void Animal::EatAround() {
        const Xyz coords[] = {
            Xyz(X()-1, Y(), Z()),
            Xyz(X()+1, Y(), Z()),
            Xyz(X(), Y()-1, Z()),
            Xyz(X(), Y()+1, Z()),
            Xyz(X(), Y(), Z()-1)
        };
        World * const world = GetWorld();
        for (const Xyz xyz : coords) {
            if ( not world->InBounds(xyz.X(), xyz.Y()) ) {
                continue;
            }
            const Block * const block =
                world->GetBlock(xyz.X(), xyz.Y(), xyz.Z());
            if ( Attractive(block->Sub()) ) {
                world->Damage(xyz.X(), xyz.Y(), xyz.Z(),
                    DamageLevel(), DamageKind());
                Eat(static_cast<subs>(block->Sub()));
            }
        }
    }

    Block * Animal::DropAfterDamage(bool *) {
        Block * const cadaver = BlockManager::NewBlock(BOX, Sub());
        cadaver->HasInventory()->Get(BlockManager::NewBlock(WEAPON, BONE));
        return cadaver;
    }

    DeferredAction * Animal::GetDeferredAction() {
        return ( deferredAction == nullptr ) ?
            (deferredAction = new DeferredAction(this)) :
            deferredAction;
    }

    Animal::Animal(const int kind, const int sub) :
            Falling(kind, sub, NONSTANDARD),
            breath(MAX_BREATH),
            satiation(SECONDS_IN_DAY)
    {}

    Animal::Animal(QDataStream & str, const int kind, const int sub) :
            Falling(str, kind, sub, NONSTANDARD),
            breath(),
            satiation()
    {
        str >> breath >> satiation;
    }

    Animal::~Animal() {
        delete deferredAction;
        deferredAction = nullptr;
    }

// Rabbit::section
    int Rabbit::Attractive(const int sub) const {
        switch ( sub ) {
        case GREENERY: return ( Satiation() < SECONDS_IN_DAY/2 ) ? 1 : 0;
        case H_MEAT:   return -16;
        case A_MEAT:   return - 1;
        case SAND:     return - 1;
        default:       return   0;
        }
    }

    void Rabbit::DoRareAction() {
        if ( not moved_in_this_turn ) {
            const int rand = qrand() & 255;
            if ( rand < 64 ) {
                GetWorld()->Move(X(), Y(), Z(),
                    static_cast<dirs>(DOWN + (rand & 3)));
            } else if ( Gravitate(4, 1, 3, 4) ) {
                if ( rand & 1 ) {
                    GetWorld()->Jump(X(), Y(), Z(), GetDir());
                } else {
                    GetWorld()->Move(X(), Y(), Z(), GetDir());
                }
            }
        }
        moved_in_this_turn = false; // for next turn
        Animal::DoRareAction();
    }

    QString Rabbit::FullName() const { return tr("Herbivore"); }

    void Rabbit::ActFrequent() {
        if ( Gravitate(2, 1, 2, 4) ) {
            if ( qrand() & 1 ) {
                world->Jump(X(), Y(), Z(), GetDir());
            } else {
                world->Move(X(), Y(), Z(), GetDir());
            }
            moved_in_this_turn = true;
        }
        Animal::ActFrequent();
    }

    int Rabbit::NutritionalValue(const subs sub) const {
        return ( GREENERY == sub ) ? SECONDS_IN_HOUR*4 : 0;
    }

// Predator:: section
    int Predator::DamageLevel() const { return 10; }
    QString Predator::FullName() const { return tr("Predator"); }

    int Predator::NutritionalValue(const subs sub) const {
        return Attractive(sub) * SECONDS_IN_HOUR;
    }

    void Predator::ActFrequent() {
        if ( Gravitate(5, 1, 2, 0) ) {
            world->Move(X(), Y(), Z(), GetDir());
        }
    }

    int Predator::Attractive(const int sub) const {
        return ( 2 * Satiation() / SECONDS_IN_DAY ) == 0 ?
            0 : ( GROUP_MEAT == GetSubGroup(sub) ) ?
                10 : ( sub == GREENERY ) ?
                    1 : 0;
    }
