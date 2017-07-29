/* ========================================================================
 * (c) Tobias Schoofs, 2015 -- 2016
 * ========================================================================
 * Fisher-Yates Shuffle (a.k.a Knuth Shuffle)
 * ========================================================================
 * Creates a random permutation of the numbers 0...max-1.
 * Max is passed in as command line argument or, if no argument is given,
 * it is set to 10.
 *
 * The algorithm is defined in Knuth, TAOCP, Vol. 2, p.145.
 * ========================================================================
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <tsalgo/random.h>

/* ------------------------------------------------------------------------
 * Get the command line argument.
 * ------------------------------------------------------------------------
 */
int getMax(int argc, char **argv, unsigned int *n) {
	int i;
	if (argc < 2) {
		*n = 10; return 0;
	}
	size_t s = strnlen(argv[1], 8);
	if (s > 6) {
		fprintf(stderr, "number too big\n");
		return -1;
	}
	for(i=0;i<s;i++) {
		if (argv[1][i] < '0' ||
		    argv[1][i] > '9') {
			fprintf(stderr, "not numerical\n");
			return -1;
		}
	}
	*n = atoi(argv[1]);
	return 0;
}

/* ------------------------------------------------------------------------
 * Main
 * ------------------------------------------------------------------------
 */
int main(int argc, char **argv) {
	unsigned int i,k,t,n;
	int *buf = NULL;

	if (getMax(argc, argv, &n) != 0) return EXIT_FAILURE;

	buf = malloc(sizeof(int)*n);
	if (buf == NULL) {
		fprintf(stderr, "no memory!\n");
		return -1;
	}

	for(i=0;i<n;i++) buf[i] = i;

	init_rand();
	for(i=0;i<n;i++) {
		k = randomUnsigned(0,n);
		if (k != i) {
			t = buf[k];
			buf[k] = buf[i];
			buf[i] = t;
		}
	}
	for(i=0;i<n;i++) {
		if (i>0) fprintf(stdout, ",");
		fprintf(stdout, "%d", buf[i]);
	}
	free(buf);
	fprintf(stdout, "\n");
	return EXIT_SUCCESS;
}
