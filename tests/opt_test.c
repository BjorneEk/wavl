#include "../src/owavl.h"
#include <stdlib.h>
#include "common/dla.h"
#include <time.h>
DLA_GEN(static,int,list, init, push, get)
#define TIME(...)	({							\
	double _time_res;							\
	do {									\
		struct timespec _time_start, _time_end;				\
		long _time_seconds, _time_nanoseconds;				\
		clock_gettime(CLOCK_MONOTONIC, &_time_start);			\
		__VA_ARGS__;							\
		clock_gettime(CLOCK_MONOTONIC, &_time_end);			\
		_time_seconds = _time_end.tv_sec - _time_start.tv_sec;		\
		_time_nanoseconds = _time_end.tv_nsec - _time_start.tv_nsec;	\
		if (_time_nanoseconds < 0) {					\
			_time_seconds--;					\
			_time_nanoseconds += 1000000000;			\
		}								\
		_time_res = _time_seconds + _time_nanoseconds / 1e9;		\
	} while(0);								\
	_time_res;								\
})
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
	sprintf(buff, "%d", *(int*)ip);
}

static int rto(int max)
{
	return rand() % (max + 1);
}
void pop(owavl_t *t, dla_t *l, size_t *len, size_t n)
{
	size_t i;
	int v;
	//char buff[100];
	for(i = 0; i < n; ++i) {
		v = rto(n * 5);
		//printf("add %d\n", v);
		//scanf("%s", buff);
		if (owavl_put(t, aint(v), cmp)) {
			list_push(l, v);
			(*len)++;
		}


		//owavl_print(stdout, t, strint);
		//exit(-1);
	}
}
void test(size_t n)
{
	dla_t l;
	owavl_t tree;
	size_t	len,
		left;
	int	v,
		i,
		*p;
	double t_search;
	len = 0;
	left = 0;
	t_search = 0;
	owavl_init(&tree);
	list_init(&l, n);
	pop(&tree, &l, &len, n);
	//wavl_print(stdout, tree, strint);
	DLA_FOREACH(&l, int, val, {
		p = owavl_get(&tree, &val, cmp);
		if (p == NULL) {
			printf("Failed for get(%d) -> NULL\n", val);
			exit(-1);
		}
		if (*p != val) {
			printf("Failed for get(%d) -> %d\n", val, *p);
			exit(-1);
		}
	})
	left = len;
	while (left != 0) {
		i = rto(left - 1);
		v = list_get(&l, i);
		t_search += TIME(p = owavl_take(&tree, &v, cmp));
		if (p == NULL) {
			printf("Failed for take(%d) -> NULL\n", v);
			exit(-1);
		}
		if (*p != v) {
			printf("Failed for take(%d) -> %d\n", v, *p);
			exit(-1);
		}
		//printf("took %d\n", v);
		//owavl_print(stdout, &tree, strint);
		DLA_FILTER(&l, int, m, *m != v,;)
		--left;
	}
	printf("search time: %lf\n", t_search);
}
int main(void)
{
	test(50000);
}