#include "../src/wavl.h"
#include <stdlib.h>
#include "common/common.h"


int main(void)
{
	wavl_t *tree = NULL;
	int	i,
		n,
		*p;
	n = 30;
	for (i = 1; i <= n; i++)
		wavl_put(&tree, aint(i), cmp);
	//wavl_print(stdout, tree, strint);
	for (i = 1; i <= 8; i++) {
		p = wavl_take(&tree, &i, cmp);
		if (*p != i) {
			printf("Failed for %d -> %d\n", i, *p);
			wavl_print(stdout, tree, strint);
		}
		free(p);
	}
	//wavl_print(stdout, tree, strint);
	i = 16;
	p = wavl_take(&tree, &i, cmp);
	if (*p != i) {
		printf("Failed for %d -> %d\n", i, *p);
		wavl_print(stdout, tree, strint);
	}
	//wavl_print(stdout, tree, strint);
}