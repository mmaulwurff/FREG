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

Block * Clock::DropAfterDamage() const { return block_manager.NewBlock(CLOCK, Sub()); }

usage_types Clock::Use() {
	//TODO: restore clock work
	/*world->EmitNotify(QString("Time is %1%2%3.").
		arg(world->TimeOfDay()/60).
		arg((world->TimeOfDay()%60 < 10) ? ":0" : ":").
		arg(world->TimeOfDay()%60));*/
	return NO;
}

int Clock::Kind() const { return CLOCK; }

QString & Clock::FullName(QString & str) const {
	switch ( sub ) {
		case IRON: return str="Iron clock";
		default:
			fprintf(stderr,
				"Clock::FullName: unlisted sub: %d\n",
				sub);
			return str="Strange clock";
	}
}

int Clock::BeforePush(const int) {
	Use();
	return NO_ACTION;
}

float Clock::TrueWeight() const { return 0.1f; }

Clock::Clock(const int sub) :
		Block(sub, NONSTANDARD)
{}

Clock::Clock (QDataStream & str, const int sub) :
		Block(str, sub, NONSTANDARD)
{}
