#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "cyclist.h"
#include "cycling.h"
#include "utils.h"


/* thread for a cyclist */
void *cyclist(void *arg) {
	int thread_num      = ((struct thread_info *)arg)->thread_num;
	int cyclist_id      = ((struct thread_info *)arg)->cyclist_id;
	int __attribute__ ((__unused__)) constant_speed  = g_constant_speed;
	int errno_cpy;
	int my_position_runway;
	int my_next_position_runway;
	int my_position_runway_bkp;
	int my_next_position_runway_bkp;
	int my_position_track;
	int __attribute__ ((__unused__)) my_lap;
	int i;

	my_position_runway = thread_num - 1;
	if(my_position_runway == 0)
		my_next_position_runway = runway_length - 1;
	else
		my_next_position_runway = my_position_runway - 1;

	my_position_track  = 0;
	my_lap = 1;
	cyclists_set++;

#ifdef DEBUG
	printf("Thread %d cyclist %d waiting\n", thread_num, cyclist_id);
#endif
	
	if(cyclists_set == current_number_of_cyclists) {
		cyclists_set = 0;

#ifdef DEBUG
		printf("Everyone set!\n");
#endif

		/* THREAD INITIALIZED */
		Sem_post(&create_thread);

		/* Tell main that we are ready! */
		Sem_post(&all_cyclists_set_up);
	}
	else
		/* THREAD INITIALIZED */
		Sem_post(&create_thread);

	/* main loop for simulation */
	while(1) {
		/* waiting for everyone to be set up */
		Sem_wait(&go);

		errno_cpy = pthread_barrier_wait(&bar);
		if((errno_cpy != PTHREAD_BARRIER_SERIAL_THREAD) &&
		   (errno_cpy != 0))
		   handle_error_en(errno_cpy, "barrier_wait bar");


		/* simulate cyclist here */
		Sem_wait(&all_runway);
		Sem_wait(tracks + my_position_runway);
		Sem_wait(tracks + my_next_position_runway);
		Sem_post(&all_runway);

		my_position_runway_bkp = my_position_runway;
		my_next_position_runway_bkp = my_next_position_runway;

		for(i = 0; i < 4; i++)
			if(runway[my_next_position_runway].position[i] == 0)
				break;

		if(i < 4) { /* there a free position =) I will proceed */
			runway[my_next_position_runway].position[i] = cyclist_id;
			runway[my_position_runway].position[my_position_track] = 0;
			my_position_track = i;

			/* update my_position_runway*/
			if(my_position_runway == 0)
			{
				my_position_runway = runway_length - 1;
				my_lap++;
			}
			else
				my_position_runway--;

			/* update my_next_position_runway*/
			if(my_position_runway == 0)
				my_next_position_runway = runway_length - 1;
			else
				my_next_position_runway--;
		}

		Sem_post(tracks + my_position_runway_bkp);
		Sem_post(tracks + my_next_position_runway_bkp);
#ifdef DEBUG
		printf("Thread %d cyclist %d started\n", thread_num, cyclist_id);
#endif
		/* end here */


		/* get ready for next iteration */
		Sem_wait(&lock_cyclists_set);
		cyclists_set++;
#ifdef DEBUG
		/*printf("Thread %d cyclist %d waiting\n", thread_num, cyclist_id);*/
#endif

		if(cyclists_set == current_number_of_cyclists) {
			cyclists_set = 0;

#ifdef DEBUG
			printf("Everyone set again!\n");
#endif
			errno_cpy = pthread_barrier_destroy(&bar);
			if(errno_cpy != 0)
				handle_error_en(errno_cpy, "barrier_destory bar");

			Sem_post(&lock_cyclists_set);
			/* Tell main that we are ready! */
			Sem_post(&all_cyclists_set_up);
		}
		Sem_post(&lock_cyclists_set);
		/* NO CODE HERE!!!! */
	}

#if 0 /* kill last runner */
		Sem_wait(&lock_current_number_of_cyclists);
		current_number_of_cyclists--;
		if(current_number_of_cyclists > 0) {
			Sem_post(&lock_current_number_of_cyclists);
		}
		else {
			if(current_number_of_cyclists == 0) {
				Sem_post(&end_simulation);
			}
			else
				/* Abort now! something weird has happened */
				handle_error("current_number_of_cyclists < 0");
		}
#endif
	pthread_exit(NULL);
}
