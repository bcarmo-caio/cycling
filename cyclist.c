#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
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
	int __attribute__ ((__unused__)) my_position_runway;
	int __attribute__ ((__unused__)) my_position_track;

	my_position_runway = cyclist_id;
	my_position_track  = 0;
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
		sem_post(&create_thread);

		/* Tell main that we are ready! */
		if(sem_post(&all_cyclists_set_up) == -1) {
			errno_cpy = errno;
			handle_error_en(errno_cpy, "sem_post all_cyclists_set_up");
		}
	}
	else
		/* THREAD INITIALIZED */
		sem_post(&create_thread);



	while(1) {
		/* waiting for everyone to be set up */
		if(sem_wait(&go) == -1) {
			errno_cpy = errno;
			handle_error_en(errno_cpy, "sem_wait go");
		}

/* simulate cyclist here */
#ifdef DEBUG
		sem_wait(&simulation);
		printf("!v==0 \n");
		printf("Thread %d cyclist %d started\n", thread_num, cyclist_id);
		print_runway();
		sem_post(&simulation);
#endif
/* end here */

/* get ready for next iteration */
		sem_wait(&lock_cyclists_set);
		cyclists_set++;

#ifdef DEBUG
		printf("Thread %d cyclist %d waiting\n", thread_num, cyclist_id);
#endif

		if(cyclists_set == current_number_of_cyclists) {
			cyclists_set = 0;

#ifdef DEBUG
			printf("Everyone set!\n");
#endif

			if(sem_post(&lock_cyclists_set) == -1) {
				errno_cpy = errno;
				handle_error_en(errno_cpy, "sem_post lock_cyclists_set");
			}
			/* Tell main that we are ready! */
			if(sem_post(&all_cyclists_set_up) == -1) {
				errno_cpy = errno;
				handle_error_en(errno_cpy, "sem_post all_cyclists_set_up");
			}
		}
		if(sem_post(&lock_cyclists_set) == -1) {
			errno_cpy = errno;
			handle_error_en(errno_cpy, "sem_post lock_cyclists_set");
		}
	}

#if 0 /* kill last runner */
		if(sem_wait(&lock_current_number_of_cyclists) == -1) {
			errno_cpy = errno;
			handle_error_en(errno_cpy, "sem_wait lock_current_number_of_cyclists");
		}
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
#endif
	pthread_exit(NULL);
}
