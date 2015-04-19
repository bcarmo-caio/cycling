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


void handle_error_en(int en, const char *msg) __attribute__ ((__noreturn__));
void handle_error_en(int en, const char *msg) {
	errno = en;
	perror(msg);
	exit(EXIT_FAILURE);
}

void handle_error(const char *msg) __attribute__ ((__noreturn__));
void handle_error(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

sem_t create_thread;
sem_t all_cyclists_set_up; /* must be initialized with 0 */
sem_t go; /* must be initialized with initial_number_of_cyclists */
sem_t end_simulation; /* must be initialized with 0 */

/**/
sem_t lock_cyclists_set; /* must be initialized with 1 */
int cyclists_set;
/**/

/**/
sem_t lock_current_number_of_cyclists;
int current_number_of_cyclists;
/**/


int g_constant_speed; /* g for global. Yes, I want a copy in each thread */
int initial_number_of_cyclists;

struct runway_position {
	int free_positions;
	int position[4];
};

struct thread_info {     /* Used as argument to thread_start() */
	int	thread_num;      /* Application-defined thread # */
	int cyclist_id;      /* Cyclist id */
};


/* circuit with 4 tracks */
struct runway_position *runway = NULL;


/* thread for a cyclist */
static void *cyclist(void *arg) {
	int thread_num      = ((struct thread_info *)arg)->thread_num;
	int cyclist_id      = ((struct thread_info *)arg)->cyclist_id;
	int __attribute__ ((__unused__)) constant_speed  = g_constant_speed;
	int errno_cpy;

	sem_wait(&lock_cyclists_set);
	cyclists_set++;
	if(cyclists_set == initial_number_of_cyclists) {
#ifdef DEBUG
		printf("Thread %d cyclist %d waiting\n", thread_num, cyclist_id);
		printf("Everyone set!\n");
#endif
		/* Tell main that we are ready! */
		if(sem_post(&all_cyclists_set_up) == -1) {
			errno_cpy = errno;
			handle_error_en(errno_cpy, "sem_post all_cyclists_set_up");
		}
	}
	else {
#ifdef DEBUG
		printf("Thread %d cyclist %d waiting\n", thread_num, cyclist_id);
#endif
		/* No one else will mess with cyclists_set if everyone is set up */
		if(sem_post(&lock_cyclists_set) == -1) {
			errno_cpy = errno;
			handle_error_en(errno_cpy, "sem_post lock_cyclists_set");
		}
	}

	sem_post(&create_thread);
	/* I am set up! waiting for everyone to be set up too */
	if(sem_wait(&go) == -1) {
		errno_cpy = errno;
		handle_error_en(errno_cpy, "sem_wait go");
	}

	if(sem_wait(&lock_current_number_of_cyclists) == -1) {
		errno_cpy = errno;
		handle_error_en(errno_cpy, "sem_wait lock_current_number_of_cyclists");
	}

#ifdef DEBUG
	printf("Thread %d cyclist %d gone\n", thread_num, cyclist_id);
#endif
	current_number_of_cyclists--;
	if(current_number_of_cyclists > 0) {
		if(sem_post(&lock_current_number_of_cyclists) == -1) {
			errno_cpy = errno;
			handle_error_en(errno_cpy, "sem_post "
					"lock_current_number_of_cyclists");
		}
	}
	else {
		if(current_number_of_cyclists == 0) {
			if(sem_post(&end_simulation) == -1) {
				errno_cpy = errno;
				handle_error_en(errno_cpy, "sem_post end_simulation");
			}
		}
		else
			/* Abort now! something weird has happened */
			handle_error("current_number_of_cyclists < 0");
	}
	
/*
	while(1) {
	}
*/	
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
	int d, i, j, __attribute__ ((__unused__)) debug_flag;
	int *start = NULL;
	pthread_t *threads = NULL;
	struct thread_info tinfo;
	char errmsg[200];
	int errno_cpy;
	int tmp;

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
	d = atoi(argv[1]);

	/* number of cyclists at start */
	initial_number_of_cyclists = atoi(argv[2]);
	current_number_of_cyclists = initial_number_of_cyclists;

	/*c := type of simulation
	 *c = 'v' means cyclists may change their speed from 25km/h to 50km/h
	 *	and vice-versa.
	 *c = 'u' means cyclist may NOT change their speed, and the speed is
	 *  a constant of 25km/h
	 */
	switch(argv[3][0]) {
		case 'c':
			g_constant_speed = 1;
			break;
		case 'v':
			g_constant_speed = 0;
			break;
		default:
			handle_error("Speed is not 'c' nor 'v'");
			break;
	}

	if (argc == 5 && (strcmp(argv[4], "-d")) == 0)
		debug_flag = 1;
	else
		debug_flag = 0;

	/* creating circuit with 4 tracks and length 'd' */
	runway = (struct runway_position *)
				malloc(d * sizeof(struct runway_position));
	if(!runway)
		handle_error("runway = malloc");

	/* allocating 'n' threads. One for each cyclist.*/
	threads = (pthread_t *)
		malloc(initial_number_of_cyclists * sizeof(pthread_t));
	if(threads == NULL)
		handle_error("threads = malloc");

	/* initializing semaphores */
	if(sem_init(&lock_cyclists_set, 0, 1) == -1) {
		errno_cpy = errno;
		handle_error_en(errno_cpy, "sem_init lock_cyclists_set");
	}

	/* Hold everyone */
	if(sem_init(&go, 0, 0) == -1) {
		errno_cpy = errno;
		handle_error_en(errno_cpy, "sem_init go"); /* or not */
	}

	if(sem_init(&end_simulation, 0, 0) == -1) {
		errno_cpy = errno;
		handle_error_en(errno_cpy, "sem_init end_simulation"); /* or not */
	}

	if(sem_init(&lock_current_number_of_cyclists, 0, 1) == -1) {
		errno_cpy = errno;
		handle_error_en(errno_cpy, "sem_init lock_current_number_of_cyclists");
	}

	if(sem_init(&all_cyclists_set_up, 0, 0) == -1) {
		errno_cpy = errno;
		handle_error_en(errno_cpy, "sem_init all_cyclists_set_up");
	}

	if(sem_init(&create_thread, 0, 1) == -1) {
		errno_cpy = errno;
		handle_error_en(errno_cpy, "sem_init create_thread");
	}
	/* initialized */

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
	/* shuffled */


	/* creating threads */
	for (i = 0; i < initial_number_of_cyclists; i++) {
		sem_wait(&create_thread);
		tinfo.thread_num = i + 1;
		tinfo.cyclist_id = start[i];
		errno_cpy = pthread_create( threads + i, /* pthread_t *thread */
									NULL, /* const pthread_attr_t *attr */
									&cyclist, /* void *(*routine) (void *) */
									(void *) &tinfo ); /*void *arg*/
		if (errno_cpy != 0)	{
			sprintf(errmsg, "pthread_cread %d for cyclist %d", i, start[i]);
			handle_error_en(errno_cpy, (const char *) errmsg);
		}
	}

	if(sem_wait(&all_cyclists_set_up) == -1) {
		errno_cpy = errno;
		handle_error_en(errno_cpy, "sem_wait all_cyclists_set_up");
	}
#ifdef DEBUG
	printf("GO!\n");
#endif
	/*3*/
	/*2*/
	/*1*/
	/*GOOOOO!*/
	/*gambiarra de teste e que nao entendi direito
	memset((void *) &go , initial_number_of_cyclists, 1);
	memset((void *) &go + 8, initial_number_of_cyclists, 1);
	*/
	/*nao queria desse jeito...
	 * talvez usar pthread_barrier*/
	for(i = 0; i < initial_number_of_cyclists; i++)
		if(sem_post(&go) == -1) {
			errno_cpy = errno;
			handle_error_en(errno_cpy, "sem_post go");
		}


/*	while (run_not_over) {
		simulate_lap(lap_number);
	}*/

	if(sem_wait(&end_simulation) == -1) {
		errno_cpy = errno;
		handle_error_en(errno_cpy, "sem_wait end_simulation");
	}

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
