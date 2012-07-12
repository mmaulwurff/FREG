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
