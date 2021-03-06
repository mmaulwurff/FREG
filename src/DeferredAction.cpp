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

#include "World.h"
#include "DeferredAction.h"
#include "Id.h"

#include "blocks/Animal.h"
#include "blocks/Inventory.h"
#include "blocks/blocks.h"

void DeferredAction::GhostMove() const {
    const dirs direction = static_cast<dirs>(num);
    if ( direction == DOWN && attachedBlock->Z() == 1        ) return;
    if ( direction == UP   && attachedBlock->Z() == HEIGHT-2 ) return;
    attachedBlock->Move(direction);
}

void DeferredAction::Move() const {
    World::GetWorld()->Move(attachedBlock->X(), attachedBlock->Y(),
        attachedBlock->Z(), static_cast<dirs>(num));
}

void DeferredAction::Jump() const {
    World::GetWorld()->Jump(
        attachedBlock->X(), attachedBlock->Y(), attachedBlock->Z(),
        attachedBlock->GetDir());
}

void DeferredAction::Build() const {
    World* const world = World::GetWorld();
    int x = X(), y = Y(), z = Z();
    if ( ENVIRONMENT != world->GetBlock(x, y, z)->PushResult(ANYWHERE) ) {
        if ( world->Move(
                attachedBlock->X(), attachedBlock->Y(), attachedBlock->Z(),
                World::Anti(attachedBlock->GetDir())) )
        { // shift coordinates to opposite side:
            switch ( attachedBlock->GetDir() ) {
            case UP:    --z; break;
            case DOWN:  ++z; break;
            case NORTH: ++y; break;
            case EAST:  --x; break;
            case SOUTH: --y; break;
            case WEST:  ++x; break;
            }
        } else {
            return;
        }
    }
    Inventory* const inv = attachedBlock->HasInventory();
    Block* const material = inv->ShowBlock(srcSlot);
    if ( not world->Build(material, x, y, z, attachedBlock) ) {
        return; // build not successful
    }
    inv->Pull(srcSlot);
    // put more material in building inventory slot:
    if ( not inv->IsEmpty(srcSlot) ) return;
    const Id id(material->Kind(), material->Sub());
    for (int i = srcSlot+1; i < inv->GetSize() &&
            inv->Count(srcSlot) < Inventory::MAX_STACK_SIZE; ++i)
    {
        const Block* const block = inv->ShowBlock(i);
        if ( block && id==Id(block->Kind(), block->Sub()) ) {
            inv->MoveInside(i, srcSlot, inv->Count(i));
        }
    }
} // void DeferredAction::Build()

void DeferredAction::Damage() const {
    if ( World::GetWorld()->Damage(X(), Y(), Z(),
            attachedBlock->DamageLevel(), attachedBlock->DamageKind()) <= 0 )
    {
        World::GetWorld()->DestroyAndReplace(X(), Y(), Z());
    }
}

void DeferredAction::Throw() const {
    World::GetWorld()->Drop(attachedBlock, X(), Y(), Z(),
        srcSlot, destSlot, num);
}

void DeferredAction::Pour() const {
    Inventory* const attached_inv = attachedBlock->HasInventory();
    if ( attached_inv == nullptr ) return;

    Block* const vessel = attached_inv->ShowBlock(srcSlot);
    if ( vessel == nullptr ) return;

    Inventory* const vessel_inv = vessel->HasInventory();
    if ( vessel_inv == nullptr ) return;

    Block* const liquid = vessel_inv->ShowBlock(0);
    if ( liquid == nullptr ) return;

    if ( World::GetWorld()->Build(liquid, X(), Y(), Z()) ) {
        vessel_inv->Pull(0);
    }
}

void DeferredAction::SetFire() const {
    World* const world = World::GetWorld();
    if ( world->GetBlock(X(), Y(), Z())->Sub() == AIR ) {
        world->Build(new Grass(GRASS, FIRE), X(), Y(), Z());
    } else if ( world->Damage(X(), Y(), Z(), 1, DAMAGE_HEAT) <= 0 ) {
        world->DestroyAndReplace(X(), Y(), Z());
    }
}

void DeferredAction::SetGhostMove(const int direction) {
    type = DEFERRED_GHOST_MOVE;
    num  = direction;
}

void DeferredAction::SetMove(const int direction) {
    type = DEFERRED_MOVE;
    num  = direction;
}

void DeferredAction::SetJump() { type = DEFERRED_JUMP; }

void DeferredAction::SetBuild(const_int(x, y, z), const int builder_slot) {
    SetXyz(x, y, z);
    srcSlot  = builder_slot;
    type     = DEFERRED_BUILD;
}

void DeferredAction::SetDamage(const_int(x, y, z)) {
    SetXyz(x, y, z);
    type = DEFERRED_DAMAGE;
}

void DeferredAction::SetThrow(const_int (x, y, z), const_int(src, dest, n)) {
    SetXyz(x, y, z);
    srcSlot  = src;
    destSlot = dest;
    num  = n;
    type = DEFERRED_THROW;
}

void DeferredAction::SetPour(const_int(x, y, z), const int src) {
    SetXyz(x, y, z);
    srcSlot = src;
    type = DEFERRED_POUR;
}

void DeferredAction::SetSetFire(const_int(x, y, z)) {
    SetXyz(x, y, z);
    type = DEFERRED_SET_FIRE;
}

void DeferredAction::MakeAction() {
    switch ( type ) {
    case DEFERRED_NOTHING: break;
    case DEFERRED_GHOST_MOVE: GhostMove(); break;
    case DEFERRED_MOVE:   Move();   break;
    case DEFERRED_JUMP:   Jump();   break;
    case DEFERRED_BUILD:  Build();  break;
    case DEFERRED_DAMAGE: Damage(); break;
    case DEFERRED_THROW:  Throw();  break;
    case DEFERRED_POUR:   Pour();   break;
    case DEFERRED_SET_FIRE: SetFire(); break;
    }
    type = DEFERRED_NOTHING;
}

DeferredAction::DeferredAction(Animal* const attached)
    : Xyz()
    , type(DEFERRED_NOTHING)
    , srcSlot()
    , destSlot()
    , num()
    , attachedBlock(attached)
{}
