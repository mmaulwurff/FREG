#ifndef WORLD_FUNC_H
#define WORLD_FUNC_H

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
	
void World::LoadShred(long longi, long lati, unsigned short istart, unsigned short jstart) {
	unsigned short i, j, k;
	for (i=istart; i<istart+shred_width; ++i)
	for (j=jstart; j<jstart+shred_width; ++j) {
		blocks[i][j][0]=new Block(NULLSTONE);
		for (k=1; k<height/2; ++k)
			blocks[i][j][k]=new Block(STONE);
		for ( ; k<height; ++k)
			blocks[i][j][k]=NULL;
	}
	for (i=istart+1; i<istart+7; ++i)
		blocks[i][jstart+1][height/2]=new Block(GLASS);
	for (i=istart+1; i<istart+7; ++i) {
		delete blocks[i][jstart+2][height/2-1];
		blocks[i][jstart+2][height/2-1]=NULL;
	}
	for (k=height/2; k>0; --k) {
		delete blocks [istart+3][jstart+3][k];
		blocks[istart+3][jstart+3][k]=NULL;
	}
}

void World::SaveShred(long longi, long lati, unsigned short istart, unsigned short jstart) {
	unsigned short i, j, k;
	for (i=istart; i<istart+shred_width; ++i)
	for (j=jstart; j<jstart+shred_width; ++j)
		for (k=0; k<height; ++k)
			delete blocks[i][j][k];
}

void World::ReloadShreds(dirs direction) { //ReloadShreds is called from Move, so there is no need to use mutex in this function
	dirs save_dir=GetPlayerDir();
	long i, j;
	for (i=longitude-1; i<=longitude+1; ++i)
	for (j=latitude-1;  j<=latitude+1;  ++j)
		SaveShred(longitude, latitude, (i-longitude+1)*shred_width, (j-latitude+1)*shred_width);
	switch (direction) {
		case NORTH: --longitude; break;
		case SOUTH: ++longitude; break;
		case EAST:  ++latitude;  break;
		case WEST:  --latitude;  break;
	}
	for (i=longitude-1; i<=longitude+1; ++i)
	for (j=latitude-1;  j<=latitude+1;  ++j)
		LoadShred(longitude, latitude, (i-longitude+1)*shred_width, (j-latitude+1)*shred_width);
	MakeSky();
	blocks[playerX][playerY][playerZ]=(Block*)(playerP=new Dwarf(this, playerX, playerY, playerZ));
	SetPlayerDir(save_dir);
	blocks[shred_width*2-5][shred_width*2-5][height/2]=(Block *)new Dwarf(this, shred_width*2-5, shred_width*2-5, height/2);
	blocks[shred_width*2-4][shred_width*2-4][height/2]=new Telegraph;
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

	//sounds and blocks' own activities
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
			soundMap[n].col=scr->Color( Kind(x, y, z), Sub(x, y, z) );
		}
		soundMap[n].ch=(' '==soundMap[n].ch) ? temp->MakeSound() : '*';
		if (' '!=soundMap[n].ch) {
			short temp=shred_width-Distance(playerX, playerY, playerZ, x, y, z);
			if (temp<0) temp=0;
			soundMap[n].lev+=(temp*10)/shred_width;
			if (soundMap[n].lev>9) soundMap[n].lev=9;
			if (soundMap[n].lev>0) {
				soundMap[n].col=scr->Color( Kind(x, y, z), Sub(x, y, z) );
				soundMap[n].lev+=1;
			}
		}
		nexttemp=temp->GetNext();
		if ( temp->IfToDestroy() ) {
			unsigned short i, j, k;
			temp->GetSelfXYZ(i, j, k);
			if (NULL!=scr->invToPrintRight &&
					scr->invToPrintRight->GetThis()==blocks[i][j][k])
				scr->invToPrintRight=NULL;
			if (NULL!=scr->invToPrintLeft &&
					scr->invToPrintLeft->GetThis()==blocks[i][j][k])
				scr->invToPrintLeft=NULL;
			delete blocks[i][j][k];
			blocks[i][j][k]=NULL;
		}
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
		return playerZ-k+'0';
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
	unsigned short i;
	for (i=1; i<max; ++i)
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

int World::Move(int i, int j, int k, dirs dir) {
	pthread_mutex_lock(&mutex);
	if ( Movable(blocks[i][j][k])==NOT_MOVABLE ||
			(!i && dir==WEST ) ||
			(!j && dir==NORTH) ||
			(!k && dir==DOWN ) ||
			(i==shred_width*3-1 && dir==EAST ) ||
			(j==shred_width*3-1 && dir==SOUTH) ||
			(k==height-1 && dir==UP) ) {
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	int newi=i, newj=j, newk=k;
	switch (dir) {
		case NORTH: --newj; break;
		case SOUTH: ++newj; break;
		case EAST:  ++newi; break;
		case WEST:  --newi; break;
		case UP:    ++newk; break;
		case DOWN:  --newk; break;
	}
	int numberMoves=0;
	if (ENVIRONMENT==Movable(blocks[newi][newj][newk]) || (numberMoves=Move(newi, newj, newk, dir)) ) {
		blocks[i][j][k]->Move(dir);
		Block *temp=blocks[i][j][k];
		blocks[i][j][k]=blocks[newi][newj][newk];
		blocks[newi][newj][newk]=temp;
		int weight;
		if ( weight=Weight(blocks[i][j][k]) )
			if (weight>0) Move(i, j, k, DOWN);
			else        Move(i, j, k, UP);
		if ( weight=Weight(blocks[newi][newj][newk]) )
			if (weight>0) newk-=Move(newi, newj, newk, DOWN);
			else        newk+=Move(newi, newj, newk, UP);
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

void World::Jump(int i, int j, int k) {
	if ( NULL!=blocks[i][j][k] && blocks[i][j][k]->Movable() ) {
		blocks[i][j][k]->SetWeight(0);
		if ( Move(i, j, k, UP) ) {
			++k;
			dirs dir;
			if (Move( i, j, k, dir=blocks[i][j][k]->GetDir() ));
				switch (dir) {
					case NORTH: --j; break;
					case SOUTH: ++j; break;
					case EAST:  ++i; break;
					case WEST:  --i; break;
					case UP:    ++k; break;
					case DOWN:  --k; break;
				}
		}
		blocks[i][j][k]->SetWeight();
		Move(i, j, k, DOWN);
	}
}

void World::Focus(int i, int j, int k, int & i_target, int & j_target, int & k_target) {
	i_target=i;
	j_target=j;
	k_target=k;
	switch ( blocks[i][j][k]->GetDir() ) {
		case NORTH: --j_target; break;
		case SOUTH: ++j_target; break;
		case EAST:  ++i_target; break;
		case WEST:  --i_target; break;
		case DOWN:  --k_target; break;
		case UP:    ++k_target; break;
	}
	if (i_target<0) i_target=0;
	else if (i_target>=shred_width*3) i_target=shred_width*3-1;
	if (j_target<0) j_target=0;
	else if (j_target>=shred_width*3) j_target=shred_width*3-1;
	if (k_target<0) k_target=0;
	else if (k_target>=height) k_target=height-1;
}

World::World() {
	//TODO: add load and save
	FILE * file=fopen("save", "r");
	if (file==NULL) {
		longitude=0;
		latitude=0;
		playerX=shred_width*2-7;
		playerY=shred_width*2-7;
		playerZ=height/2;
	} else {
		fscanf(file, "longitude: %ld\nlatitude: %ld\nplayerX: %hd\n playerY: %hd\n playerZ: %hd\n",
				&longitude, &latitude, &playerX, &playerY, &playerZ);
		fclose(file);
	}
	activeList=NULL;
	long i, j;
	for (i=longitude-1; i<=longitude+1; ++i)
	for (j=latitude-1;  j<=latitude+1;  ++j)
		LoadShred(longitude, latitude, (i-longitude+1)*shred_width, (j-latitude+1)*shred_width);
	MakeSky();
	blocks[playerX][playerY][playerZ] =(Block*)( playerP=new Dwarf(this, playerX, playerY, playerZ) );
	blocks[shred_width*2-5][shred_width*2-5][height/2]=(Block*)new Dwarf(this, shred_width*2-5, shred_width*2-5, height/2);
	time=end_of_night-5;
	for (i=0; i<9; ++i) {
		soundMap[i].ch=' ';
		soundMap[i].lev=0;
		soundMap[i].col=WHITE_BLACK;
	}
	scr=NULL;
	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &mutex_attr);
	pthread_create(&eventsThread, NULL, PhysThread, this);
}

World::~World() {
	pthread_cancel(eventsThread);
	pthread_mutex_destroy(&mutex);
	FILE * file=fopen("save", "w");
	if ("file!=NULL") {
		fprintf(file, "longitude: %ld\nlatitude: %ld\nplayerX: %hd\nplayerY: %hd\nplayerZ: %hd\n",
				longitude, latitude, playerX, playerY, playerZ);
		fclose(file);
	}
	long i, j;
	for (i=longitude-1; i<=longitude+1; ++i)
	for (j=latitude-1;  j<=latitude+1;  ++j)
		SaveShred(longitude, latitude, (i-longitude+1)*shred_width, (j-latitude+1)*shred_width);
}

void *PhysThread(void *vptr_args) {
	while (1) {
		((World*)vptr_args)->PhysEvents();
		sleep(1);
	}
}

#endif
