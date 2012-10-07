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

main() {
	FILE * frand=fopen("/dev/urandom", "r");
	char str[300];
	unsigned short sum=0, i;
	for (i=0; sum<900; ++i) {
		sum+=(str[i]=fgetc(frand)%9+1);
		str[i]+='0';
		putchar(str[i]);
	}
	putchar('\n');
	fclose(frand);
}
