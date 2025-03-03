#include "../src/wavl.h"
#include <stdlib.h>
#include "common/dla.h"
#include <time.h>
#include "common/common.h"
DLA_GEN(static,int,list, init, push, get)

void pop(wavl_t **tree, dla_t *l, size_t *len, size_t n)
{
	size_t i;
	int v;
	for(i = 0; i < n; ++i) {
		v = rto(n * 5);
		if (wavl_put(tree, aint(v), cmp)) {
			list_push(l, v);
			(*len)++;
		}
	}
}
void test(size_t n)
{
	dla_t l;
	wavl_t *tree = NULL;
	size_t	len,
		left;
	int	v,
		i,
		*p;
	double t_search;
	perf_t perf;
	len = 0;
	left = 0;
	t_search = 0;
	list_init(&l, n);
	pop(&tree, &l, &len, n);
	//wavl_print(stdout, tree, strint);

	left = len;
	while (left != 0) {
		i = rto(left - 1);
		v = list_get(&l, i);
		PERF_START(perf);
		p = wavl_take(&tree, &v, cmp);
		PERF_END(perf);
		t_search = perf_seconds(perf);
		if (p == NULL) {
			printf("Failed for %d -> NULL\n", v);
			exit(-1);
		}
		if (*p != v) {
			printf("Failed for %d -> %d\n", v, *p);
			exit(-1);
		}
		//printf("took %d\n", v);
		//wavl_print(stdout, tree, strint);
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