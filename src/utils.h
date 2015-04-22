#ifndef UTILS_H
#define UTILS_H

void handle_error_en(int en, const char *msg) __attribute__ ((__noreturn__));
void handle_error(const char *msg) __attribute__ ((__noreturn__));

#ifdef WAIT_SEM_TIMEOUT
#error "WAIT_SEM_TIMEOUT redefinition!"
#else
#define WAIT_SEM_TIMEOUT 5
#endif

#define Clock_gettime(clock, ts) do { \
	if (clock_gettime(clock, ts) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "clock_gettime(" # clock ", " # ts ")"); \
	} \
	} while (0)

/* Macros for semaphores */
#define Sem_init(sem, shared, num) do { \
	if(sem_init(sem, shared, num) == -1) { \
		errno_cpy = errno; \
		handle_error_en(errno_cpy, "sem_init(" # sem ", " # shared \
				", " # num ")"); \
	} \
	} while(0)

#define PRINT_SEM_POST_OK_AND_NOK
#if defined (PRINT_SEM_POST_OK_AND_NOK) && defined(DEBUG)
#define Sem_post(sem, thread) do { \
	if(sem_post(sem) == -1) { \
		errno_cpy = errno; \
		sprintf(errmsg, "sem_post(%s, %d)", # sem, thread); \
		handle_error_en(errno_cpy, errmsg); \
	} \
	else \
		errno_cpy = errno; \
		sprintf(errmsg, "sem_post(%s, %d)", # sem, thread); \
		errno = errno_cpy; \
		perror(errmsg); \
	} while(0)
#else
#define Sem_post(sem, thread) do { \
	if(sem_post(sem) == -1) { \
		errno_cpy = errno; \
		sprintf(errmsg, "sem_post(%s, %d)", # sem, thread); \
		handle_error_en(errno_cpy, errmsg); \
	} \
	} while(0)
#endif /* PRINT_SEM_OK_AND_NOK */

#define PRINT_SEM_WAIT_OK_AND_NOK
#if defined PRINT_SEM_WAIT_OK_AND_NOK && defined(DEBUG)
#define Sem_wait(sem, ts, thread) do { \
	Clock_gettime(CLOCK_REALTIME, ts); \
	(*ts).tv_sec += WAIT_SEM_TIMEOUT; \
	if(sem_timedwait(sem, ts) == -1) { \
		errno_cpy = errno; \
		sprintf(errmsg, "sem_timedwait(%s, %d)", # sem, thread); \
		handle_error_en(errno_cpy, errmsg); \
	} \
	else \
		errno_cpy = errno; \
		sprintf(errmsg, "sem_timedwait(%s, %d)", # sem, thread); \
		errno = errno_cpy; \
		perror(errmsg); \
	} while(0)
#else
#define Sem_wait(sem, ts, thread) do { \
	Clock_gettime(CLOCK_REALTIME, ts); \
	(*ts).tv_sec += WAIT_SEM_TIMEOUT; \
	if(sem_timedwait(sem, ts) == -1) { \
		errno_cpy = errno; \
		sprintf(errmsg, "sem_timedwait(%s, %d)", # sem, thread); \
		handle_error_en(errno_cpy, errmsg); \
	} \
	} while(0)
#endif

/* Macros for barriers */
#define Pthread_barrier_init(bar, shared, num) do { \
	errno_cpy = pthread_barrier_init(bar, shared, num); \
	if(errno_cpy != 0) \
		handle_error_en(errno_cpy, "pthread_barrier_init(" # bar ", " \
			# shared ", " # num ")"); \
	} while(0) 

#define Pthread_barrier_wait(bar) do { \
	errno_cpy = pthread_barrier_wait(bar); \
	if((errno_cpy != PTHREAD_BARRIER_SERIAL_THREAD) && \
	   (errno_cpy != 0)) \
	   handle_error_en(errno_cpy, "pthread_barrier_wait(" # bar ")"); \
	} while(0)

#define Pthread_barrier_destroy(bar) do { \
	errno_cpy = pthread_barrier_destroy(bar); \
	if(errno_cpy != 0) \
		handle_error_en(errno_cpy, "pthread_barrier_destory(" # bar ")"); \
	} while(0)

/* Initializing all semaphores */
#define init_semaphores() do { \
	Sem_init(&lock_cyclists_set, 0, 1); \
	/* Hold everyone */ \
	Sem_init(&go, 0, 0); \
	Sem_init(&lock_current_number_of_cyclists, 0, 1); \
	Sem_init(&all_cyclists_set_up, 0, 0); \
	Sem_init(&create_thread, 0, 0); \
	tracks = (sem_t *) malloc(runway_length * sizeof(sem_t)); \
	if(tracks == NULL) \
		handle_error("tracks = malloc"); \
	for(i = 0; i < runway_length; i++) \
		Sem_init(tracks + i, 0, 1); \
	Sem_init(&all_runway, 0, 1); \
	} while(0)

#ifdef DEBUG
void print_runway(void);
#endif

#endif /* UTILS_H */
