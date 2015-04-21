#ifndef CYCLING_H
#define CYCLING_H

#include <pthread.h>
#include <semaphore.h>

extern sem_t create_thread; /* initialized with 0 */
extern sem_t all_cyclists_set_up; /* initialized with 0 */
extern sem_t go; /* initialized with 0 */
extern sem_t end_simulation; /* initialized with 0 */

/**/
extern sem_t lock_cyclists_set; /* initialized with 1 */
extern int cyclists_set;
/**/

/**/
extern sem_t lock_current_number_of_cyclists;
extern int current_number_of_cyclists;
/**/


extern int g_constant_speed; /* g for global */
extern int initial_number_of_cyclists;

struct runway_position {
	int free_positions;
	int position[4];
};

struct thread_info {     /* Used as argument to thread_start() */
	pthread_t thread_id;
	int thread_num;
	int cyclist_id;
	int position_runway;
	int next_position_runway;
	int position_runway_bkp;
	int next_position_runway_bkp;
	int position_track;
	int lap;
};

#ifdef DEBUG
extern sem_t simulation;
#endif

/* circuit with 4 tracks */
extern struct runway_position *runway;
extern sem_t *tracks; /* Each one initialized with 1 */
extern sem_t all_runway; /* Initialized with 1 */
extern int runway_length;

extern pthread_barrier_t bar; /* trying to be fair to threads */

#endif
