#include "utils.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

void handle_error_en(int en, const char *msg) {
	errno = en;
	perror(msg);
	exit(EXIT_FAILURE);
}

void handle_error(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}
