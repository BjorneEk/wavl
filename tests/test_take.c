#include "../src/wavl.h"
#include <stdlib.h>

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
static wavl_t *node(void *data)
{
	wavl_t *res;

	res = calloc(1,sizeof(wavl_t));
	res->data = data;
	res->r = 0;
	return res;
}
bool test_wavl_add(wavl_t **tree, void *data, int (*cmp)(void*, void*))
{
	wavl_t **n;
	wavl_t *p;
	int d;

	if (*tree == NULL) {
		*tree = node(data);
		return true;
	}
	n = tree;
	p = NULL;

	while (*n != NULL) {
		d = cmp(data, (*n)->data);
		p = *n;
		n = &(*n)->succ[d < 0 ? 0 : 1];
		if (d == 0)
			return false;
	}
	*n = node(data);
	(*n)->parent = p;
	return true;
}

int main(void)
{
	wavl_t *tree = NULL;
	int	i,
		n,
		*p;
	n = 30;
	for (i = 1; i <= n; i++)
		wavl_put(&tree, aint(i), cmp);
	wavl_print(stdout, tree, strint);
	for (i = 1; i <= 8; i++) {
		p = wavl_take(&tree, &i, cmp);
		if (*p != i) {
			printf("Failed for %d -> %d\n", i, *p);
		}
		free(p);
	}
	wavl_print(stdout, tree, strint);
	i = 16;
	p = wavl_take(&tree, &i, cmp);
	if (*p != i) {
		printf("Failed for %d -> %d\n", i, *p);
	}
	wavl_print(stdout, tree, strint);
}