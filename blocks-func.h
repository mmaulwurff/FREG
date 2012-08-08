#ifndef BLOCKS_FUNC_H
#define BLOCKS_FUNC_H

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

Active::Active(World * w, unsigned short x, unsigned short y, unsigned short z) :
		x_self(x), y_self(y), z_self(z), whereWorld(w), prev(NULL) {
	if (NULL==whereWorld->activeList)
		next=NULL;
	else {
		next=whereWorld->activeList;
		whereWorld->activeList->prev=this;
	}
	whereWorld->activeList=this;
}

Active::~Active() {
	if (NULL!=next)
		next->prev=prev;
	if (NULL!=prev)
		prev->next=next;
	else {
		whereWorld->activeList=next;
		if (NULL!=whereWorld->activeList)
			whereWorld->activeList->prev=NULL;
	}
}

void Chest::Use() {
	if (INVENTORY!=GetWorld()->scr->viewLeft) {
		GetWorld()->scr->viewLeft=INVENTORY;
		GetWorld()->scr->invToPrintLeft=this;
	} else
		GetWorld()->scr->viewLeft=NORMAL;
}

#endif
