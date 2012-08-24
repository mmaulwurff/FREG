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
#include "world.h"

void BlockFromFile(FILE * in, Block * & block, World * world,
		unsigned short i, unsigned short j, unsigned short k) {
	char str[300];
	fgets(str, 300, in);
	int kind;
	sscanf(str, "%d", &kind);
	switch(kind) {
		case BLOCK:     block=new Block(str); break;
		case TELEGRAPH: block=new Telegraph(str); break;
		case PICK:      block=new Pick(str); break;
		case DWARF:     block=new Dwarf(world, i, j, k, str, in); break;
		case CHEST:     block=new Chest(str, in); break;
		case PILE:      block=new Pile(world, i, j, k, str, in); break;
		case LIQUID:    block=new Liquid(world, i, j, k, str, in); break;
		case GRASS:     block=new Grass(world, i, j, k, str, in); break;
		case -1:        block=NULL; break;
		default:
			fprintf(stderr, "BlockFromFile(FILE *, Block * &, World *, unsigned short, unsigned short, unsigned short): unlisted kind\n");
			block=NULL;
	}
}

void Active::Register(World * w) {
	whereWorld=w;
	prev=NULL;
	if (NULL==whereWorld->activeList)
		next=NULL;
	else {
		next=whereWorld->activeList;
		whereWorld->activeList->prev=this;
	}
	whereWorld->activeList=this;
}

void Active::Unregister() {
	if (NULL!=whereWorld) {
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
}

float Dwarf::LightRadius() {
	if ( this==whereWorld->GetPlayerP() ) return 1.8;
	else return 0;
}

before_move_return Dwarf::BeforeMove(dirs dir) {
	if (dir==direction)
		whereWorld->GetAll(x_self, y_self, z_self);
	return NOTHING;
}

before_move_return Pile::BeforeMove(dirs dir) {
	direction=dir;
	whereWorld->DropAll(x_self, y_self, z_self);
	return NOTHING;
}

void Liquid::Act() {
	switch (random()%4) {
		//when player is on the border of shred, moving him causes World::ReloadShreds(dir). so, cheks are neede for liquids not to push player.
		case 0:
			if (y_self!=shred_width+1)
				whereWorld->Move(x_self, y_self, z_self, NORTH);
			break;
		case 1:
			if (x_self!=shred_width*2-2)
				whereWorld->Move(x_self, y_self, z_self, EAST);
			break;
		case 2:
			if (y_self!=shred_width*2-2)
				whereWorld->Move(x_self, y_self, z_self, SOUTH);
			break;
		case 3:
			if (x_self!=shred_width+1)
				whereWorld->Move(x_self, y_self, z_self, WEST);
			break;
	}
}

void Grass::Act() {
	if (!(random()%10)) {
		unsigned short i_start, j_start, k_start,
			       i_end,   j_end,   k_end,
			       i, j, k;
		i_start=(x_self>0) ? x_self-1 : 0;
		j_start=(y_self>0) ? y_self-1 : 0;
		k_start=(z_self>0) ? z_self-1 : 0;
		i_end=(x_self<shred_width*3-1) ? x_self+1 : shred_width*3-1;
		j_end=(y_self<shred_width*3-1) ? y_self+1 : shred_width*3-1;
		k_end=(z_self<height-2)        ? z_self+1 : height-2;

		for (i=i_start; i<=i_end; ++i)
		for (j=j_start; j<=j_end; ++j)
		for (k=k_start; k<=k_end; ++k)
			if ( SOIL==whereWorld->Sub(i, j, k) && AIR==whereWorld->Sub(i, j, ++k) )
				whereWorld->blocks[i][j][k]=new Grass(whereWorld, i, j, k);
	}
}
