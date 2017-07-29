#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <tsalgo/types.h>

#define MEGA  1024*1024
#define BLOCK 8192

int main () {
	char *fname = "rsc/sorted.bin";
	FILE *stream;
	char buf[BLOCK];
	int i,j;
	uint64_t k;

	stream = fopen(fname,"r");
	if (stream == NULL) {
		printf("cannot open file\n");
		return EXIT_FAILURE;
	}

	j=BLOCK;
	for (i=0;i<MEGA;i+=8) {
		if (j==BLOCK) {
			if (fread(buf,BLOCK,1,stream) != 1) {
				printf("cannot read file\n");
				fclose(stream);
				return EXIT_FAILURE;
			}
			j=0;
		}
		memcpy(&k,buf+j,8);
		printf("%09lu\n",k);j+=8;
	}
	fclose(stream);
	return EXIT_SUCCESS;
}
