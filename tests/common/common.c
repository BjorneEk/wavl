
#include "common.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

int N = 1000;
int cmp(void *a, void *b)
{
	return *(int*)a - *(int*)b;
}
int *aint(int i)
{
	int *r;
	r = malloc(sizeof(int));
	*r = i;
	return r;
}
void strint(char *buff, void *ip)
{
	if (*(int*)ip < 0)
		sprintf(buff, "%d", *(int*)ip);
	else
		sprintf(buff, " %d", *(int*)ip);
}

double perf_seconds(perf_t p)
{
	long	time_seconds,
		time_nanoseconds;
	time_seconds = p.time_end.tv_sec - p.time_start.tv_sec;
	time_nanoseconds = p.time_end.tv_nsec - p.time_start.tv_nsec;
	if (time_nanoseconds < 0) {
		time_seconds--;
		time_nanoseconds += 1000000000;
	}
	return (double)(time_seconds + time_nanoseconds) / 1e9;
}


static FILE *report_file(const char *test)
{
	char file[4096] = "files/";
	int len;
	const char *s;
	FILE *res;

	len = strlen(test);
	s = &test[len];
	while (*--s != '/')
		;
	strcat(file, s);
	len = strlen(file);
	file[len - 2] = '\0';
	res = fopen(file, "a+");
	if (!res) {
		fprintf(stderr, "Error opening file '%s' in function %s (File: %s, Line: %d): %s\n", file, __func__, __FILE__, __LINE__, strerror(errno));
		exit(-1);
	}
	return res;
}

void report(const char *test, const char *fmt, ...)
{
	FILE *fp;
	va_list args;

	va_start(args, fmt);

	fp = report_file(test);
	vfprintf(fp, fmt, args);
	va_end(args);
	fclose(fp);
}

int rto(int max)
{
	return rand() % (max + 1);
}
