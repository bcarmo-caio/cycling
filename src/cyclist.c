#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "cyclist.h"
#include "cycling.h"
#include "utils.h"

/* Thread for a cyclist */
void *cyclist(void *me) {
	int errno_cpy;
	int i;
#define me ((struct thread_info *) me)

	me->position_runway = me->thread_num - 1;
	if(me->position_runway == 0)
		me->next_position_runway = runway_length - 1;
	else
		me->next_position_runway = me->position_runway - 1;

	me->position_track = 0;
	me->lap = 1;
	me->completed_laps = -1;
	me->status = RUNNING;
	me->kill_self = 0;
	cyclists_set++;

#ifdef DEBUG
	printf("Thread %d cyclist %d waiting\n", me->thread_num, me->cyclist_id);
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

	/* Main loop for simulation */
	while(1) {
		/* Waiting for everyone to be set up */
		Sem_wait(&go);
		Pthread_barrier_wait(&bar);

		if (me->kill_self) {
			Sem_wait(&lock_current_number_of_cyclists);
			current_number_of_cyclists--;
			Sem_post(&lock_current_number_of_cyclists);
			pthread_exit(NULL);
		}

		/* Simulate cyclist here */
		Sem_wait(&all_runway);
		Sem_wait(tracks + me->position_runway);
		Sem_wait(tracks + me->next_position_runway);
		Sem_post(&all_runway);

		me->position_runway_bkp = me->position_runway;
		me->next_position_runway_bkp = me->next_position_runway;

		for(i = 0; i < 4; i++)
			if(runway[me->next_position_runway][i] == 0)
				break;

		if(i < 4) { /* There a free position =) I will proceed */
			runway[me->next_position_runway][i] = me->cyclist_id;
			runway[me->position_runway][me->position_track] = 0;
			me->position_track = i;

			/* Update my_position_runway */
			if(me->position_runway == 0) {
				me->position_runway = runway_length - 1;
				me->lap++;
				me->completed_laps++;
			}
			else
				me->position_runway--;

			/* Update my_next_position_runway */
			if(me->position_runway == 0)
				me->next_position_runway = runway_length - 1;
			else
				me->next_position_runway--;
		}

		Sem_post(tracks + me->position_runway_bkp);
		Sem_post(tracks + me->next_position_runway_bkp);
#ifdef DEBUG
		printf("Thread %d cyclist %d started\n",
				me->thread_num, me->cyclist_id);
#endif
		/* End here */

		/* Get ready for next iteration */
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
			Pthread_barrier_destroy(&bar);
			Sem_post(&lock_cyclists_set);
			/* Tell main that we are ready! */
			Sem_post(&all_cyclists_set_up);
		}
		Sem_post(&lock_cyclists_set);
		/* NO CODE HERE!!!! */
	}

#if 0 /* Kill last runner */
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
#undef me
}
