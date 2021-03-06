#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <signal.h>

#include "utils.h"
#include "cycling.h"
#include "cyclist.h"


sem_t create_thread; /* Must be initialized with 0 */
sem_t go; /* Must be initialized with 0 */

sem_t lock_cyclists_set; /* Must be initialized with 1 */
int cyclists_set = 0;

int initial_number_of_cyclists;
sem_t lock_current_number_of_cyclists;
int current_number_of_cyclists;

/* Circuit with 4 tracks */
int **runway;
sem_t *tracks; /* Each one must be initialized with 1 */
sem_t all_runway; /* Must be initialized with 1 */
int runway_length;

int variable_speed;

pthread_barrier_t bar;
sem_t all_cyclists_set_up; /* Must be initialized with 0 */
sem_t xavaska; /* Must be initialized with 0 */


int main(int argc, char **argv) {
	int i, j, c;
	int current_lap = 1;
	int completed_current_lap;
	int someone_eliminated;
	int last[3] = {-1, -1, -1};
	int *final_position = NULL;
	int next_breaking_attempt = 4;
	int debug_flag = 0;
	int debug_time = 0;
	int *start = NULL;
	/*pthread_t *threads = NULL;*/
	struct thread_info *tinfo = NULL;
	char errmsg[200];
	int errno_cpy;
	int tmp;
	struct timespec ts;
	int already_tried = 0;

	/*** Initialization ***/
	if(argc < 4) {
		sprintf(errmsg, "%s d n [v|u] ['d']\n"
				"\td := distance\n"
				"\tn := # cyclists\n"
				"\tv := variable speed\n"
				"\tu := constant speed\n"
				"\t'd' (optional) := enable debug mode", argv[0]);
		handle_error(errmsg);
	}
	runway_length = atoi(argv[1]);
	initial_number_of_cyclists = atoi(argv[2]);
	current_number_of_cyclists = initial_number_of_cyclists;

	switch(argv[3][0]) {
		case 'v':
			variable_speed = 1;
			break;
		case 'u':
			variable_speed = 0;
			break;
		default:
			handle_error("Speed is not 'u' nor 'v'");
			break;
	}
	if (argc == 5 && (strcmp(argv[4], "-d")) == 0) debug_flag = 1;

	srand(time(NULL));

	/* Creating circuit with 4 tracks and length 'runway_length' */
	runway = malloc(runway_length * sizeof(int *));
	for (i = 0; i < runway_length; i++) runway[i] = malloc(4*sizeof(int));
	if(!runway) handle_error("runway = malloc");
	for (i = 0; i < runway_length; i++) runway[i][0] = runway[i][1] = runway[i][2] = runway[i][3] = 0;

	/* Creating array to store the final positions of every cyclist */
	final_position = malloc(runway_length * sizeof(int));
	memset((void *) final_position, -1, runway_length * sizeof(int));


	start = (int *) malloc(initial_number_of_cyclists * sizeof(int));
	if(start == NULL)
		handle_error("start = malloc");

	/* Shuffle start */
	for(i = 0; i < initial_number_of_cyclists; i++)
		start[i] = i + 1;
	/* http://en.wikipedia.org/wiki/Fisher-Yates_shuffle */
	for(i = initial_number_of_cyclists - 1; i >= 1; i--) {
		j = rand() % (i + 1);
		tmp = start[i];
		start[i] = start[j];
		start[j] = tmp;
	}
	for(i = 0; i < initial_number_of_cyclists; i++)
		runway[i][0] = start[i];
	memset(start, 0, initial_number_of_cyclists * sizeof(int));
	free(start);
	start = NULL;
	/* Shuffled */

	tinfo = malloc(initial_number_of_cyclists * sizeof(struct thread_info));
	if(tinfo == NULL)
		handle_error("malloc tinfo");
	memset(tinfo, 0, initial_number_of_cyclists * sizeof(struct thread_info));

	for(i = 0; i < initial_number_of_cyclists; i++) {
		tinfo[i].thread_num = i + 1;
		tinfo[i].cyclist_id = runway[i][0];
	}

	init_semaphores();

	/* Creating threads */
	for (i = 0; i < initial_number_of_cyclists; i++) {
		errno_cpy = pthread_create( &(tinfo[i].thread_id), /*pthread_t *thread*/
									NULL, /* const pthread_attr_t *attr */
									&cyclist, /* void *(*routine) (void *) */
									(void *) &(tinfo[i])); /*void *arg*/
		if (errno_cpy != 0)	{
			sprintf(errmsg, "pthread_create %d for cyclist %d", i, start[i]);
			handle_error_en(errno_cpy, (const char *) errmsg);
		}
		Sem_wait(&create_thread, &ts, MAIN_THREAD_ID);
	}
	/* NO CODE HERE!!!!! */

	/*** Start simulation ***/
	Sem_wait(&all_cyclists_set_up, &ts, MAIN_THREAD_ID);
	/*printf("Race started!\n");*/

	/*  MAIN LOOP  */
	printf("Race started!\n");
	while (1) {
#ifdef DEBUG
		printf("\nIteration %d:\n", i);
		printf("Printing runway\n");
		print_runway();
		printf("GO!\n");
#endif
		/*** Run threads, one iteration ***/
		/*3*/
		/*2*/
		/*1*/
		/*GOOOOO!*/
		/*printf("current_number_of_cyclists %d\n", current_number_of_cyclists);*/
		Pthread_barrier_init(&bar, 0, current_number_of_cyclists);
		for(i = 0; i < current_number_of_cyclists; i++) Sem_post(&go, MAIN_THREAD_ID);
		/* NO CODE HERE!!!!! */
		Sem_wait(&all_cyclists_set_up, &ts, MAIN_THREAD_ID);
		Pthread_barrier_destroy(&bar);

		/*** Elimination ***/
		someone_eliminated = 0;
		completed_current_lap = 0;
		for (c = 0; c < initial_number_of_cyclists; c++)
			if ((tinfo[c].status == RUNNING) && (tinfo[c].completed_laps >= current_lap))
				completed_current_lap++;
		if (completed_current_lap == current_number_of_cyclists) {
			/* Find last cyclist */
			for (c = 0; c < initial_number_of_cyclists; c++) {
				if (tinfo[c].status == RUNNING) {
					last[0] = c; /* First running cyclist */
					break;
				}
			}
			for (c = 0; c < initial_number_of_cyclists; c++) {
				if (tinfo[c].status == RUNNING) {
					if (tinfo[c].completed_laps < tinfo[last[0]].completed_laps)
						last[0] = c;
					else if ((tinfo[c].completed_laps == tinfo[last[0]].completed_laps) &&
							 (tinfo[c].position_runway > tinfo[last[0]].position_runway))
						last[0] = c;
				}
			}
			/* Find second last cyclist */
			if (current_number_of_cyclists >= 2) {
				for (c = 0; c < initial_number_of_cyclists; c++) {
					if ((c != last[0]) && (tinfo[c].status == RUNNING)) {
						last[1] = c;
						break;
					}
				}
				for (c = 0; c < initial_number_of_cyclists; c++) {
					if ((c != last[0]) && (tinfo[c].status == RUNNING)) {
						if (tinfo[c].completed_laps < tinfo[last[1]].completed_laps)
							last[1] = c;
						else if ((tinfo[c].completed_laps == tinfo[last[1]].completed_laps) &&
								 (tinfo[c].position_runway > tinfo[last[1]].position_runway))
							last[1] = c;
					}
				}
			}
			/* Find third last cyclist */
			if (current_number_of_cyclists >= 3) {
				for (c = 0; c < initial_number_of_cyclists; c++) {
					if ((c != last[0]) && (c != last[1] && tinfo[c].status == RUNNING)) {
						last[2] = c;
						break;
					}
				}
				for (c = 0; c < initial_number_of_cyclists; c++) {
					if ((c != last[0]) && (c != last[1]) && (tinfo[c].status == RUNNING)) {
						if (tinfo[c].completed_laps < tinfo[last[2]].completed_laps)
							last[2] = c;
						else if ((tinfo[c].completed_laps == tinfo[last[2]].completed_laps) &&
								 (tinfo[c].position_runway > tinfo[last[2]].position_runway))
							last[2] = c;
					}
				}
			}
			/* Eliminate */
			/*printf("eliminando\n");*/
			someone_eliminated = 1;
			tinfo[last[0]].status = ELIMINATED;
			runway[tinfo[last[0]].position_runway][tinfo[last[0]].position_track] = 0;
			errno_cpy = pthread_cancel(tinfo[last[0]].thread_id);
			if(errno_cpy != 0) handle_error_en(errno_cpy, "cancel thread (eliminated)");
			errno_cpy = pthread_join(tinfo[last[0]].thread_id, NULL);
			if(errno_cpy != 0) handle_error_en(errno_cpy, "join thread (eliminated)");
			final_position[current_number_of_cyclists-1] = tinfo[last[0]].cyclist_id;
			if (current_number_of_cyclists >= 2) {
				printf("\nCyclist %d eliminated!\n", tinfo[last[0]].cyclist_id);
				printf("The second last is cyclist %d\n", tinfo[last[1]].cyclist_id);
			}
			if (current_number_of_cyclists >= 3)
				printf("The third last is cyclist %d\n", tinfo[last[2]].cyclist_id);
			current_lap++;
			already_tried = 0;
		}

		/*** Breaking ***/
		if ( ((current_lap % 4) == 0) && !already_tried && (current_number_of_cyclists - someone_eliminated > 3)) {
			already_tried = 1;
			for (c = 0; c < initial_number_of_cyclists; c++)
				if (tinfo[c].completed_laps == next_breaking_attempt)
					break;
			if (c < initial_number_of_cyclists) {
				if (rand() % 100 == 42) /* any number 0 to 99 */ {
					/*printf("Someone will break down\n");*/
					c = rand() % initial_number_of_cyclists;
					while (1) { /* for sure this will stop */
						if (tinfo[c].status == RUNNING) {
							tinfo[c].status = BROKEN;
							if (final_position[current_number_of_cyclists-1] == -1)
								final_position[current_number_of_cyclists-1] = tinfo[c].cyclist_id;
							else
								final_position[current_number_of_cyclists-2] = tinfo[c].cyclist_id;

							runway[tinfo[c].position_runway][tinfo[c].position_track] = 0;
							errno_cpy = pthread_cancel(tinfo[c].thread_id);
							if(errno_cpy != 0) handle_error_en(errno_cpy, "cancel thread (broken)");
							errno_cpy = pthread_join(tinfo[c].thread_id, NULL);
							if(errno_cpy != 0) handle_error_en(errno_cpy, "join thread (broken)");

							someone_eliminated++;

							printf("Cyclist %d just broke. Lap %d!\n", tinfo[c].cyclist_id, current_lap);
							break;
						}
						c = (c + 1) % initial_number_of_cyclists;
					}
				}
			}
		}

		/*** Check if race ended ***/
		current_number_of_cyclists -= someone_eliminated;
		if (current_number_of_cyclists == 0) {
			break;
		}

		/*** Debug ***/
		if (debug_flag) {
			debug_time++;
			if (debug_time == 200 /*(14.400 / 72 = 200)*/) {
				debug_time = 0;
				printf("\nDebug:\n");
				printf("Cyclist | Lap | Position\n");
				for (c = 0; c < initial_number_of_cyclists; c++) {
					printf("%7d | %3d | ", tinfo[c].cyclist_id, tinfo[c].completed_laps);
					switch (tinfo[c].status) {
						case BROKEN:
							printf("Broken\n");
							break;
						case ELIMINATED:
							printf("Eliminated\n");
							break;
						case RUNNING:
							printf("%d\n", tinfo[c].position_runway);
							break;
						default:
							handle_error("Cyclist status unknown");
							break;
					}
				}
			}
		}
	}
	printf("\nRace ended!\n\n");
	printf("Final standings:\n");
	printf("%d: Cyclist %d   Gold medal!\n", 1, final_position[0]);
	printf("%d: Cyclist %d   Silver medal!\n", 2, final_position[1]);
	printf("%d: Cyclist %d   Bronze medal!\n", 3, final_position[2]);
	for (i = 3; i < initial_number_of_cyclists; i++)
		printf("%d: Cyclist %d\n", i + 1, final_position[i]);
	
	/*pthread_exit(NULL);*/
	return 0;
}
