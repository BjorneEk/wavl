
#include "common/common.h"
#include "../src/wavl.h"
#include <stdlib.h>
#include "common/dla.h"

int comp(void*a, void*b)
{
	return *(int*)a - *(int*)b;
}

void pop(wavl_t **tree, size_t n)
{
	size_t i;
	for(i = 0; i < n; ++i) {
		wavl_put(tree, aint(i), comp);
	}
}

void test(size_t n)
{
	wavl_t *tree = NULL;
	int	v,
		*p;
	pop(&tree, n);
	//wavl_print(stdout, tree, strint);

	for (v = 0; v < n; ++v) {
		p = wavl_take(&tree, &v, comp);
		if (p == NULL) {
			printf("Failed for %d -> NULL\n", v);
			exit(-1);
		}
		if (*p != v) {
			printf("Failed for %d -> %d\n", v, *p);
			exit(-1);
		}
	}
}
int main(void)
{
	srand(time(NULL));
	test(50000000);
}
