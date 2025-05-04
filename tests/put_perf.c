#include "common/common.h"
#include "../src/wavl.h"
#include "common/dla.h"

int main(void)
{
	size_t i, j;
	wavl_t *t = NULL;
	perf_t p;

	for (i = 1; i <= N; ++i) {
		PERF_START(p);
		for (j = 0; j < i; ++j)
			wavl_put(&t, aint(rto(N)), cmp);
		PERF_END(p)
		REPORT("%d: %lf\n", i, perf_seconds(p));
		wavl_free(&t);
	}
}
