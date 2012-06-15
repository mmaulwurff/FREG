#include <stdio.h>

main() {
	int i, j;
	char  temp=0;
	FILE *sky= fopen( "sky.txt", "r"),
	     *sky2=fopen("sky2.txt", "w"),
	     *map= fopen( "map.txt", "w");
	while (EOF!=(temp=getc(sky))) if ('\n'!=temp) fputc(temp, sky2);
	fclose(sky2);
	sky2=fopen("sky2.txt", "r");
	for (i=0; i<192; ++i) {
		for (j=1; j<8;   ++j) {
			if (' '!=getc(sky2)) ++temp;
			(void)fgetc(sky);
			if (j!=7) temp<<=1;
		}
		fputc(temp, map);
	}
	fclose(sky);
	fclose(map);
	fclose(sky2);
}
