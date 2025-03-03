#include "../src/wavl.h"
#include <stdint.h>
#include <stdlib.h>
#include "common/common.h"
#define TAG_MASK 0x1
#define PTR_MASK (~(uintptr_t)TAG_MASK)
static wavl_t *node(void *data)
{
	wavl_t *res;

	res = calloc(1,sizeof(wavl_t));

	res->ptr_with_par = ((uintptr_t)data & PTR_MASK);
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
		d = cmp(data, (void*)((*n)->ptr_with_par & PTR_MASK));
		p = *n;
		n = &(*n)->succ[d < 0 ? 0 : 1];
		if (d == 0)
			return false;
	}
	*n = node(data);
	(*n)->parent = p;
	return true;
}
wavl_t *rl_tree(void)
{
	wavl_t *tree = NULL;

	test_wavl_add(&tree, aint(4), cmp);
	test_wavl_add(&tree, aint(2), cmp);
	test_wavl_add(&tree, aint(1), cmp);
	test_wavl_add(&tree, aint(3), cmp);
	test_wavl_add(&tree, aint(8), cmp);
	test_wavl_add(&tree, aint(6), cmp);
	test_wavl_add(&tree, aint(7), cmp);
	test_wavl_add(&tree, aint(5), cmp);
	test_wavl_add(&tree, aint(10), cmp);
	test_wavl_add(&tree, aint(9), cmp);
	test_wavl_add(&tree, aint(11), cmp);
	return tree;
}
wavl_t *drl_tree(void)
{
	wavl_t *tree = NULL;

	test_wavl_add(&tree, aint(4), cmp);
	test_wavl_add(&tree, aint(2), cmp);
	test_wavl_add(&tree, aint(12), cmp);
	test_wavl_add(&tree, aint(1), cmp);
	test_wavl_add(&tree, aint(3), cmp);
	test_wavl_add(&tree, aint(8), cmp);
	test_wavl_add(&tree, aint(14), cmp);
	test_wavl_add(&tree, aint(6), cmp);
	test_wavl_add(&tree, aint(10), cmp);
	test_wavl_add(&tree, aint(13), cmp);
	test_wavl_add(&tree, aint(15), cmp);
	test_wavl_add(&tree, aint(5), cmp);
	test_wavl_add(&tree, aint(7), cmp);
	test_wavl_add(&tree, aint(9), cmp);
	test_wavl_add(&tree, aint(11), cmp);
	return tree;
}
wavl_t *dll_tree(void)
{
	wavl_t *tree = NULL;

	test_wavl_add(&tree, aint(12), cmp);
	test_wavl_add(&tree, aint(14), cmp);
	test_wavl_add(&tree, aint(4), cmp);
	test_wavl_add(&tree, aint(15), cmp);
	test_wavl_add(&tree, aint(13), cmp);
	test_wavl_add(&tree, aint(8), cmp);
	test_wavl_add(&tree, aint(2), cmp);
	test_wavl_add(&tree, aint(10), cmp);
	test_wavl_add(&tree, aint(6), cmp);
	test_wavl_add(&tree, aint(3), cmp);
	test_wavl_add(&tree, aint(1), cmp);
	test_wavl_add(&tree, aint(11), cmp);
	test_wavl_add(&tree, aint(9), cmp);
	test_wavl_add(&tree, aint(7), cmp);
	test_wavl_add(&tree, aint(5), cmp);
	return tree;
}
bool tree_eq(wavl_t *t1, wavl_t *t2)
{
	if (t1 == NULL && t2 == NULL)
		return true;
	if (t1 == NULL || t2 == NULL)
		return false;
	if (*(int*)(t1->ptr_with_par & PTR_MASK) != *(int*)(t2->ptr_with_par & PTR_MASK))
		return false;
	return tree_eq(t1->succ[0], t2->succ[0]) && tree_eq(t1->succ[1], t2->succ[1]);
}

bool test_double_rotate(void)
{
	wavl_t *t1, *t2;
	bool res;
	t1 = drl_tree();
	t2 = dll_tree();

	double_rotate(&t1, t1->succ[1], true);
	double_rotate(&t2, t2->succ[0], false);
	res = tree_eq(t1, t2);
	if (!res) {
		printf("double T1\n");
		wavl_print(stdout, t1, strint);
		printf("double T2\n");
		wavl_print(stdout, t2, strint);
	}
	return res;
}

bool test_rotate(void)
{
	wavl_t *t1, *t2;
	bool res;
	t1 = rl_tree();
	t2 = rl_tree();

	single_rotate(&t1, t1->succ[1], true);
	single_rotate(&t1, t1->succ[0], false);
	res = tree_eq(t1, t2);
	if (!res) {
		printf("single T1\n");
		wavl_print(stdout, t1, strint);
		printf("single T2\n");
		wavl_print(stdout, t2, strint);
	}
	return res;
}
int main(void)
{
	bool r1, r2;
	/*
	wavl_t *tree = NULL;
	wavl_t *x, *y;
	tree = rl_tree();
	y = tree;
	x = y->succ[1];
	wavl_print(stdout, tree, strint);
	printf("a = 2(1, 3), b = 6(7, 5), c = 10(9, 11), y = 4, x = 8\n");
	single_rotate(&tree, x, true);
	wavl_print(stdout, tree, strint);
	tree = drl_tree();
	printf("a = 2(1, 3), b = 6(5, 7), c = 10(9, 11), d = 14(13, 15), x = 2, y = 12, z = 4, v = 8, w = 14\n");
	wavl_print(stdout, tree, strint);
	y = tree->succ[1];
	double_rotate(&tree, y, true);
	wavl_print(stdout, tree, strint);
	tree = dll_tree();
	printf("d = 2(1, 3), c = 6(5, 7), b = 10(9, 11), a = 14(13, 15), x = 14, y = 4, z = 12, v = 8, w = 2\n");
	wavl_print(stdout, tree, strint);
	y = tree->succ[0];
	double_rotate(&tree, y, false);
	wavl_print(stdout, tree, strint);
	*/
	r1 = test_rotate();
	r2 = test_double_rotate();
	if (!r1 || !r2)
		return -1;
	return 0;
}