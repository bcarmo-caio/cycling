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
