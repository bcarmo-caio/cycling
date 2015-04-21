#ifndef CYCLING_H
#define CYCLING_H

#include <pthread.h>
#include <semaphore.h>

#define BROKEN 0
#define ELIMINATED -1
#define RUNNING 1

/* Used as argument to thread_start() */
struct thread_info {
	pthread_t thread_id;
	int thread_num;
	int cyclist_id;
	int position_runway;
	int next_position_runway;
	int position_runway_bkp;
	int next_position_runway_bkp;
	int position_track;
	int lap;
	int completed_laps;
	int status;
	int kill_self;
	int speed;
	int advanced_half_meter;
};

extern sem_t create_thread; /* Initialized with 0 */
extern sem_t all_cyclists_set_up; /* Initialized with 0 */
extern sem_t go; /* Initialized with 0 */

extern sem_t lock_cyclists_set; /* Initialized with 1 */
extern int cyclists_set;

extern int initial_number_of_cyclists;
extern sem_t lock_current_number_of_cyclists;
extern int current_number_of_cyclists;

/* Circuit with 4 tracks */
extern int **runway;
extern sem_t *tracks; /* Each one initialized with 1 */
extern sem_t all_runway; /* Initialized with 1 */
extern int runway_length;

extern int variable_speed;

extern pthread_barrier_t bar; /* Trying to be fair to threads */

#endif
