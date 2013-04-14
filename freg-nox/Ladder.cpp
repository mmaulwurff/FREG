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
#include "BlockManager.h"
#include <QDataStream>

Block * Ladder::DropAfterDamage() const { return block_manager.NewBlock(LADDER, Sub()); }

QString & Ladder::FullName(QString & str) const { return str="Ladder"; }

int Ladder::Kind() const { return LADDER; }

int Ladder::BeforePush(const int) { return MOVE_UP; }

float Ladder::TrueWeight() const { return 20; }

bool Ladder::Catchable() const { return true; }

Ladder::Ladder(const int sub) :
		Block(sub)
{}

Ladder::Ladder(
		QDataStream & str,
		const int sub)
		:
		Block(str, sub)
{}
