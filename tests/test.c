#include "../src/wavl.h"
#include <stdlib.h>
#include "common/dla.h"
DLA_GEN(static,int,list, init, push, get)

int cmp(void *a, void *b)
{
	return *(int*)a - *(int*)b;
}

int *aint(int i)
{
	int *r;
	r = malloc(sizeof(int));
	*r = i;
	return r;
}
void strint(char *buff, void *ip)
{
	sprintf(buff, "%d", *(int*)ip);
}

static int rto(int max)
{
	return rand() % (max + 1);
}
void test(size_t n)
{
	dla_t l;
	wavl_t *tree;
	size_t	len,
		left;
	int	v,
		i,
		*p;

	len = 0;
	left = 0;
	list_init(&l, n);

	for(i = 0; i < n; ++i) {
		v = rto(n * 5);
		if (wavl_put(&tree, aint(v), cmp)) {
			list_push(&l, v);
			len++;
		}
	}
	wavl_print(stdout, tree, strint);
	left = len;
	while (left != 0) {
		i = rto(left);
		v = list_get(&l, i);
		p = wavl_take(&tree, &v, cmp);
		if (p == NULL) {
			printf("Failed for %d -> NULL\n", i);
			exit(-1);
		}
		if (*p != v) {
			printf("Failed for %d -> %d\n", v, *p);
			exit(-1);
		}
		printf("took %d\n", v);
		wavl_print(stdout, tree, strint);
		DLA_FILTER(&l, int, m, *m != v,;)
		--left;
	}
}
int main(void)
{
	test(10);
}