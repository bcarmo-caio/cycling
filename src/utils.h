#ifndef UTILS_H
#define UTILS_H

void handle_error_en(int en, const char *msg) __attribute__ ((__noreturn__));
void handle_error(const char *msg) __attribute__ ((__noreturn__));

/* Macros for semaphores */
#define Sem_init(sem, shared, num) do { \
	if(sem_init(sem, shared, num) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_init " # sem); \
	} \
	while(0)

#define Sem_post(sem) do { \
	if(sem_post(sem) != -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_post " # sem); \
	} \
	} while(0)

#define Sem_wait(sem) do { \
	if(sem_wait(sem) != -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_wait " # sem); \
	} \
	} while(0)

/* Macros for barriers */
#define Pthread_barrier_init(bar) do { \
	errno_cpy = pthread_barrier_init(bar, 0, current_number_of_cyclists); \
	if(errno_cpy != 0) \
		handle_error_en(errno_cpy, "barrier_init " # bar); \
	while(0) 

#define Pthread_barrier_wait(bar) do { \
	errno_cpy = pthread_barrier_wait(bar); \
	if((errno_cpy != PTHREAD_BARRIER_SERIAL_THREAD) && \
	   (errno_cpy != 0)) \
	   handle_error_en(errno_cpy, "barrier_wait " # bar); \
	while(0)

#define Pthread_barrier_destroy(bar) do { \
	errno_cpy = pthread_barrier_destroy(bar); \
	if(errno_cpy != 0) \
		handle_error_en(errno_cpy, "barrier_destory " # bar); \
	while(0)


/* Initializing all semaphores */
#define pre_init_semaphores do { \
	if(sem_init(&lock_cyclists_set, 0, 1) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_init lock_cyclists_set"); \
	} \
	\
	/* Hold everyone */ \
	if(sem_init(&go, 0, 0) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_init go"); /* or not */ \
	} \
	\
	if(sem_init(&end_simulation, 0, 0) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_init end_simulation"); /* or not */ \
	} \
	\
	if(sem_init(&lock_current_number_of_cyclists, 0, 1) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_init lock_current_number_of_cyclists");\
	} \
	\
	if(sem_init(&all_cyclists_set_up, 0, 0) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_init all_cyclists_set_up"); \
	} \
	\
	if(sem_init(&create_thread, 0, 0) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_init create_thread"); \
	} \
	\
	tracks = (sem_t *) malloc(runway_length * sizeof(sem_t)); \
	if(tracks == NULL) \
		handle_error("tracks = malloc"); \
	for(i = 0; i < runway_length; i++) \
		if(sem_init(tracks + i, 0, 1) == -1) { \
			errno_cpy = errno; \
			handle_error_en(errno_cpy, "sem_init tracks"); \
		} \
	\
	if(sem_init(&all_runway, 0, 1) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_init all_runway"); \
	}

#ifdef DEBUG
#define pos_init_semaphores  \
	\
	if(sem_init(&simulation, 0, 1) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_init simulation"); \
		} \
	printf("semaphores initialized [with debug]\n"); \
	} while(0)
#else
#define pos_init_semaphores \
	\
	printf("semaphores initialized\n"); \
	} while(0)
#endif /* DEBUG */

#define init_semaphores() \
	pre_init_semaphores \
	pos_init_semaphores 
	

#ifdef DEBUG
void print_runway(void);
#endif


#endif /* UTILS_H */
