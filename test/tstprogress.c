#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <progress.h>

int main() {
	int i,z;
	progress_t p;

	for (i=21;i<200;i++) {
		printf("%d\n", i);
		init_progress(&p, stdout, i);
		for(z=0;z<i;z++) update_progress(&p,z);
		close_progress(&p);
		printf("\n");
	}
}
