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
	if ( min_rad >= max_rad ) {
		printf("%c: min_rad (%hu) >= max_rad (%hu)\n",
			ch, min_rad, max_rad);
	}
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

int main(int argc, char * argv[]) {
	ushort size=75;
	char outer='~';
	srand(0);
	char filename[50]="map.txt";
	ushort i;
	for (i=1; i<argc; ++i) {
		if ( '-'==argv[i][0] ) {
			switch ( argv[i][1] ) {
				case 's': size=atoi(argv[++i]); break;
				case 'o': outer=argv[++i][0]; break;
				case 'r': srand(atoi(argv[++i])); break;
				case 'f': {
					ushort n=0;
					while ( filename[n]=argv[++i][n++] );
				} break;
				default:
					fputs("No such parameter.\n", stderr);
			}
		}
	}
	if ( size<10 ) {
		size=10;
	}

	char map[size][size];
	ushort x, y;
	for (y=0; y<size; ++y)
	for (x=0; x<size; ++x) {
		map[x][y]=outer;
	}

	const ushort min_rad=size/3;
	const ushort max_rad=size/2;
	circle(min_rad, max_rad, '.', size, map);
	circle(min_rad/2, max_rad/2, '%', size, map);
	circle(min_rad/3, max_rad/3+1, '+', size, map);
	circle(min_rad/4, max_rad/4+1, '^', size, map);

	//rivers
	const ushort river_width=4;
	const ushort river_width_deg=river_width*80/size;
	for (i=river_width_deg; i<360-river_width_deg; ++i) {
		if ( !(rand()%(60*80/size)) ) {
			ushort j;
			for (j=i-river_width_deg; j<=i+river_width_deg; ++j) {
				ushort r;
				for (r=max_rad/3; r<max_rad; ++r) {
					map[(int)(r*cos(j*2*pi/360))+size/2]
						[(int)(r*sin(j*2*pi/360))+
							size/2]='~';
				}
			}
		}
	}
		
	FILE * const file=fopen(filename, "w");
	for (y=0; y<size; ++y, fputc('\n', file))
	for (x=0; x<size; ++x) {
		fputc(map[x][y], file);
	}

	fclose(file);
}
