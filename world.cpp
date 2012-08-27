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

#include <unistd.h>
#include "header.h"
#include "world.h"
	
void World::SaveAllShreds() {
	for (long i=longitude-1; i<=longitude+1; ++i)
	for (long j=latitude-1;  j<=latitude+1;  ++j)
		SaveShred(i, j, (j-latitude+1)*shred_width, (i-longitude+1)*shred_width);
}

void World::LoadAllShreds() {
	for (long i=longitude-1; i<=longitude+1; ++i)
	for (long j=latitude-1;  j<=latitude+1;  ++j)
		LoadShred(i, j, (j-latitude+1)*shred_width, (i-longitude+1)*shred_width);
	char str[50];
	FileName(str, longitude, latitude);
	FILE * check=fopen(str, "r");
	if (NULL==check) {
		blocks[playerX][playerY][playerZ]=(Block*)(playerP=new Dwarf(this, playerX, playerY, playerZ));
		//blocks[shred_width*2-5][shred_width*2-5][height/2]=(Block *)new Dwarf(this, shred_width*2-5, shred_width*2-5, height/2);
		//blocks[shred_width*2-4][shred_width*2-4][height/2]=new Telegraph;
	} else {
		fclose(check);
		if ( DWARF!=Kind(playerX, playerY, playerZ) ) {
			if (NULL!=blocks[playerX][playerY][playerZ])
				delete blocks[playerX][playerY][playerZ];
			blocks[playerX][playerY][playerZ]=(Block*)(playerP=new Dwarf(this, playerX, playerY, playerZ));
			fprintf(stderr, "World::LoadAllShreds(): new player place\n");
		} else
			playerP=(Dwarf *)blocks[playerX][playerY][playerZ];
	}
}

void World::LoadShred(long longi, long lati, const unsigned short istart, const unsigned short jstart) {
	char str[50];
	FileName(str, longi, lati);
	FILE * in=fopen(str, "r");
	if (NULL==in) {
		unsigned short i, j, k;
		for (i=istart; i<istart+shred_width; ++i)
		for (j=jstart; j<jstart+shred_width; ++j) {
			blocks[i][j][0]=new Block(NULLSTONE);
			for (k=1; k<height/2-2; ++k)
				blocks[i][j][k]=new  Block(STONE);//Liquid(this, i, j, k, WATER);
			blocks[i][j][k++]=new Block(SOIL);
			blocks[i][j][k++]=new Grass(this, i, j, height/2);
			for ( ; k<height-1; ++k)
				blocks[i][j][k]=NULL;
		}
		/*for (i=istart+1; i<istart+7; ++i) {
			if (NULL==blocks[i][jstart+1][height/2-1])
				delete blocks[i][jstart][height/2-1];
			blocks[i][jstart+1][height/2-1]=new Block(STONE);
		}*/
		//long pit
		/*for (i=istart+1; i<istart+7; ++i) {
			delete blocks[i][jstart+2][height/2-1];
			blocks[i][jstart+2][height/2-1]=NULL;
		}*/
		//deep pit
		/*for (k=height/2; k>0; --k) {
			delete blocks[istart+4][jstart+3][k];
			blocks[istart+4][jstart+3][k]=NULL;
		}*/
		delete blocks[istart][jstart][height/2-1];
		blocks[istart][jstart][height/2-1]=new Bush(this, istart, jstart, height/2-1);

		for (i=istart; i<=istart+2; ++i)
		for (j=jstart+6; j<=jstart+8; ++j) {
			delete blocks[i][j][height/2-1];
			blocks[i][j][height/2-1]=NULL;
		}
		Tree(istart+1, jstart+6, height/2-1, 4);

		delete blocks[istart+1][jstart][height/2-1];
		blocks[istart+1][jstart][height/2-1]=new Block(ROSE);
	} else {
		for (unsigned short i=istart; i<istart+shred_width; ++i)
		for (unsigned short j=jstart; j<jstart+shred_width; ++j)
			for (unsigned short k=0; k<height-1; ++k)
				BlockFromFile(in, blocks[i][j][k], this, i, j, k);
		fclose(in);
	}
}

void World::SaveShred(long longi, long lati, unsigned short istart, unsigned short jstart) {
	char str[50];
	FileName(str, longi, lati);
	FILE * out=fopen(str, "w");
	for (unsigned short i=istart; i<istart+shred_width; ++i)
	for (unsigned short j=jstart; j<jstart+shred_width; ++j)
		for (unsigned short k=0; k<height-1; ++k) {
			if (NULL!=out)
				if (NULL!=blocks[i][j][k]) blocks[i][j][k]->SaveToFile(out);
				else fprintf(out, "-1\n");
			delete blocks[i][j][k];
		}
	if (NULL!=out) fclose(out);
}

void World::ReloadShreds(dirs direction) { //ReloadShreds is called from Move, so there is no need to use mutex in this function
	bool flagLeft=false, flagRight=false;
	if (scr->blockToPrintLeft==playerP)
		flagLeft=true;
	if (scr->blockToPrintRight==playerP)
		flagRight=true;
	SaveAllShreds();
	switch (direction) {
		case NORTH: --longitude; break;
		case SOUTH: ++longitude; break;
		case EAST:  ++latitude;  break;
		case WEST:  --latitude;  break;
		default: fprintf(stderr, "World::ReloadShreds(dirs): invalid direction.\n");
	}
	LoadAllShreds();
	if (flagLeft) scr->blockToPrintLeft=playerP;
	if (flagRight) scr->blockToPrintRight=playerP;
}

void World::PhysEvents() {
	pthread_mutex_lock(&mutex);

	//sun/moon moving, time increment
	static bool if_star=false;
	unsigned short i=(TimeOfDay()<end_of_night) ?
		TimeOfDay()*(float)shred_width*3/end_of_night :
		(TimeOfDay()-end_of_night)*(float)shred_width*3/(seconds_in_day-end_of_night);
	delete blocks[i][int(shred_width*1.5)][height-1];
	blocks[i][int(shred_width*1.5)][height-1]=new Block( if_star ? STAR : SKY );
	++time;
	i=(TimeOfDay()<end_of_night) ?
		TimeOfDay()*(float)shred_width*3/end_of_night :
		(TimeOfDay()-end_of_night)*(float)shred_width*3/(seconds_in_day-end_of_night);
	if_star=( STAR==blocks[i][int(shred_width*1.5)][height-1]->Sub() ) ? true : false;
	delete blocks[i][int(shred_width*1.5)][height-1];
	blocks[i][int(shred_width*1.5)][height-1]=new Block(SUN_MOON);

	//sounds and blocks' own activities, falling
	for (i=0; i<9; ++i) {
		soundMap[i].ch=' ';
		soundMap[i].lev=0;
		soundMap[i].col=BLACK_BLACK;
	}
	Active * nexttemp;
	for ( Active * temp=activeList; NULL!=temp; temp=nexttemp ) {
		temp->Act();
		unsigned short x, y, z, n;
		temp->GetSelfXYZ(x, y, z);
		switch ( MakeDir(playerX, playerY, x, y) ) {
			case HERE:       n=4; break;
			case NORTH:      n=1; break;
			case NORTH_EAST: n=2; break;
			case EAST:       n=5; break;
			case SOUTH_EAST: n=8; break;
			case SOUTH:      n=7; break;
			case SOUTH_WEST: n=6; break;
			case WEST:       n=3; break;
			case NORTH_WEST: n=0; break;
		}
		if (playerX==x && playerY==y && playerZ==z) {
			soundMap[n].ch=' ';
			soundMap[n].lev=playerP->Noise();
			soundMap[n].col=(NULL!=scr) ?
				scr->Color( Kind(x, y, z), Sub(x, y, z) ) :
				BLACK_WHITE;
		}
		soundMap[n].ch=(' '==soundMap[n].ch) ? temp->MakeSound() : '*';
		if (' '!=soundMap[n].ch) {
			short temp=shred_width-Distance(playerX, playerY, playerZ, x, y, z);
			if (temp<0) temp=0;
			soundMap[n].lev+=(temp*10)/shred_width;
			if (soundMap[n].lev>9) soundMap[n].lev=9;
			if (soundMap[n].lev>0) {
				soundMap[n].col=(NULL!=scr) ?
					scr->Color( Kind(x, y, z), Sub(x, y, z) ) :
					BLACK_WHITE;
				soundMap[n].lev+=1;
			}
		}
		nexttemp=temp->GetNext();
		if ( temp->IfToDestroy() ) {
			if (NULL!=scr) {
				if (NULL!=scr->blockToPrintRight &&
						scr->blockToPrintRight->GetThis()==blocks[x][y][z])
					scr->blockToPrintRight=NULL;
				if (NULL!=scr->blockToPrintLeft &&
						scr->blockToPrintLeft->GetThis()==blocks[x][y][z])
					scr->blockToPrintLeft=NULL;
			}
			delete blocks[x][y][z];
			blocks[x][y][z]=NULL;
		} else if ( temp->ShouldFall() )
			Move(x, y, z, DOWN);
	}
	
	pthread_mutex_unlock(&mutex);
	if (NULL!=scr) {
		scr->Print();
		scr->PrintSounds();
	}
}

char World::CharNumber(int i, int j, int k) {
	if (height-1==k) return ' ';
	if (i==playerX && j==playerY && k==playerZ)
		switch ( playerP->GetDir() ) {
			case NORTH: return '^';
			case SOUTH: return 'v';
			case EAST:  return '>';
			case WEST:  return '<';
			case DOWN:  return 'x';
			case UP:    return '.';
		}	
	if ( UP==GetPlayerDir() ) {
		if (k > playerZ && k < playerZ+10) return k-playerZ+'0';
	} else {
		if (k==playerZ) return ' ';
		if (k>playerZ-10) return playerZ-k+'0';
	}
	return '+';
}

char World::CharNumberFront(int i, int j) {
	unsigned short ret;
	if ( NORTH==playerP->GetDir() || SOUTH==playerP->GetDir() ) {
		if ( (ret=abs(playerY-j))<10 ) return ret+'0';
	} else
		if ( (ret=abs(playerX-i))<10 ) return ret+'0';
	return '+';
}

bool World::DirectlyVisible(int x_from, int y_from, int z_from,
                            int x_to,   int y_to,   int z_to) {
	if (x_from==x_to && y_from==y_to && z_from==z_to) return true;
	unsigned short max=(abs(z_to-z_from) > abs(y_to-y_from)) ? abs(z_to-z_from) : abs(y_to-y_from);
	if (abs(x_to-x_from) > max) max=abs(x_to-x_from);
	float x_step=(float)(x_to-x_from)/max,
	      y_step=(float)(y_to-y_from)/max,
	      z_step=(float)(z_to-z_from)/max;
	for (unsigned short i=1; i<max; ++i)
		if ( !Transparent(nearbyint(x_from+i*x_step),
		                  nearbyint(y_from+i*y_step),
		                  nearbyint(z_from+i*z_step)))
		   	return false;
	return true;
}

bool World::Visible(int x_from, int y_from, int z_from,
                    int x_to,   int y_to,   int z_to) {
	short temp;
	if ((DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to)) ||
		(Transparent(x_to+(temp=(x_to>x_from) ? (-1) : 1), y_to, z_to) && DirectlyVisible(x_from, y_from, z_from, x_to+temp, y_to, z_to)) ||
		(Transparent(x_to, y_to+(temp=(y_to>y_from) ? (-1) : 1), z_to) && DirectlyVisible(x_from, y_from, z_from, x_to, y_to+temp, z_to)) ||
		(Transparent(x_to, y_to, z_to+(temp=(z_to>z_from) ? (-1) : 1)) && DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to+temp)))
			return true;
	return false;
}

int World::Move(int i, int j, int k, dirs dir, unsigned stop) {
	pthread_mutex_lock(&mutex);
	int newi, newj, newk;
	if ( NULL==blocks[i][j][k] ||
			NOT_MOVABLE==Movable(blocks[i][j][k]) ||
			Focus(i, j, k, newi, newj, newk, dir) ) {
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	if ( DESTROY==(blocks[i][j][k]->BeforeMove(dir)) ) {
		delete blocks[i][j][k];
		blocks[i][j][k]=NULL;
		pthread_mutex_unlock(&mutex);
		return 1;
	}
	int numberMoves=0;
	if (stop && (ENVIRONMENT!=Movable(blocks[i][j][k]) || !Equal(blocks[i][j][k], blocks[newi][newj][newk])) &&
			(ENVIRONMENT==Movable(blocks[newi][newj][newk]) || (numberMoves=Move(newi, newj, newk, dir, stop-1)) )) {
		blocks[i][j][k]->Move(dir);
		if (NULL!=blocks[newi][newj][newk])
			blocks[newi][newj][newk]->Move( Anti(dir) );

		Block * temp=blocks[i][j][k];
		blocks[i][j][k]=blocks[newi][newj][newk];
		blocks[newi][newj][newk]=temp;

		float weight=Weight(blocks[newi][newj][newk])-Weight(blocks[newi][newj][newk-1]);
		if (stop && weight)
			Move(newi, newj, newk, (weight>0) ? DOWN : UP, stop-1);

		if (blocks[newi][newj][newk]==(Block*)playerP) {
			playerX=newi;
			playerY=newj;
			playerZ=newk;
			if (playerX==shred_width-1) {
				playerX+=shred_width;
				ReloadShreds(WEST);
			} else if (playerX==shred_width*2) {
				playerX-=shred_width;
				ReloadShreds(EAST);
			} else if (playerY==shred_width-1) {
				playerY+=shred_width;
				ReloadShreds(NORTH);
			} else if (playerY==shred_width*2) {
				playerY-=shred_width;
				ReloadShreds(SOUTH);
			}
		}
		pthread_mutex_unlock(&mutex);
		return numberMoves+1;
	}
	pthread_mutex_unlock(&mutex);
	return 0;
}

void World::Jump(int i, int j, int k) { if ( NULL!=blocks[i][j][k] &&
		blocks[i][j][k]->Movable() ) { blocks[i][j][k]->SetWeight(0);
	if ( Move(i, j, k, UP) ) { ++k; dirs dir; if (Move( i, j, k,
				dir=blocks[i][j][k]->GetDir() )); switch (dir)
	{ case NORTH: --j; break; case SOUTH: ++j; break; case EAST:  ++i;
		break; case WEST:  --i; break; case UP:    ++k; break; case
			DOWN:  --k; break; } } blocks[i][j][k]->SetWeight();
	Move(i, j, k, DOWN); }
}

int World::Focus(int i, int j, int k, int & i_target, int & j_target, int & k_target, dirs dir) {
	i_target=i;
	j_target=j;
	k_target=k;
	switch (dir) {
		case NORTH: --j_target; break;
		case SOUTH: ++j_target; break;
		case EAST:  ++i_target; break;
		case WEST:  --i_target; break;
		case DOWN:  --k_target; break;
		case UP:    ++k_target; break;
	}

	bool bound_flag=false;
	if (i_target<0) {
		i_target=0;
		bound_flag=true;
	} else if (i_target>=shred_width*3) {
		i_target=shred_width*3-1;
		bound_flag=true;
	}
	if (j_target<0) {
		j_target=0;
		bound_flag=true;
	} else if (j_target>=shred_width*3) {
		j_target=shred_width*3-1;
		bound_flag=true;
	}
	if (k_target<0) {
		k_target=0;
		bound_flag=true;
	} else if (k_target>=height) {
		k_target=height-1;
		bound_flag=true;
	}
	return (bound_flag) ? 1 : 0;
}

World::World() : scr(NULL), activeList(NULL) {
	FILE * file=fopen("save", "r");
	if (file==NULL) {
		longitude=0;
		latitude=0;
		playerX=shred_width*2-7;
		playerY=shred_width*2-7;
		playerZ=height/2;
		time=end_of_night+5;
	} else {
		fscanf(file, "longitude: %ld\nlatitude: %ld\nplayerX: %hd\n playerY: %hd\n playerZ: %hd\ntime: %ld\n",
				&longitude, &latitude, &playerX, &playerY, &playerZ, &time);
		fclose(file);
	}
	LoadAllShreds();
	MakeSky();
	for (unsigned short i=0; i<9; ++i) {
		soundMap[i].ch=' ';
		soundMap[i].lev=0;
		soundMap[i].col=WHITE_BLACK;
	}
	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &mutex_attr);
	pthread_create(&eventsThread, NULL, PhysThread, this);
}

World::~World() {
	pthread_mutex_lock(&mutex);
	pthread_cancel(eventsThread);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	FILE * file=fopen("save", "w");
	if (file!=NULL) {
		fprintf(file, "longitude: %ld\nlatitude: %ld\nplayerX: %hd\nplayerY: %hd\nplayerZ: %hd\ntime: %ld\n",
				longitude, latitude, playerX, playerY, playerZ, time);
		fclose(file);
	}
	SaveAllShreds();
}

void *PhysThread(void *vptr_args) {
	while (1) {
		((World*)vptr_args)->PhysEvents();
		sleep(1);
	}
}
