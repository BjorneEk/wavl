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
	if (*(int*)ip < 0)
		sprintf(buff, "%d", *(int*)ip);
	else
		sprintf(buff, " %d", *(int*)ip);
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
void add(wavl_t **t, int i)
{
	wavl_put(t, aint(i), cmp);
	printf("%d\n", i);
	wavl_print(stdout, *t, strint);
}
void rem(wavl_t **t, int i)
{
	wavl_take(t, &i, cmp);
	wavl_print(stdout, *t, strint);
}
int main(void)
{
	wavl_t *tree = NULL;
	int i;
	for (i = 1; i <= 0; i++) {
		add(&tree, i);
	}
	//add(&tree, -1);
	//add(&tree, -2);
	//add(&tree, 7);
	//rem(&tree, 7);
	//rem(&tree, 6);
}