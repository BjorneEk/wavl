#include "../src/owavl.h"
#include "common/common.h"

int main(void)
{
	owavl_t tree;
	int	i,
		*p;
	owavl_init(&tree);
	i = 0;
	p = owavl_take(&tree, &i, cmp);
	return (p != 0) ? -1 : 0;
}