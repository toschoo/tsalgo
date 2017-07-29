#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <tsalgo/random.h>

#define ELEMENTS 1000000

unsigned buf[ELEMENTS];
int counts[10];

int cmp(const void *left, const void *right) {
	return *(int*)left - *(int*)right;
}

typedef struct {
	int    min;
	int    max;
	int    sum;
	double avg;
	double std;
	double med;
} stats_t;

void desc(stats_t *s) {
	int i;

	s->sum = 0;
	s->min = ELEMENTS;
	s->max = 0;

	/* sum, average, min and max */
	for(i=0;i<10;i++) {
		s->sum += counts[i];
		if (counts[i] > s->max) s->max = counts[i];
		if (counts[i] < s->min) s->min = counts[i];
	}

	/* average */
	s->avg = s->sum/10;

	double sm = 0;
	for(i=0;i<10;i++) {
		sm += (counts[i] - s->avg)*(counts[i] - s->avg);
	}
	sm /= 9;
	s->std = sqrt(sm);

	/* median */
	qsort(counts, 10, sizeof(int), cmp);
	s->med = (counts[5] + counts[6]) / 2;

}

int main() {
	int i = 0;
	stats_t s;

	init_rand();

	/* random numbers */
	for(i=ELEMENTS-1;i>=0;i--) {
		buf[i] = randomUnsigned(0,10);
	}

	/* count */
	for(i=9;i>=0;i--) {
		counts[i] = 0;	
	}
	for(i=ELEMENTS-1;i>=0;i--) {
		counts[buf[i]]++;
	}
	for(i=0;i<10;i++) {
		fprintf(stdout, "%06d ", counts[i]);
	}
	fprintf(stdout, "\n");

	/* stats */
	desc(&s);
	fprintf(stdout, 
	"Sum: %d, Max: %d, Min: %d, Median: %.2f, Average: %.2f, Std: %.2f\n",
	        s.sum, s.max, s.min, s.med, s.avg, s.std);

	int p = ELEMENTS/100;
	int pupr = ELEMENTS/10 + p;
	int plow = ELEMENTS/10 - p;

	if (s.avg != ELEMENTS/10) return EXIT_FAILURE;
	if (s.sum != ELEMENTS) return EXIT_FAILURE;
	if (s.max > pupr) return EXIT_FAILURE;
	if (s.min < plow) return EXIT_FAILURE;
	if (s.std > p) return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
