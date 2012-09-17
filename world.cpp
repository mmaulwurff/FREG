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
		blocks[spawnX][spawnY][spawnZ]=(Block*)(playerP=new Dwarf(this, spawnX, spawnY, spawnZ));
		//blocks[shred_width*2-5][shred_width*2-5][height/2]=(Block *)new Dwarf(this, shred_width*2-5, shred_width*2-5, height/2);
		//blocks[shred_width*2-4][shred_width*2-4][height/2]=new Telegraph;
	} else {
		fclose(check);
		if ( DWARF!=Kind(spawnX, spawnY, spawnZ) ) {
			if (NULL!=blocks[spawnX][spawnY][spawnZ])
				delete blocks[spawnX][spawnY][spawnZ];
			blocks[spawnX][spawnY][spawnZ]=(Block*)(playerP=new Dwarf(this, spawnX, spawnY, spawnZ));
			fprintf(stderr, "World::LoadAllShreds(): new player place\n");
		} else
			playerP=(Dwarf *)blocks[spawnX][spawnY][spawnZ];
	}

	ReEnlightenAll();
}

void World::LoadShred(const long longi, const long lati, const unsigned short istart, const unsigned short jstart) {
	char str[50];
	FileName(str, longi, lati);
	FILE * in=fopen(str, "r");
	if (NULL==in) {
		unsigned short i, j;
		for (i=istart; i<istart+shred_width; ++i)
		for (j=jstart; j<jstart+shred_width; ++j)
			blocks[i][j][0]=new Block(NULLSTONE);

		unsigned short k;
		for (i=istart; i<istart+shred_width; ++i)
		for (j=jstart; j<jstart+shred_width; ++j)
		for (k=1; k<height-1; ++k)
			blocks[i][j][k]=NULL;
				
		switch ( TypeOfShred(longi, lati) ) {
			case '#': NullMountain(istart, jstart); break;
			case 't': Forest(istart, jstart, longi, lati); break;
			case '~': Water(istart, jstart, longi, lati); break;
			case '+': Hill(istart, jstart, longi, lati); break;
			case '.': Plain(istart, jstart); break;
			default:
				Plain(istart, jstart);
				fprintf(stderr, "World::LoadShred: unknown type of shred: %c", TypeOfShred(longi, lati));
		}
	} else {
		for (unsigned short i=istart; i<istart+shred_width; ++i)
		for (unsigned short j=jstart; j<jstart+shred_width; ++j)
			for (unsigned short k=0; k<height-1; ++k)
				BlockFromFile(in, blocks[i][j][k], this, i, j, k);
		fclose(in);
	}
}

void World::SaveShred(const long longi, const long lati, const unsigned short istart, const unsigned short jstart) {
	char str[50];
	FileName(str, longi, lati);
	FILE * out=fopen(str, "w");

	if (NULL!=out) {
		for (unsigned short i=istart; i<istart+shred_width; ++i)
		for (unsigned short j=jstart; j<jstart+shred_width; ++j)
		for (unsigned short k=0; k<height-1; ++k)
			if (NULL!=blocks[i][j][k]) {
				blocks[i][j][k]->SaveToFile(out);
				delete blocks[i][j][k];
			} else fprintf(out, "-1\n");
		fclose(out);
	} else
		for (unsigned short i=istart; i<istart+shred_width; ++i)
		for (unsigned short j=jstart; j<jstart+shred_width; ++j)
		for (unsigned short k=0; k<height-1; ++k)
			if (NULL!=blocks[i][j][k])
				delete blocks[i][j][k];
}

void World::ReloadShreds(const dirs direction) { //ReloadShreds is called from Move, so there is no need to use mutex in this function
	long l;
	Active * block;
	short i, j, k;
	switch (direction) {
		case NORTH:
			for (l=latitude-1; l<=latitude+1; ++l)
				SaveShred(longitude+1, l, (l-latitude+1)*shred_width, 2*shred_width);
			
			for (i=0; i<shred_width*3; ++i)
			for (j=shred_width*2-1; j>=0; --j)
			for (k=0; k<height-1; ++k) {
				blocks[i][j+shred_width][k]=blocks[i][j][k];
				block=(Active *)( ActiveBlock(i, j, k) );
				if (NULL!=block)
					block->ReloadToNorth();
			}

			--longitude;
			for (l=latitude-1; l<=latitude+1; ++l)
				LoadShred(longitude-1, l, (l-latitude+1)*shred_width, 0);
		break;
		case SOUTH:
			for (l=latitude-1; l<=latitude+1; ++l)
				SaveShred(longitude-1, l, (l-latitude+1)*shred_width, 0);
 
			for (i=0; i<shred_width*3; ++i)
			for (j=shred_width; j<shred_width*3; ++j)
			for (k=0; k<height-1; ++k) {
				blocks[i][j-shred_width][k]=blocks[i][j][k];
				block=(Active *)( ActiveBlock(i, j, k) );
				if (NULL!=block)
					block->ReloadToSouth();
			}

			++longitude;
			for (l=latitude-1; l<=latitude+1; ++l)
				LoadShred(longitude+1, l, (l-latitude+1)*shred_width, shred_width*2);
		break;
		case EAST:
			for (l=longitude-1; l<=longitude+1; ++l)
				SaveShred(l, latitude-1, 0, (l-longitude+1)*shred_width);
 
			for (i=shred_width; i<shred_width*3; ++i)
			for (j=0; j<shred_width*3; ++j)
			for (k=0; k<height-1; ++k) {
				blocks[i-shred_width][j][k]=blocks[i][j][k];
				block=(Active *)( ActiveBlock(i, j, k) );
				if (NULL!=block)
					block->ReloadToEast();
			}

			++latitude;
			for (l=longitude-1; l<=longitude+1; ++l)
				LoadShred(l, latitude+1, shred_width*2, (l-longitude+1)*shred_width);
		break;
		case WEST:
			for (l=longitude-1; l<=longitude+1; ++l)
				SaveShred(l, latitude+1, shred_width*2, (l-longitude+1)*shred_width);

			for (i=shred_width*2-1; i>=0; --i)
			for (j=0; j<shred_width*3; ++j)
			for (k=0; k<height-1; ++k) {
				blocks[i+shred_width][j][k]=blocks[i][j][k];
				block=(Active *)( ActiveBlock(i, j, k) );
				if (NULL!=block)
					block->ReloadToWest();
			}

			--latitude;
			for (l=longitude-1; l<=longitude+1; ++l)
				LoadShred(l, latitude-1, 0, (l-longitude+1)*shred_width);
		break;
		default: fprintf(stderr, "World::ReloadShreds(dirs): invalid direction.\n");
	}

	ReEnlightenAll();
}

void World::PhysEvents() {
	pthread_mutex_lock(&mutex);

	//sun/moon moving, time increment
	++time_step;
	if ( !(time_step % time_steps_in_sec) ) {
		time_step=0;
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

		for (i=0; i<9; ++i) {
			soundMap[i].ch=' ';
			soundMap[i].lev=0;
			soundMap[i].col=BLACK_BLACK;
		}

		switch (time) {
			case end_of_evening:
			case end_of_night: ReEnlightenAll(); break;
		}
	}

	//blocks' own activities, falling
	Active * nexttemp;
	for ( Active * temp=activeList; NULL!=temp; temp=nexttemp ) {
		temp->Act();
		unsigned short x, y, z;
		temp->GetSelfXYZ(x, y, z);
		if (0==time_step) {//sounds
			unsigned short n;
			unsigned short playerX, playerY, playerZ;
			playerP->GetSelfXYZ(playerX, playerY, playerZ);
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
				default:
					n=4;
					fprintf(stderr, "World::PhysEvents(): unlisted dir\n");
			}
			if ((Block *)playerP==blocks[x][y][z]) {
				soundMap[n].ch=' ';
				soundMap[n].lev=playerP->Noise();
				soundMap[n].col=(NULL!=scr) ?
					scr->Color( Kind(x, y, z), Sub(x, y, z) ) :
					BLACK_WHITE;
			} else if ( ' '!=temp->MakeSound() ) {
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
			}
		}
		if ( temp->IfToDestroy() ) {
			nexttemp=temp->GetNext();
			if (NULL!=scr) {
				if (NULL!=scr->blockToPrintRight &&
						(Block *)(scr->blockToPrintRight)==blocks[x][y][z])
					scr->blockToPrintRight=NULL;
				if (NULL!=scr->blockToPrintLeft &&
						(Block *)(scr->blockToPrintLeft)==blocks[x][y][z])
					scr->blockToPrintLeft=NULL;
			}
			delete blocks[x][y][z];
			blocks[x][y][z]=NULL;
		} else if ( temp->ShouldFall() ) {
			Move(x, y, z, DOWN);
			nexttemp=temp->GetNext();
		} else
			nexttemp=temp->GetNext();
	}
	
	if (NULL!=scr) {
		scr->Print();
		if (0==time_step)
			scr->PrintSounds();
	}
	pthread_mutex_unlock(&mutex);
}

char World::CharNumber(const int i, const int j, const int k) const {
	if (height-1==k) return ' ';
	if ((Block *)playerP==blocks[i][j][k])
		switch ( playerP->GetDir() ) {
			case NORTH: return '^';
			case SOUTH: return 'v';
			case EAST:  return '>';
			case WEST:  return '<';
			case DOWN:  return 'x';
			case UP:    return '.';
			default:
				fprintf(stderr, "World::ChanNumber(int, int, int): unlisted dir: %d\n", (int)playerP->GetDir());
				return '*';
		}

	unsigned short playerX, playerY, playerZ;
	playerP->GetSelfXYZ(playerX, playerY, playerZ);
	if ( UP==GetPlayerDir() ) {
		if (k > playerZ && k < playerZ+10) return k-playerZ+'0';
	} else {
		if (k==playerZ) return ' ';
		if (k>playerZ-10) return playerZ-k+'0';
	}
	return '+';
}

char World::CharNumberFront(const int i, const int j) const {
	unsigned short ret;
	unsigned short playerX, playerY, playerZ;
	playerP->GetSelfXYZ(playerX, playerY, playerZ);
	if ( NORTH==playerP->GetDir() || SOUTH==playerP->GetDir() ) {
		if ( (ret=abs(playerY-j))<10 ) return ret+'0';
	} else
		if ( (ret=abs(playerX-i))<10 ) return ret+'0';
	return '+';
}

bool World::DirectlyVisible(const int x_from, const int y_from, const int z_from,
                            const int x_to,   const int y_to,   const int z_to) const {
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

bool World::Visible(const int x_from, const int y_from, const int z_from,
                    const int x_to,   const int y_to,   const int z_to) const {
	short temp;
	if ((DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to)) ||
		(Transparent(x_to+(temp=(x_to>x_from) ? (-1) : 1), y_to, z_to) && DirectlyVisible(x_from, y_from, z_from, x_to+temp, y_to, z_to)) ||
		(Transparent(x_to, y_to+(temp=(y_to>y_from) ? (-1) : 1), z_to) && DirectlyVisible(x_from, y_from, z_from, x_to, y_to+temp, z_to)) ||
		(Transparent(x_to, y_to, z_to+(temp=(z_to>z_from) ? (-1) : 1)) && DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to+temp)))
			return true;
	return false;
}

int World::Move(const unsigned short i, const unsigned short j, const unsigned short k, const dirs dir, const unsigned stop) {
	pthread_mutex_lock(&mutex);
	unsigned short newi, newj, newk;
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
	if (stop && (ENVIRONMENT!=blocks[i][j][k]->Movable() || !Equal(blocks[i][j][k], blocks[newi][newj][newk])) &&
			(ENVIRONMENT==Movable(blocks[newi][newj][newk]) || (numberMoves=Move(newi, newj, newk, dir, stop-1)) )) {
		Block * temp=blocks[i][j][k];
		blocks[i][j][k]=blocks[newi][newj][newk];
		blocks[newi][newj][newk]=temp;
		
		ReEnlighten(newi, newj, newk);

		if ( NULL!=blocks[i][j][k] )
			blocks[i][j][k]->Move( Anti(dir) );

		if ( blocks[newi][newj][newk]->Move(dir) )
			GetPlayerCoords(newi, newj, newk);

		float weight=Weight(blocks[newi][newj][newk])-Weight(blocks[newi][newj][newk-1]);
		if ( weight<0 ) {
			if ( ENVIRONMENT==blocks[newi][newj][newk]->Movable() )
				numberMoves+=Move(newi, newj, newk, UP, stop-1);
		} else if (weight)
			numberMoves+=Move(newi, newj, newk, DOWN, stop-1);

		++numberMoves;
	}
	pthread_mutex_unlock(&mutex);
	return numberMoves;
}

void World::Jump(const int i, const int j, int k) {
	pthread_mutex_lock(&mutex);
	if ( NULL!=blocks[i][j][k] && MOVABLE==blocks[i][j][k]->Movable() ) {
		Block * to_move=blocks[i][j][k];
		blocks[i][j][k]->SetWeight(0);
		int k_plus;
		if ( (k_plus=Move(i, j, k, UP)) ) {
			k+=k_plus;
			blocks[i][j][k]->SetWeight();
			if ( !Move( i, j, k, to_move->GetDir()) )
				Move(i, j, k, DOWN);
		} else
			blocks[i][j][k]->SetWeight();
	}
	pthread_mutex_unlock(&mutex);
}

int World::Focus(const unsigned short i, const unsigned short j, const unsigned short k,
		unsigned short & i_target, unsigned short & j_target, unsigned short & k_target, const dirs dir) const {
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
		default: fprintf(stderr, "World::Focus(int, int, int, int&, int&, int&, dirs): unlisted dir: %d\n", (int)dir);
	}
	return ( !InBounds(i, j, k) );
}

World::World() : time_step(0), activeList(NULL), scr(NULL) {
	FILE * file=fopen("save", "r");
	if (file==NULL) {
		longitude=3;
		latitude=3;
		spawnX=shred_width*2-7;
		spawnY=shred_width*2-7;
		spawnZ=height/2;
		time=end_of_night-5;
		strncpy(worldName, "The_Land_of_Doubts\0", 20);
		worldSize=1000;
	} else {
		fscanf(file, "longitude: %ld\nlatitude: %ld\nspawnX: %hd\n spawnY: %hd\n spawnZ: %hd\ntime: %ld\nWorld:%s\nSize:%hd\n",
				&longitude, &latitude, &spawnX, &spawnY, &spawnZ, &time, worldName, &worldSize);
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
	GetPlayerCoords(spawnX, spawnY, spawnZ);
	FILE * file=fopen("save", "w");
	if (file!=NULL) {
		fprintf(file, "longitude: %ld\nlatitude: %ld\nspawnX: %hd\nspawnY: %hd\nspawnZ: %hd\ntime: %ld\nWorld:%s\nSize:%hd\n",
				longitude, latitude, spawnX, spawnY, spawnZ, time, worldName, worldSize);
		fclose(file);
	}
	SaveAllShreds();

	for (unsigned short i=0; i<shred_width*3; ++i)
	for (unsigned short j=0; j<shred_width*3; ++j)
		delete blocks[i][j][height-1];
}

void *PhysThread(void *vptr_args) {
	while (1) {
		((World*)vptr_args)->PhysEvents();
		usleep(1000000/time_steps_in_sec);
	}
	return NULL;
}
