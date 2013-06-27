	/*
	*This file is part of FREG.
	*
	*FREG is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*FREG is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#include "world.h"
#include "blocks.h"
#include "DeferredAction.h"

void DeferredAction::Move() const {
	world->Move(
		attachedBlock->X(),
		attachedBlock->Y(),
		attachedBlock->Z(),
		attachedBlock->GetDir());
}

void DeferredAction::Jump() const {
	world->Jump(
		attachedBlock->X(),
		attachedBlock->Y(),
		attachedBlock->Z(),
		attachedBlock->GetDir());
}

void DeferredAction::Build() {
	if ( DOWN==attachedBlock->GetDir() &&
			AIR!=world->Sub(xTarg, yTarg, zTarg) )
	{
		if ( world->Move(
				attachedBlock->X(),
				attachedBlock->Y(),
				attachedBlock->Z(),
				UP) )
		{
			++zTarg;
		} else {
			return;
		}
	}
	const int kind=material->Kind();
	const int sub =material->Sub();
	if ( !world->Build(material,
			xTarg, yTarg, zTarg,
			World::TurnRight(attachedBlock->GetDir()),
			attachedBlock) )
	{ // build not successful
		return;
	}
	Inventory * const inv=attachedBlock->HasInventory();
	if ( !inv ) {
		return;
	}
	inv->Pull(srcSlot);
	//put more material in building
	//inventory slot:
	if ( inv->Number(srcSlot) ) {
		return;
	}
	for (ushort i=srcSlot+1; i<inv->Size() &&
		inv->Number(srcSlot)<MAX_STACK_SIZE; ++i)
	{
		const Block * const block_i=inv->ShowBlock(i);
		if ( block_i &&
				kind==block_i->Kind() &&
				sub ==block_i->Sub() )
		{
			inv->MoveInside(i, srcSlot, inv->Number(i));
		}
	}
}

void DeferredAction::Damage() const {
	world->Damage(xTarg, yTarg, zTarg,
		attachedBlock->DamageLevel(),
		attachedBlock->DamageKind());
}

void DeferredAction::Throw() const {
	world->Drop(
		attachedBlock->X(),
		attachedBlock->Y(),
		attachedBlock->Z(),
		srcSlot, destSlot, num);
}

void DeferredAction::SetMove() {
	type=DEFERRED_MOVE;
	world->AddDeferredAction(this);
}
void DeferredAction::SetJump() {
	type=DEFERRED_JUMP;
	world->AddDeferredAction(this);
}
void DeferredAction::SetBuild(
		const ushort x_targ, const ushort y_targ, const ushort z_targ,
		Block * const mat,
		const ushort builder_slot)
{
	xTarg=x_targ;
	yTarg=y_targ;
	zTarg=z_targ;
	material=mat;
	srcSlot=builder_slot;
	type=DEFERRED_BUILD;
	world->AddDeferredAction(this);
}
void DeferredAction::SetDamage(
		const ushort x_targ,
		const ushort y_targ,
		const ushort z_targ)
{
	xTarg=x_targ;
	yTarg=y_targ;
	zTarg=z_targ;
	type=DEFERRED_DAMAGE;
	world->AddDeferredAction(this);
}
void DeferredAction::SetThrow(const ushort src, const ushort dest,
		const ushort n)
{
	srcSlot=src;
	destSlot=dest;
	num=n;
	type=DEFERRED_THROW;
	world->AddDeferredAction(this);
}

void DeferredAction::MakeAction() {
	switch ( type ) {
		case DEFERRED_MOVE:   Move();   break;
		case DEFERRED_JUMP:   Jump();   break;
		case DEFERRED_BUILD:  Build();  break;
		case DEFERRED_DAMAGE: Damage(); break;
		case DEFERRED_THROW:  Throw();  break;
	}
	type=DEFERRED_NOTHING;
}

World * DeferredAction::GetWorld() const { return world; }

DeferredAction::DeferredAction(Active * const attached, World * const w) :
		type(DEFERRED_NOTHING),
		attachedBlock(attached),
		xTarg(),
		yTarg(),
		zTarg(),
		material(),
		srcSlot(),
		destSlot(),
		num(),
		world(w)
{}
