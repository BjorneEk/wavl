
#ifndef _COMMON_H_
#define _COMMON_H_

#define _POSIX_C_SOURCE 199309L

#include <time.h>

int cmp(void *a, void *b);
int *aint(int i);
void strint(char *buff, void *ip);

typedef struct perf {
	struct timespec time_start;
	struct timespec time_end;
} perf_t;

#define PERF_START(p) do {clock_gettime(CLOCK_MONOTONIC, &(p).time_start);} while (0);
#define PERF_END(p) do {clock_gettime(CLOCK_MONOTONIC, &(p).time_end);} while (0);

double perf_seconds(perf_t p);

int rto(int max);

void report(const char *test, const char *fmt, ...);
extern int N;

#define REPORT(...) report(__FILE__, __VA_ARGS__)
#endif
