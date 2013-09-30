	/* freg, Free-Roaming Elementary Game with open and interactive world
	*  Copyright (C) 2012-2013 Alexander 'mmaulwurff' Kromm
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

#include <QDataStream>
#include "Active.h"
#include "Shred.h"
#include "world.h"
#include "DeferredAction.h"
#include "Xyz.h"

QString Active::FullName() const {
	switch ( Sub() ) {
	case SAND:  return tr("Sand");
	case WATER: return tr("Snow");
	default:
		fprintf(stderr, "Active::FullName: Unlisted sub: %d\n", Sub());
		return "Unkown active block";
	}
}

quint8 Active::Kind() const { return ACTIVE; }
Active * Active::ActiveBlock() { return this; }
int  Active::ShouldAct() const { return NEVER; }
bool Active::IsFalling() const { return falling; }
bool Active::ShouldFall() const { return true; }
int  Active::PushResult(const int) const { return MOVABLE; }
void Active::DoRareAction() {}
void Active::DoFrequentAction() {}
void Active::ActFrequent() {
	if ( !IsToDelete() ) {
		if ( GetDeferredAction() ) {
			GetDeferredAction()->MakeAction();
		}
		DoFrequentAction();
	}
}
void Active::ActRare() {
	if ( !IsToDelete() ) {
		DoRareAction();
	}
}

void Active::SetFalling(const bool set) {
	if ( !(falling=set) ) {
		fall_height = 0;
	}
}

void Active::SetDeferredAction(DeferredAction * const action) {
	delete deferredAction;
	deferredAction = action;
}
DeferredAction * Active::GetDeferredAction() const { return deferredAction; }

void Active::FallDamage() {
	if ( fall_height > SAFE_FALL_HEIGHT ) {
		World * const world=GetWorld();
		const ushort dmg=(fall_height - SAFE_FALL_HEIGHT)*10;
		world->Damage(X(), Y(), Z()-1, dmg, DAMAGE_FALL);
		world->Damage(X(), Y(), Z(), dmg, DAMAGE_FALL);
	}
	fall_height=0;
}

ushort Active::X() const { return x_self; }
ushort Active::Y() const { return y_self; }
ushort Active::Z() const { return z_self; }

bool Active::Move(const int dir) {
	switch ( dir ) {
	case NORTH: --y_self; break;
	case SOUTH: ++y_self; break;
	case EAST:  ++x_self; break;
	case WEST:  --x_self; break;
	case UP:    ++z_self; break;
	}
	bool overstep;
	if ( DOWN == dir ) {
		--z_self;
		++fall_height;
		overstep = false;
	} else {
		Shred * const new_shred = GetWorld()->GetShred(X(), Y());
		if ( (overstep = ( shred != new_shred )) ) {
			falling = false;
			new_shred->Register(this);
		}
	}
	emit Moved(dir);
	return overstep;
}

void Active::SendSignalAround(const QString & signal) const {
	World * const world = GetWorld();
	const Xy coords[] = {
		Xy( X()-1, Y()   ),
		Xy( X()+1, Y()   ),
		Xy( X(),   Y()-1 ),
		Xy( X(),   Y()+1 )
	};
	for (ushort i=0; i<sizeof(coords)/sizeof(Xy); ++i) {
		if ( world->InBounds(coords[i].GetX(), coords[i].GetY()) ) {
			world->GetBlock( coords[i].GetX(), coords[i].GetY(),
				Z() )->ReceiveSignal(signal);
		}
	}
	world->GetBlock(X(), Y(), Z()-1)->ReceiveSignal(signal);
	world->GetBlock(X(), Y(), Z()+1)->ReceiveSignal(signal);
}

void Active::SetShred(Shred * const new_shred) { shred = new_shred; }
Shred * Active::GetShred() const { return shred; }
World * Active::GetWorld() const { return world; }

void Active::Damage(const ushort dmg, const int dmg_kind) {
	const int last_dur = durability;
	Block::Damage(dmg, dmg_kind);
	if ( last_dur != durability ) {
		ReceiveSignal(OUCH);
		switch ( dmg_kind ) {
		case HUNGER:
			ReceiveSignal(tr("You faint from hunger!"));
		break;
		case HEAT:
			ReceiveSignal(tr("You burn!"));
		break;
		case BREATH:
			ReceiveSignal(tr("You choke withot air!"));
		break;
		case DAMAGE_FALL:
			ReceiveSignal(tr("You fall, damage %1.").
				arg(last_dur-durability));
		break;
		default:
			ReceiveSignal(tr("Received %1 damage!").
				arg(last_dur-durability));
		}
		emit Updated();
	}
}

void Active::ReceiveSignal(const QString & str) { emit ReceivedText(str); }

void Active::ReloadToNorth() { y_self += SHRED_WIDTH; }
void Active::ReloadToSouth() { y_self -= SHRED_WIDTH; }
void Active::ReloadToWest()  { x_self += SHRED_WIDTH; }
void Active::ReloadToEast()  { x_self -= SHRED_WIDTH; }

void Active::EmitUpdated() { emit Updated(); }

void Active::SaveAttributes(QDataStream & out) const { out << fall_height; }

void Active::SetXYZ(const ushort x, const ushort y, const ushort z) {
	x_self = x;
	y_self = y;
	z_self = z;
}

void Active::SetToDelete() {
	if ( !frozen ) {
		frozen = true;
		GetShred()->AddToDelete(this);
		ReceiveSignal(tr("You die."));
		emit Destroyed();
	}
}
bool Active::IsToDelete() const { return frozen; }

Active::Active(const int sub, const quint16 id, const quint8 transp) :
		Block(sub, id, transp),
		fall_height(0),
		falling(false),
		frozen(false),
		deferredAction(0),
		x_self(), y_self(), z_self(),
		shred()
{}
Active::Active(QDataStream & str, const int sub, const quint16 id,
		const quint8 transp)
	:
		Block(str, sub, id, transp),
		falling(false),
		frozen(false),
		deferredAction(0),
		x_self(), y_self(), z_self(),
		shred()
{
	str >> fall_height;
}
Active::~Active() { delete deferredAction; }

bool Active::Gravitate(const ushort range, const ushort down, const ushort up,
		const ushort calmness)
{
	World * const world = GetWorld();
	// analyse world around
	short for_north = 0, for_west = 0;
	for (ushort x=X()-range; x<=X()+range; ++x)
	for (ushort y=Y()-range; y<=Y()+range; ++y) {
		if ( !world->InBounds(x, y) ) {
			continue;
		}
		Shred * const shred = world->GetShred(x, y);
		const ushort x_in = Shred::CoordInShred(x);
		const ushort y_in = Shred::CoordInShred(y);
		for (ushort z=Z()-down;  z<=Z()+up; ++z) {
			short attractive;
			if (World::InVertBounds(z) &&
					( attractive = Attractive(shred->
						GetBlock(x_in, y_in, z)->
							Sub()) ) &&
					world->DirectlyVisible(X(), Y(), Z(),
						x, y, z))
			{
				if ( y!=Y() ) for_north += attractive/(Y()-y);
				if ( x!=X() ) for_west  += attractive/(X()-x);
			}
	}
	}
	// make direction and move there
	if ( qAbs(for_north)>calmness || qAbs(for_west)>calmness ) {
		SetDir( ( qAbs(for_north)>qAbs(for_west) ) ?
			( ( for_north>0 ) ? NORTH : SOUTH ) :
			( ( for_west >0 ) ? WEST  : EAST  ) );
		return true;
	} else {
		return false;
	}
}

short Active::Attractive(const int) const { return 0; }

bool Active::IsSubAround(const quint8 sub) const {
	const World * const world = GetWorld();
	return (
			sub == world->GetBlock(X(), Y(), Z()-1)->Sub() ||
			sub == world->GetBlock(X(), Y(), Z()+1)->Sub() ||
			(world->InBounds(X()-1, Y()) &&
			sub == world->GetBlock(X()-1, Y(), Z())->Sub()) ||
			(world->InBounds(X()+1, Y()) &&
			sub==world->GetBlock(X()+1, Y(), Z())->Sub()) ||
			(world->InBounds(X(), Y()-1) &&
			sub==world->GetBlock(X(), Y()-1, Z())->Sub()) ||
			(world->InBounds(X(), Y()+1) &&
			sub==world->GetBlock(X(), Y()+1, Z())->Sub()) );
}
