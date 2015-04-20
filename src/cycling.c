/*XXX tirei o -Wno_unused_but_set do makefile
 * se quiser deixar uma var sem usar, faca
 * __attribute__ ((__unused__)) var
 * para deixar explicito que devemos fazer algo depois
 * */

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


void debug(void);

sem_t create_thread; /* must be initialized with 0*/
sem_t all_cyclists_set_up; /* must be initialized with 0 */
sem_t go; /* must be initialized with 0 */
sem_t end_simulation; /* must be initialized with 0 */

/**/
sem_t lock_cyclists_set; /* must be initialized with 1 */
int cyclists_set;
/**/

/**/
sem_t lock_current_number_of_cyclists;
int current_number_of_cyclists;
/**/


int g_constant_speed; /* g for global */
int initial_number_of_cyclists;

/* circuit with 4 tracks */
/**/
struct runway_position *runway;
sem_t *tracks; /* Each one must be initialized with 1 */
sem_t all_runway; /* Must be initialized with 1 */
/**/
int runway_length;

pthread_barrier_t bar;

#ifdef DEBUG
sem_t simulation;
#endif

int main(int argc, char **argv) {
	int d, i, j, __attribute__ ((__unused__)) debug_flag;
	int *start = NULL;
	pthread_t *threads = NULL;
	struct thread_info tinfo;
	char errmsg[200];
	int errno_cpy;
	int tmp;
#ifdef DEBUG
	int abort_on_start;
#endif

	if(argc < 4) {
		sprintf(errmsg, "%s d n [v|u] ['d']\n"
				"\td := distancia\n"
				"\tn := # ciclistas\n"
				"\tv := velocidade variada\n"
				"\tu := velocidade constante\n"
				"\t'd' (opcional) := habilita modo debug", argv[0]);
		handle_error(errmsg);
	}
	
	srand(time(NULL));

	/*d := track length*/
	runway_length = d = atoi(argv[1]);

	/* number of cyclists at start */
	initial_number_of_cyclists = atoi(argv[2]);
	current_number_of_cyclists = initial_number_of_cyclists;

	/* TODO: fix this comparison */
	if(runway_length <= initial_number_of_cyclists)
		handle_error("Nao cabe todo mundo na pista para largada.\n"
				"Tem mais ciclistas que o tamanaho do circuito");

	/*c := type of simulation
	 *c = 'v' means cyclists may change their speed from 25km/h to 50km/h
	 *	and vice-versa.
	 *c = 'u' means cyclist may NOT change their speed, and the speed is
	 *  a constant of 25km/h
	 */
	switch(argv[3][0]) {
		case 'u':
			g_constant_speed = 1;
			break;
		case 'v':
			g_constant_speed = 0;
			break;
		default:
			handle_error("Speed is not 'u' nor 'v'");
			break;
	}

	if (argc == 5 && (strcmp(argv[4], "-d")) == 0)
		debug_flag = 1;
	else
		debug_flag = 0;

#ifdef DEBUG
	abort_on_start = 0;
	if (argc == 5 && (strcmp(argv[4], "-A")) == 0)
		abort_on_start = 1;
	else
		if (argc == 6 && (strcmp(argv[5], "-A")) == 0)
			abort_on_start = 1;
#endif

	/* creating circuit with 4 tracks and length 'd' */
	runway = (struct runway_position *)
				malloc(d * sizeof(struct runway_position));
	if(!runway)
		handle_error("runway = malloc");
	memset((void *) runway, 0, d * sizeof(struct runway_position));

	/* allocating 'n' threads. One for each cyclist.*/
	threads = (pthread_t *)
		malloc(initial_number_of_cyclists * sizeof(pthread_t));
	if(threads == NULL)
		handle_error("threads = malloc");

	init_semaphores();

	start = (int *) malloc(initial_number_of_cyclists * sizeof(int));
	if(start == NULL)
		handle_error("start = malloc");

	/* shuffle start */
	for(i = 0; i < initial_number_of_cyclists; i++)
		start[i] = i + 1;
	/* http://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle */
	for(i = initial_number_of_cyclists - 1; i >= 1; i--)
	{
		j = rand() % (i + 1);
		tmp = start[i];
		start[i] = start[j];
		start[j] = tmp;
	}
	for(i = 0; i < initial_number_of_cyclists; i++)
		runway[i].position[0] = start[i];
	memset(start, 0, initial_number_of_cyclists * sizeof(int));
	free(start);
	start = NULL;
	/* shuffled */


	/* creating threads */
	for (i = 0; i < initial_number_of_cyclists; i++) {
		tinfo.thread_num = i + 1;
		tinfo.cyclist_id = runway[i].position[0];
		errno_cpy = pthread_create( threads + i, /* pthread_t *thread */
									NULL, /* const pthread_attr_t *attr */
									&cyclist, /* void *(*routine) (void *) */
									(void *) &tinfo); /*void *arg*/
		if (errno_cpy != 0)	{
			sprintf(errmsg, "pthread_cread %d for cyclist %d", i, start[i]);
			handle_error_en(errno_cpy, (const char *) errmsg);
		}
		Sem_wait(&create_thread);
	}
	/* NO CODE HERE!!!!! */
	/* start simulation */
	for(i = 0; i < 5; i++) {
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
		/*3*/
		/*2*/
		/*1*/
		/*GOOOOO!*/
		Pthread_barrier_init(&bar, 0, current_number_of_cyclists);

		for(j = 0; j < current_number_of_cyclists; j++)
			Sem_post(&go);
	/* NO CODE HERE!!!!! */
	}
	printf("End iteration %d\n", i);
	printf("End race\n");
	return EXIT_SUCCESS;


	/* XXX  program stucks here */
	Sem_wait(&end_simulation);

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

/*
simulate_lap(lap_number) {
	if (debug_flag && lap_number % 200 == 0) {
		/* debug /
		debug();
	}
	if (lap_number % 4 == 0) {
		/* breaking /
		breaking();
		update_positioning_data();
	}
	if (lap_number % 2 == 0) {
		/* elimination /
		while (lap_not_over) {
			simulate_72ms();
			pthread_barrier_wait(72ms_barrier);
		}
	}
}

void simulate_72ms(void) {
}

void breaking(void) {
}

void update_positioning_data(void) {
}
*/
void debug(void) {
}
