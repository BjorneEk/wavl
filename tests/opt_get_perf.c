#include "../src/owavl.h"
#include "common/dla.h"
#include "common/common.h"
#include "common/dla.h"
DLA_GEN(static,int,list, init, push, getp)
void populate(owavl_t *t, dla_t *l, size_t n)
{
	size_t i;
	int *v;
	list_init(l, n);
	v = aint(rto(N));
	i = 0;
	while (i < n) {
		if (owavl_put(t, v, cmp)) {
			list_push(l, *v);
			++i;
			v = aint(rto(N));
		} else {
			*v = rto(N);
		}
	}
}

int main(void)
{
	size_t i, j;
	owavl_t t;
	perf_t p;
	dla_t l;
	int *ret, *in;
	double xtime = 0;
	double sum = 0;
	srand(time(NULL));
	for (i = 1; i <= N; ++i) {
		owavl_init(&t);
		populate(&t, &l, i);
		xtime = 0;
		for (j = 0; j < i; ++j) {
			in = list_getp(&l, j);
			PERF_START(p);
			ret = owavl_get(&t, in, cmp);
			PERF_END(p);
			xtime += perf_seconds(p);
			if (*ret != *in)
				return -1;
		}
		sum += xtime;
		REPORT("%d: %lf\n", i, xtime);
		owavl_free(&t);
	}
	REPORT("sum: %lf\n", sum);
}