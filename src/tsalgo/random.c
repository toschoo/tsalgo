#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <tsalgo/random.h>

void init_rand() {
	srand(time(NULL) ^ (intptr_t)&printf);
}

double randomProbability() {
	return (double)rand()/(double)RAND_MAX;
}

unsigned int randomUnsigned(int min, int max) {
	float u = (float)randomProbability();
	return (max-min)*u+min;
}
