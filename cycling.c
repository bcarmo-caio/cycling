#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

struct runway_position {
	int free_positions;
	int position[4];
};

void *cyclist(void *arg) {
	return NULL;
}

int main(int argc, char **argv) {
	int d, n, i, debug_flag = 0;
	char c;
	struct runway_position *runway;
	pthread_t *threads;

	d = atoi(argv[1]);
	n = atoi(argv[2]);
	c = argv[3][0];
	if (argc == 5 && strcmp(argv[4], "-d") == 0) debug_flag = 1; 

	runway = malloc(d*sizeof(struct runway_position));
	threads = malloc(n*sizeof(pthread_t));
	for (i = 0; i < n; i++)
		pthread_create(&threads[i], NULL, &cyclist, NULL);

	while (run_not_over) {
		simulate_lap(lap_number);
	}

	return 0;
}

simulate_lap(lap_number) {
	if (debug_flag && lap_number % 200 == 0) {
		// debug
		debug();
	}
	if (lap_number % 4 == 0) {
		// breaking
		breaking();
		update_positioning_data();
	}
	if (lap_number % 2 == 0) {
		// elimination
		while (lap_not_over) {
			simulate_72ms();
			pthread_barrier_wait(72ms_barrier);
		}
	}
}

simulate_72ms() {
}

breaking() {
}

update_positioning_data() {
}

debug() {
}
