#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include "utils.h"
#include "cycling.h"
#include "cyclist.h"

sem_t create_thread; /* Must be initialized with 0 */
sem_t all_cyclists_set_up; /* Must be initialized with 0 */
sem_t go; /* Must be initialized with 0 */
sem_t end_simulation; /* Must be initialized with 0 */

sem_t lock_cyclists_set; /* Must be initialized with 1 */
int cyclists_set;

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

#ifdef DEBUG
sem_t simulation;
#endif

int main(int argc, char **argv) {
	int i, j, c;
	int current_lap = 1;
	int completed_current_lap;
	int last[3];
	int *final_position;
	int next_breaking_attempt = 4;
	int debug_flag = 0;
	int debug_time = 0;
	int *start = NULL;
	pthread_t *threads = NULL;
	struct thread_info *tinfo = NULL;
	char errmsg[200];
	int errno_cpy;
	int tmp;
#ifdef DEBUG
	int abort_on_start;
#endif

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

#ifdef DEBUG
	abort_on_start = 0;
	if (argc == 5 && (strcmp(argv[4], "-A")) == 0)
		abort_on_start = 1;
	else
		if (argc == 6 && (strcmp(argv[5], "-A")) == 0)
			abort_on_start = 1;
#endif

	/* Creating circuit with 4 tracks and length 'runway_length' */
	runway = malloc(runway_length * sizeof(int *));
	for (i = 0; i < runway_length; i++) runway[i] = malloc(4*sizeof(int));
	if(!runway) handle_error("runway = malloc");
	for (i = 0; i < runway_length; i++) runway[i][0] = runway[i][1] = runway[i][2] = runway[i][3] = 0;

	/* Creating array to store the final positions of every cyclist */
	final_position = malloc(runway_length * sizeof(int));
	memset((void *) final_position, 0, runway_length * sizeof(int));

	init_semaphores();

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

	/* Creating threads */
	for (i = 0; i < initial_number_of_cyclists; i++) {
		errno_cpy = pthread_create( &(tinfo[i].thread_id), /*pthread_t *thread*/
									NULL, /* const pthread_attr_t *attr */
									&cyclist, /* void *(*routine) (void *) */
									(void *) &(tinfo[i])); /*void *arg*/
		if (errno_cpy != 0)	{
			sprintf(errmsg, "pthread_cread %d for cyclist %d", i, start[i]);
			handle_error_en(errno_cpy, (const char *) errmsg);
		}
		Sem_wait(&create_thread);
	}
	/* NO CODE HERE!!!!! */

	/*** Start simulation ***/
	Sem_wait(&all_cyclists_set_up);
	/* while (1) { */ for(i = 0; i < 5; i++) {
		/*** Run threads, one iteration ***/
		/*3*/
		/*2*/
		/*1*/
		/*GOOOOO!*/
		Pthread_barrier_init(&bar, 0, current_number_of_cyclists);
		for(j = 0; j < current_number_of_cyclists; j++) Sem_post(&go);
		/* NO CODE HERE!!!!! */
		Sem_wait(&all_cyclists_set_up);

#ifdef DEBUG
		if(abort_on_start == 1)
			exit(EXIT_SUCCESS);
		if(i > 0)
			printf("End iteration %d\n", i);
		printf("Iteration %d\n", i + 1);
		Sem_wait(&simulation);
		printf("Printing runway\n");
		print_runway();
		printf("GO!\n");
#endif

		/*
		/*** Elimination ***
		completed_current_lap = 0;
		for (c = 0; c < initial_number_of_cyclists; c++)
			if (tinfo[c].lap >= current_lap)
				completed_current_lap++;
		if (completed_current_lap == current_number_of_cyclists) {
			last[0] = 0;
			/* Find last 3 cyclists *
			for (c = 0; c < initial_number_of_cyclists; c++)
				if (tinfo[c].status == RUNNING) {
					if (tinfo[c].lap < tinfo[last[0]].lap) last[0] = c;
					else if (tinfo[c].lap == tinfo[last[0]].lap &&
							tinfo[c].position_runway <
							tinfo[last[0]].position_runway) last[0] = c;
				}
			for (c = 0; c < initial_number_of_cyclists; c++)
				if (c != last[0] && tinfo[c].status == RUNNING) {
					if (tinfo[c].lap < tinfo[last[0]].lap) last[0] = c;
					else if (tinfo[c].lap == tinfo[last[0]].lap &&
							tinfo[c].position_runway <
							tinfo[last[0]].position_runway) last[1] = c;
				}
			for (c = 0; c < initial_number_of_cyclists; c++)
				if (c != last[0] && c!= last[1] && tinfo[c].status == RUNNING) {
					if (tinfo[c].lap < tinfo[last[0]].lap) last[0] = c;
					else if (tinfo[c].lap == tinfo[last[0]].lap &&
							tinfo[c].position_runway <
							tinfo[last[0]].position_runway) last[2] = c;
				}
			/*eliminate last one; update array final_position; update status;
			printf("Cyclist %d eliminated\n", last[0]);
			search penultimo and antepenultimo in the arrayzão and print;
			current_lap++;*
		}*/

		/*** Breaking ***/
		if (current_number_of_cyclists > 3) {
			next_breaking_attempt = 0;/*XXX remove this line*/
			for (c = 0; c < initial_number_of_cyclists; c++)
				if (tinfo[c].completed_laps == next_breaking_attempt)
					break;
			if (tinfo[c].completed_laps == next_breaking_attempt) {
				if (1){/*rand() % 100 == 42) {XXX substitute 1 for the commented code*/
					c = rand() % initial_number_of_cyclists;
					while (1) {
						if (tinfo[c].status == RUNNING) {
							tinfo[c].status = BROKEN;
							final_position[current_number_of_cyclists-1] =
								tinfo[c].cyclist_id;
							runway[tinfo[c].position_runway][tinfo[c].position_track] = 0;
							tinfo[c].kill_self = 1;
							printf("Cyclist %d just broke!\n", tinfo[c].cyclist_id);
							break;
						}
						c = (c + 1) % initial_number_of_cyclists;
					}
				}
				next_breaking_attempt += 4;
			}
		}

		/*** Debug ***/
		if (debug_flag) {
			debug_time++;
			if (debug_time == 1/*XXX change 1 to 200*/) {
				debug_time = 0;
				printf("\nDebug:\n");
				printf("C. ID | Laps | Current position\n");
				for (c = 0; c < initial_number_of_cyclists; c++) {
					printf("%5d | %4d | ", tinfo[c].cyclist_id, tinfo[c].completed_laps);
					switch (tinfo[c].status) {
						case BROKEN:
							printf("Broken\n");
							break;
						case ELIMINATED:
							printf("Eliminated\n");
							break;
						default:
						case RUNNING:
							printf("%d\n", tinfo[c].position_runway);
							break;
					}
				}
				printf("\n");
			}
		}
	}
	printf("End iteration %d\n", i);
	printf("End race\n");
	printf("\nClassificação final:\n");
	for (i = 0; i < initial_number_of_cyclists; i++)
		printf("%dº: %d\n", i + 1, final_position[i]);
	return EXIT_SUCCESS;


	/* XXX program gets stuck here */
	Sem_wait(&end_simulation);

	/*** Join threads ***/
	for(i = 0; i < initial_number_of_cyclists; i++)	{
		errno_cpy = pthread_join(threads[i], NULL);
		if(errno_cpy != 0) {
			sprintf(errmsg, "Error joining thread %d id %0lx",
					i, (unsigned long int) threads[i]);
			handle_error(errmsg);
		}
	}
	
	pthread_exit(NULL);
	return 0;
}
