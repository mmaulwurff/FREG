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

#include "world.h"
#include "blocks/Active.h"
#include "blocks/Inventory.h"
#include "DeferredAction.h"
#include "BlockManager.h"

void DeferredAction::GhostMove() const {
    attachedBlock->Move(( NOWHERE==num ) ?
        attachedBlock->GetDir() : num);
}

void DeferredAction::Move() const {
    GetWorld()->Move(attachedBlock->X(), attachedBlock->Y(),
        attachedBlock->Z(), ( NOWHERE==num ) ?
            attachedBlock->GetDir() : num);
}

void DeferredAction::Jump() const {
    GetWorld()->Jump(
        attachedBlock->X(), attachedBlock->Y(), attachedBlock->Z(),
        attachedBlock->GetDir());
}

void DeferredAction::Build() const {
    short z = z_self;
    if ( DOWN==attachedBlock->GetDir() &&
            AIR!=GetWorld()->GetBlock(x_self, y_self, z_self)->Sub() )
    {
        if ( world->Move(attachedBlock->X(), attachedBlock->Y(),
                attachedBlock->Z(), UP) )
        {
            ++z;
        } else {
            return;
        }
    }
    const quint16 id = material->GetId();
    if ( not world->Build(material, x_self, y_self, z,
            World::TurnRight(attachedBlock->GetDir()), attachedBlock) )
    { // build not successful
        return;
    } // else:
    Inventory * const inv = attachedBlock->HasInventory();
    if ( inv == nullptr ) {
        return;
    } // else:
    inv->Pull(srcSlot);
    // put more material in building inventory slot:
    if ( inv->Number(srcSlot) ) {
        return;
    } // else:
    for (int i=srcSlot+1; i<inv->Size() &&
        inv->Number(srcSlot)<MAX_STACK_SIZE; ++i)
    {
        const Block * const block_i = inv->ShowBlock(i);
        if ( block_i && id==block_i->GetId() ) {
            inv->MoveInside(i, srcSlot, inv->Number(i));
        }
    }
} // void DeferredAction::Build()

void DeferredAction::Damage() const {
    if ( world->Damage(X(), Y(), Z(),
            attachedBlock->DamageLevel(),
            attachedBlock->DamageKind()) <= 0 ) // durability
    {
        world->DestroyAndReplace(X(), Y(), Z());
    }
}

void DeferredAction::Throw() const {
    world->Drop(attachedBlock, x_self, y_self, z_self,
        srcSlot, destSlot, num);
    attachedBlock->EmitUpdated();
}

void DeferredAction::Pour() const {
    Inventory * const attached_inv = attachedBlock->HasInventory();
    if ( attached_inv == nullptr ) return;

    Block * const vessel = attached_inv->ShowBlock(srcSlot);
    if ( vessel == nullptr ) return;

    Inventory * const vessel_inv = vessel->HasInventory();
    if ( vessel_inv == nullptr ) return;

    Block * const liquid = vessel_inv->ShowBlock(0);
    if ( liquid == nullptr ) return;

    if ( world->Build(liquid, x_self, y_self, z_self) ) {
        vessel_inv->Pull(0);
    }
}

void DeferredAction::SetFire() const {
    if ( world->GetBlock(x_self, y_self, z_self)->Sub() == AIR ) {
        world->Build(BlockManager::NewBlock(GRASS, FIRE),
            x_self, y_self, z_self);
    }
}

void DeferredAction::SetGhostMove(const ushort dir) {
    type = DEFERRED_GHOST_MOVE;
    num  = dir;
}

void DeferredAction::SetMove(const ushort dir) {
    type = DEFERRED_MOVE;
    num  = dir;
}

void DeferredAction::SetJump() { type = DEFERRED_JUMP; }

void DeferredAction::SetBuild(const short x, const short y, const short z,
        Block * const mat, const ushort builder_slot)
{
    SetXyz(x, y, z);
    material = mat;
    srcSlot  = builder_slot;
    type     = DEFERRED_BUILD;
}

void DeferredAction::SetDamage(const short x, const short y, const short z) {
    SetXyz(x, y, z);
    type = DEFERRED_DAMAGE;
}

void DeferredAction::SetThrow(const short x, const short y, const short z,
        const ushort src, const ushort dest, const ushort n)
{
    SetXyz(x, y, z);
    srcSlot  = src;
    destSlot = dest;
    num  = n;
    type = DEFERRED_THROW;
}

void DeferredAction::SetPour(const short x, const short y, const short z,
        const ushort src)
{
    SetXyz(x, y, z);
    srcSlot = src;
    type = DEFERRED_POUR;
}

void DeferredAction::SetSetFire(const short x, short y, short z) {
    SetXyz(x, y, z);
    type = DEFERRED_SET_FIRE;
}

void DeferredAction::MakeAction() {
    switch ( type ) {
    case DEFERRED_NOTHING: break;
    case DEFERRED_MOVE:   Move();   break;
    case DEFERRED_JUMP:   Jump();   break;
    case DEFERRED_BUILD:  Build();  break;
    case DEFERRED_THROW:  Throw();  break;
    case DEFERRED_POUR:   Pour();   break;
    case DEFERRED_DAMAGE: Damage(); break;
    case DEFERRED_SET_FIRE: SetFire(); break;
    case DEFERRED_GHOST_MOVE: GhostMove(); break;
    }
    type = DEFERRED_NOTHING;
}

World * DeferredAction::GetWorld() const { return world; }

DeferredAction::DeferredAction(Active * const attached) :
        Xyz(),
        type(DEFERRED_NOTHING),
        attachedBlock(attached),
        material(),
        srcSlot(),
        destSlot(),
        num()
{}
