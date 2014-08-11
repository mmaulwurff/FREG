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

inner_actions Animal::ActInner() {
    if ( Sub() != H_MEAT && Sub() != A_MEAT ) return INNER_ACTION_NONE;
    if ( satiation <= 0 ) {
        Damage(5, DAMAGE_HUNGER);
    } else {
        --satiation;
        Mend();
    }
    emit Updated();
    return INNER_ACTION_NONE;
}

void Animal::ActFrequent() {
    if ( defActionPending ) {
        defActionPending = false;
        deferredAction->MakeAction();
        delete deferredAction;
    }
}

void Animal::DoRareAction() {
    if ( Sub() != H_MEAT && Sub() != A_MEAT ) return; // mechanical
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

int Animal::Breath() const { return breath; }
int Animal::Satiation() const { return satiation; }
int Animal::ShouldAct() const { return FREQUENT_SECOND | FREQUENT_RARE; }
int Animal::DamageKind() const { return DAMAGE_BITE; }
int Animal::NutritionalValue(subs) const { return 0; }
Animal * Animal::IsAnimal() { return this; }

bool Animal::Eat(const subs sub) {
    const int value = NutritionalValue(sub);
    if ( value ) {
        satiation += value;
        ReceiveSignal(tr("Ate."));
        if ( SECONDS_IN_DAY < satiation ) {
            satiation = 1.1 * SECONDS_IN_DAY;
            ReceiveSignal(tr("You have gorged yourself!"));
        }
        return true;
    } else {
        return false;
    }
}

void Animal::SaveAttributes(QDataStream & out) const {
    Falling::SaveAttributes(out);
    out << breath << satiation;
}

void Animal::EatGrass() {
    for (int x=X()-1; x<=X()+1; ++x)
    for (int y=Y()-1; y<=Y()+1; ++y) {
        if ( world->InBounds(x, y) &&
                GREENERY == world->GetBlock(x, y, Z())->Sub() )
        {
            TryDestroy(x, y, Z());
            Eat(GREENERY);
            return;
        }
    }
}

Block * Animal::DropAfterDamage(bool *) {
    Block * const cadaver = BlockManager::NewBlock(CONTAINER, Sub());
    cadaver->HasInventory()->Get(BlockManager::NewBlock(WEAPON, BONE));
    return cadaver;
}

void Animal::SetDeferredAction(DeferredAction * const action) {
    if ( defActionPending ) {
        delete deferredAction;
    }
    deferredAction = action;
    defActionPending = true;
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
