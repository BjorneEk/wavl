#include "../src/wavl.h"
#include "common/common.h"

int main(void)
{
	wavl_t *tree = NULL;
	int	i,
		*p;

	i = 0;
	p = wavl_take(&tree, &i, cmp);
	return (p != 0) ? -1 : 0;
}