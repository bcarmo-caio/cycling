#ifndef CYCLING_H
#define CYCLING_H

#include <semaphore.h>

extern sem_t create_thread;
extern sem_t all_cyclists_set_up; /* must be initialized with 0 */
extern sem_t go; /* must be initialized with initial_number_of_cyclists */
extern sem_t end_simulation; /* must be initialized with 0 */

/**/
extern sem_t lock_cyclists_set; /* must be initialized with 1 */
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
	int	thread_num;      /* Application-defined thread # */
	int cyclist_id;      /* Cyclist id */
};


/* circuit with 4 tracks */
extern struct runway_position *runway;

#endif
