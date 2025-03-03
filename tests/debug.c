#include "../src/owavl.h"
#include "common/common.h"


void add(owavl_t *t, int i)
{
	owavl_put(t, aint(i), cmp);
	//printf("%d\n", i);
	//owavl_print(stdout, t, strint);
}
void rem(owavl_t *t, int i)
{
	owavl_take(t, &i, cmp);
	owavl_print(stdout, t, strint);
}
int main(void)
{
	owavl_t tree;
	int i;
	owavl_init(&tree);
	for (i = 1; i <= 0; i++) {
		add(&tree, i);
	}
	//owavl_print(stdout, &tree, strint);
	//add(&tree, -1);
	//add(&tree, -2);
	//add(&tree, 7);
	//rem(&tree, 7);
	//rem(&tree, 6);
	return 0;
}