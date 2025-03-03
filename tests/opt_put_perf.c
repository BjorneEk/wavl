#include "../src/owavl.h"
#include "common/dla.h"
#include "common/common.h"

int main(void)
{
	size_t i, j;
	owavl_t t;
	perf_t p;
	srand(time(NULL));
	for (i = 1; i <= N; ++i) {
		owavl_init(&t);
		PERF_START(p);
		for (j = 0; j < i; ++j)
			owavl_put(&t, aint(rto(N)), cmp);
		PERF_END(p)
		REPORT("%d: %lf\n", i, perf_seconds(p));
		owavl_free(&t);
	}
}