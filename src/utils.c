#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>
#include "utils.h"

void handle_error_en(int en, const char *msg) {
	errno = en;
	perror(msg);
	exit(EXIT_FAILURE);
}

void handle_error(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

#ifdef DEBUG
#include "cycling.h"

void print_runway(void) {
	int i, j, errno_cpy;
	char runner[4][10];
	printf("-------------------------------------------------------------"
			"----\n");
	for(i = 0; i < runway_length; i++) {
		for(j = 0; j < 4; j++)
			if(runway[i][j] == 0)
				sprintf(runner[j], "--");
			else
				sprintf(runner[j], "%d", runway[i][j]);
		printf("|\t%s\t|\t%s\t|\t%s\t|\t%s\t|\n", runner[0], runner[1],
				runner[2], runner[3]);
	}
	printf("-------------------------------------------------------------"
			"----\n");
}
#endif /* DEBUG */
