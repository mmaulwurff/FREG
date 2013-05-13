#include <stdio.h>
#include <math.h>

typedef unsigned short ushort;
const float pi=3.141592;

int deg(const ushort x, const ushort y, const ushort size) {
	float fi;
	float x_cent=x-size/2;
	float y_cent=y-size/2;
	if ( x_cent>0 && y_cent>=0 ) {
		fi=atan(y_cent/x_cent);
	} else if ( x_cent>0 && y_cent<0 ) {
		fi=atan(y_cent/x_cent)+2*pi;
	} else if ( x_cent<0 ) {
		fi=atan(y_cent/x_cent)+pi;
	} else if ( x_cent==0 && y_cent>0 ) {
		fi=pi/2;
	} else if ( x_cent==0 && y_cent<0 ) {
		fi=3*pi/2;
	} else {
		fi=0;
	}
	return 360*fi/2/pi;
}

int r(const ushort x, const ushort y, const ushort size) {
	return sqrt((x-size/2)*(x-size/2)+(y-size/2)*(y-size/2));
}

void circle(
		const ushort min_rad, const ushort max_rad,
	       	const char ch,
		const ushort size,
		char map[size][size])
{
	float maxs[360]={rand()%max_rad};
	if ( maxs[0]<min_rad ) {
		maxs[0]=min_rad;
	}
	float rad_change;
	ushort x, y;
	for (x=1; x<360; ++x) {
		rad_change=(rand()%400-200.0)/200.0;
		maxs[x]=maxs[x-1]+rad_change;
		if ( maxs[x] > max_rad ) {
			maxs[x]=max_rad;
			rad_change-=0.01;
		} else if ( maxs[x] < min_rad ) {
			maxs[x]=min_rad;
			rad_change+=0.01;
		}
		if ( x>315 ) {
			maxs[x]+=(maxs[0]-maxs[x-1])/90;
		}
	}
	for (y=0; y<size; ++y)
	for (x=0; x<size; ++x) {
		if ( r(x, y, size) < maxs[deg(x, y, size)] ) {
			map[x][y]=ch;
		}
	}
}

int main() {
	const ushort size=50;
	const char outer='~';
	const unsigned int seed=276;
	srand(seed);
	FILE * const file=fopen("map.txt", "w");
	ushort x, y;
	const ushort num_vertex=8;
	
	const ushort min_rad=size/6;
	const ushort max_rad=size/2-1;
	
	char map[size][size];
	for (y=0; y<size; ++y)
	for (x=0; x<size; ++x) {
		map[x][y]=outer;
	}

	circle(min_rad, max_rad, '.', size, map);
	circle(min_rad/2, max_rad/2, '%', size, map);
	circle(min_rad/4, max_rad/4, '^', size, map);

	//rivers
	/*ushort i;
	for (i=0; i<360; ++i) {
		if ( !(rand()%30) ) {
			ushort r;
			for (r=max_rad/3; r<max_rad; ++r) {
				map
					[(int)(r*cos(i*2*pi/360))+size/2]
					[(int)(r*sin(i*2*pi/360))+size/2]='~';
			}
		}
	}*/
		
	for (y=0; y<size; ++y, fputc('\n', file))
	for (x=0; x<size; ++x) {
		fputc(map[x][y], file);
		fputc(map[x][y], file);
	}

	fclose(file);
}
