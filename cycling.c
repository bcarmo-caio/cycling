#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void *cyclist(void *arg) {
	return NULL;
}

int main(int argc, char **argv) {
	int d, n, i, debug = 0;
	char c;
	pthread_t *threads;

	d = atoi(argv[1]);
	n = atoi(argv[2]);
	c = argv[3][0];
	if (argc == 5 && strcmp(argv[4], "-d") == 0) debug = 1; 

	threads = malloc(n*sizeof(pthread_t));
	for (i = 0; i < n; i++)
		pthread_create(&threads[i], NULL, &cyclist, NULL);

	return 0;
}
