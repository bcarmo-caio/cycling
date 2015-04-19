#ifndef UTILS_H
#define UTILS_H

void handle_error_en(int en, const char *msg) __attribute__ ((__noreturn__));
void handle_error(const char *msg) __attribute__ ((__noreturn__));

#ifdef DEBUG
void print_runway(void);
#endif


#endif /* UTILS_H */
