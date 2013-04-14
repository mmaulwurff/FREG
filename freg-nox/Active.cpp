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

#include "blocks.h"
#include "Shred.h"
#include "world.h"
#include <QDataStream>

QString & Active::FullName(QString & str) const {
	switch (sub) {
		case SAND: return str="Sand";
		default:
			fprintf(stderr,
				"Active:FullName(QString&): Unlisted sub: %d\n",
				sub);
			return str="Unkown active block";
	}
}

int Active::Kind() const { return ACTIVE; }

Active * Active::ActiveBlock() { return this; }

void Active::SetNotFalling() { falling=false; }
bool Active::IsFalling() const { return falling; }

ushort Active::X() const { return x_self; }
ushort Active::Y() const { return y_self; }
ushort Active::Z() const { return z_self; }

int Active::Move(const int dir) {
	switch ( dir ) {
		case NORTH: --y_self; break;
		case SOUTH: ++y_self; break;
		case EAST:  ++x_self; break;
		case WEST:  --x_self; break;
		case UP:    ++z_self; break;
		case DOWN:  --z_self; break;
		default:
			fprintf(stderr,
				"Active::Move: unlisted dir: %d\n",
				dir);
			return 0;
	}
	if ( DOWN==dir ) {
		falling=true;
		++fall_height;
	} else {
		if ( GetWorld()->GetShred(x_self, y_self)!=whereShred ) {
			whereShred->RemActive(this);
			whereShred=GetWorld()->GetShred(x_self, y_self);
			whereShred->AddActive(this);
		}
		fall_height=0;
	}
	emit Moved(dir);
	return 0;
}

void Active::Act() {
	if ( !falling && fall_height ) {
		if ( fall_height > safe_fall_height ) {
			const ushort dmg=(fall_height - safe_fall_height)*10;
			fall_height=0;
			GetWorld()->Damage(x_self, y_self, z_self-1, dmg);
			if ( GetWorld()->Damage(x_self, y_self, z_self, dmg) ) {
				return;
			} else {
				emit Updated();
			}
		} else {
			fall_height=0;
		}
	}
}

World * Active::GetWorld() const { return whereShred->GetWorld(); }

int Active::Movable() const { return MOVABLE; }

bool Active::ShouldFall() const { return true; }

before_move_return Active::BeforeMove(const int) { return NOTHING; }

int Active::Damage(const ushort dmg, const int dmg_kind) {
	const int last_dur=durability;
	const int new_dur=Block::Damage(dmg, dmg_kind);
	if ( last_dur != new_dur )
		emit Updated();
	return new_dur;
}

void Active::ReloadToNorth() { y_self+=SHRED_WIDTH; }
void Active::ReloadToSouth() { y_self-=SHRED_WIDTH; }
void Active::ReloadToWest()  { x_self+=SHRED_WIDTH; }
void Active::ReloadToEast()  { x_self-=SHRED_WIDTH; }

void Active::SaveAttributes(QDataStream & out) const {
	out << timeStep << fall_height << falling;
}

void Active::Register(
		Shred * const sh,
		const ushort x,
		const ushort y,
		const ushort z)
{
	if ( !whereShred ) {
		whereShred=sh;
		x_self=x;
		y_self=y;
		z_self=z;
		whereShred->AddActive(this);
	}
}

void Active::Unregister() {
	if ( whereShred ) {
		whereShred->RemActive(this);
		whereShred=0;
	}
}

Active::Active(
		const int sub,
		const quint8 transp)
		:
		Block(sub, transp),
		fall_height(0),
		falling(false),
		timeStep(0),
		whereShred(0)
{}

Active::Active(
		QDataStream & str,
		const int sub,
		const quint8 transp) //see default in blocks.h
		:
		Block(str, sub, transp),
		whereShred(0)
{
	str >> timeStep >> fall_height >> falling;
}

Active::~Active() {
       	Unregister();
	emit Destroyed();
}

