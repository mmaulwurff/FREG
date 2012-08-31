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
	//if some kind will not be listed here, blocks of this kind just will not load.
	//unless kind is inherited from Inventory class or one of its derivatives - in this case this may cause something bad.
	switch(kind) {
		case BLOCK:     block=new Block(str); break;
		case TELEGRAPH: block=new Telegraph(str); break;
		case PICK:      block=new Pick(str); break;
		case DWARF:     block=new Dwarf(world, i, j, k, str, in); break;
		case CHEST:     block=new Chest(str, in); break;
		case PILE:      block=new Pile(world, i, j, k, str, in); break;
		case LIQUID:    block=new Liquid(world, i, j, k, str, in); break;
		case GRASS:     block=new Grass(world, i, j, k, str, in); break;
		case BUSH:      block=new Bush(world, i, j, k, str, in); break;
		case RABBIT:    block=new Rabbit(world, i, j, k, str); break;
		case -1:        block=NULL; break;
		default:
			fprintf(stderr, "BlockFromFile(FILE *, Block * &, World *, unsigned short, unsigned short, unsigned short): unlisted kind\n");
			block=NULL;
	}
}

void Active::SafeMove() {
	//when player is on the border of shred, moving him causes World::ReloadShreds(dir). so, cheks are neede for liquids not to push player.
	int x_focus, y_focus, z_focus;
	whereWorld->Focus(x_self, y_self, z_self, x_focus, y_focus, z_focus, direction);
	if (x_focus!=whereWorld->playerX || y_focus!=whereWorld->playerY)
		whereWorld->Move(x_self, y_self, z_self, direction);
}
void Active::SafeJump() {
	SetWeight(0);
	if ( whereWorld->Move(x_self, y_self, z_self, UP) ) {
		SetWeight();
		SafeMove();
	} else
		SetWeight();
}

void Active::Register(World * w) {
	whereWorld=w;
	if (NULL!=whereWorld) {
		prev=NULL;
		if (NULL==whereWorld->activeList)
			next=NULL;
		else {
			next=whereWorld->activeList;
			whereWorld->activeList->prev=this;
		}
		whereWorld->activeList=this;
	} else {
		next=NULL;
		prev=NULL;
	}
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
		case 0: SetDir(NORTH); break;
		case 1:	SetDir(EAST);  break;
		case 2: SetDir(SOUTH); break;
		case 3:	SetDir(WEST);  break;
	}
	SafeMove();
}

void Grass::Act() {
	if ( !(random()%seconds_in_hour) ) {
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

void Rabbit::Act() {
	if (random()%6) {
		short for_north=0, for_west=0;

		unsigned short const x_start=(x_self-7>0) ? x_self-7 : 0;
		unsigned short const y_start=(y_self-7>0) ? y_self-7 : 0;
		unsigned short const z_start=(z_self-1>0) ? z_self-1 : 0;
		unsigned short const x_end=(x_self+7<shred_width*3) ? x_self+7 : shred_width*3-1;
		unsigned short const y_end=(y_self+7<shred_width*3) ? y_self+7 : shred_width*3-1;
		unsigned short const z_end=(z_self+1<height-1) ? z_self+1 : height-2;

		for (unsigned short x=x_start; x<=x_end; ++x)
		for (unsigned short y=y_start; y<=y_end; ++y)
		for (unsigned short z=z_start; z<=z_end; ++z)
			switch ( whereWorld->Kind(x, y, z) ) {
				case DWARF:
					for_north+=y-y_self;
					for_west+=x-x_self;
					//fprintf(stderr, "Rabbit noticed dwarf at %hd, %hd, %hd\n", x, y, z);
				break;
				case RABBIT:
					for_north-=y-y_self;
					for_west-=x-x_self;
				break;
			}

		if ( abs(for_north)>abs(for_west) ) {
			if (for_north>0) SetDir(NORTH);
			else SetDir(SOUTH);
		} else {
			if (for_west>0) SetDir(WEST);
			else SetDir(EAST);
		}
	} else {
		switch (random()%4) {
			case 0: SetDir(NORTH); break;
			case 1: SetDir(SOUTH); break;
			case 2: SetDir(EAST);  break;
			case 3: SetDir(WEST);  break;
		}
	}
	(random()%2) ? SafeMove() : SafeJump();
}
