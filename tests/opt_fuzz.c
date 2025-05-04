#include "common/common.h"
#include "../src/owavl.h"
#include <stdlib.h>
#include "common/dla.h"
#include <time.h>
DLA_GEN(static,int,list, init, push, get)

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
	perf_t perf;
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
		PERF_START(perf);
		p = owavl_take(&tree, &v, cmp);
		PERF_END(perf);
		t_search = perf_seconds(perf);
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
	REPORT("search time: %lf\n", t_search);
}
int main(void)
{
	srand(time(NULL));
	test(50000);
}
