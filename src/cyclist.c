#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "cyclist.h"
#include "cycling.h"
#include "utils.h"

/* Thread for a cyclist */
void *cyclist(void *me) {
#define me ((struct thread_info *) me)
	int errno_cpy;
	char errmsg[200];
	int i;

	/* Initialization */
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
	if (variable_speed) me->speed = 25;
	me->advanced_half_meter = 0;
	cyclists_set++;

	if(cyclists_set == current_number_of_cyclists) {
		cyclists_set = 0;

		/* Thread initialized */
		Sem_post(&create_thread, me->thread_num);

		/* Tell main that we are ready! */
		Sem_post(&all_cyclists_set_up, me->thread_num);
	}
	else
		/* Thread initialized */
		Sem_post(&create_thread, me->thread_num);

	/* Main loop for simulation */
	while(1) {
		/* Waiting for everyone to be set up */
		Sem_wait(&go, &(me->ts), me->thread_num);
		Pthread_barrier_wait(&bar);

		/* This cyclist was eliminated or broke, destroy thread */
		Sem_wait(&all_runway, &(me->ts), me->thread_num);
#ifdef DEBUG
		/*printf("Thread %d cyclist %d started\n", me->thread_num, me->cyclist_id);*/
#endif
		if (me->kill_self) {
			current_number_of_cyclists--;
			if (current_number_of_cyclists == 0) {
				Pthread_barrier_destroy(&bar);
				Sem_post(&all_cyclists_set_up, me->thread_num);
			}
			Sem_post(&all_runway, me->thread_num);
			pthread_exit(NULL);
		}

		/* Simulate cyclist here */
		if (variable_speed && me->speed == 25 && !me->advanced_half_meter) {
			me->advanced_half_meter = 1;
		}
		else if (!variable_speed || me->speed == 50 || (me->speed == 25 && me->advanced_half_meter)) {
			Sem_wait(tracks + me->position_runway, &(me->ts), me->thread_num);
			Sem_wait(tracks + me->next_position_runway, &(me->ts), me->thread_num);
			Sem_post(&all_runway, me->thread_num);

			me->advanced_half_meter = 0;
			me->position_runway_bkp = me->position_runway;
			me->next_position_runway_bkp = me->next_position_runway;

			for(i = 0; i < 4; i++)
				if(runway[me->next_position_runway][i] == 0)
					break;

			if(i < 4) { /* There is a free position =) I will proceed */
				runway[me->next_position_runway][i] = me->cyclist_id;
				runway[me->position_runway][me->position_track] = 0;
				me->position_track = i;

				/* Update my_position_runway */
				if(me->position_runway == 0) {
					me->position_runway = runway_length - 1;
					me->lap++;
					me->completed_laps++;
					if (variable_speed && me->completed_laps >= 1)
						me->speed = (( rand() % 2) + 1) * 25;
				}
				else
					me->position_runway--;

				/* Update my_next_position_runway */
				if(me->position_runway == 0)
					me->next_position_runway = runway_length - 1;
				else
					me->next_position_runway--;
			}

			Sem_post(tracks + me->position_runway_bkp, me->thread_num);
			Sem_post(tracks + me->next_position_runway_bkp, me->thread_num);
		}

		Sem_post(tracks + me->position_runway_bkp, me->thread_num);
		Sem_post(tracks + me->next_position_runway_bkp, me->thread_num);
#ifdef DEBUG
		/*printf("Thread %d cyclist %d ended\n", me->thread_num, me->cyclist_id);*/
#endif
		/* End here */

		/* Get ready for next iteration */
		Sem_wait(&lock_cyclists_set, &(me->ts), me->thread_num);
		cyclists_set++;

		if(cyclists_set == current_number_of_cyclists) {
			cyclists_set = 0;
#ifdef DEBUG
			printf("Everyone set!\n");
#endif
			Pthread_barrier_destroy(&bar);
			Sem_post(&lock_cyclists_set, me->thread_num);
			/* Tell main that we are ready! */
			Sem_post(&all_cyclists_set_up, me->thread_num);
		}
		Sem_post(&lock_cyclists_set, me->thread_num);
		/* NO CODE HERE!!!! */
	}
#undef me
}
