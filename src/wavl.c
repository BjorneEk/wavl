
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wavl.h"

static wavl_t *node(void *data)
{
	wavl_t *res;

	res = calloc(1,sizeof(wavl_t));
	res->data = data;
	res->r = 0;
	return res;
}

static bool get_par(wavl_t *n)
{
	return n == NULL ? true : n->r;
}

static wavl_t *sibling(wavl_t *n)
{
	if (n == NULL || n->parent == NULL)
		return NULL;
	return n->parent->succ[n == n->parent->succ[0] ? 1 : 0];
}

static bool is_leaf(wavl_t *n)
{
	return n->succ[0] == NULL && n->succ[1] == NULL;
}

static void prom(wavl_t *n)
{
	if (n != NULL)
		n->r = !n->r;
}
static void dem(wavl_t *n)
{
	if (n != NULL)
		n->r = !n->r;
}

void double_rotate(wavl_t **tree, wavl_t *y, bool left)
{
	wavl_t *z, *v;
	int zpi;
	int i;

	i = left ? 0 : 1;
	z = y->parent;
	v = y->succ[i];

	if (z == *tree) {
		*tree = v;
		v->parent = NULL;
	} else {
		zpi = z == z->parent->succ[0] ? 0 : 1;
		z->parent->succ[zpi] = v;
		v->parent = z->parent;
	}

	z->succ[!i] = v->succ[i];
	v->succ[i]->parent = z;

	y->succ[i] = v->succ[!i];
	v->succ[!i]->parent = y;

	v->succ[i] = z;
	z->parent = v;

	v->succ[!i] = y;
	y->parent = v;
}
void single_rotate(wavl_t **tree, wavl_t *x, bool left)
{
	wavl_t *y;
	int i, yid;

	i = left? 0 : 1;
	y = x->parent;
	// = x->succ[i];
	//printf("tree: %d %d\n", *(int*)(*tree)->data, *tree == y);
	//printf("i:%d y = (%d) (%d, %d), x = (%d) (%d, %d)\n", i,*(int*)y->data, y->succ[0] == NULL ? -1 : *(int*)y->succ[0]->data, y->succ[1] == NULL ? -1 : *(int*)y->succ[1]->data, *(int*)x->data, x->succ[0] == NULL ? -1 : *(int*)x->succ[0]->data, x->succ[1] == NULL ? -1 : *(int*)x->succ[1]->data);
	y->succ[!i] = x->succ[i];
	if (y->succ[!i] != NULL)
		y->succ[!i]->parent = y;

	x->succ[i] = y;
	if (y->parent == NULL) {
		*tree = x;
		x->parent = NULL;
	} else {
		yid = y->parent->succ[0] == y ? 0 : 1;
		y->parent->succ[yid] = x;
		x->parent = y->parent;
	}
	y->parent = x;
	//printf("tree = (%d) (%d) (%d)\n", *(int*)(*tree)->data, *(int*)(*tree)->succ[0]->data, *(int*)(*tree)->succ[1]->data);
	//exit(-1);

}

static bool is_0_1_node(wavl_t *x)
{
	bool par, parp, pars;

	par = get_par(x);
	parp = get_par(x->parent);
	pars = get_par(sibling(x));
	return (!par && !parp && pars) || (par && parp && !pars);
}

static bool is_0_2_node(wavl_t *x)
{
	bool par, parp, pars;

	par = get_par(x);
	parp = get_par(x->parent);
	pars = get_par(sibling(x));
	return (par && parp && pars) || (!par && !parp && !pars);
}
bool wavl_put(wavl_t **tree, void *data, int (*cmp)(void*, void*))
{
	wavl_t **n;
	wavl_t *p;
	wavl_t *x, *y, *z, *px;
	bool should_rebalance;
	int d;
	int i;

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
	should_rebalance = is_leaf(p);

	*n = node(data);
	(*n)->parent = p;
	if (*(int*) (*n)->data == 3)
		printf("tree = (%d) (%d) (%d)\n", *(int*)(*tree)->data, *(int*)(*tree)->succ[1]->data, *(int*)(*tree)->succ[1]->succ[1]->data);
	if (!should_rebalance)
		return true;

	px = p;
	x = NULL;
	do {
		prom(px);
		x = px;
		px = x->parent;

		if (px == NULL)
			return true;
	} while(is_0_1_node(x));

	if (!is_0_2_node(x))
		return true;

	z = x->parent;
	i = x == z->succ[0] ? 0 : 1;
	y = x->succ[!i];
	//printf("left: %s\n",i?"true":"false");
	if (y == NULL || get_par(y) == get_par(x)) {
		single_rotate(tree, x, i);
		dem(z);
	} else {
		double_rotate(tree, x, i);
		prom(y);
		dem(x);
		dem(z);
	}

	return true;
}

static wavl_t *xwavl_get(wavl_t **tree, void *data, int (*cmp)(void*,void*))
{
	wavl_t *n;
	int d;

	n = *tree;
	while (n != NULL) {
		d = cmp(data, n->data);
		if (d == 0)
			return n;
		n = n->succ[d < 0 ? 0 : 1];
	}
	return NULL;
}

void *wavl_get(wavl_t **tree, void *data, int (*cmp)(void*,void*))
{
	wavl_t *n;
	n = xwavl_get(tree, data, cmp);
	if (n == NULL)
		return NULL;
	return n->data;
}

void *wavl_take(wavl_t **tree, void *data, int (*cmp)(void*,void*));

static void print_tree(FILE *out, wavl_t *n, char *pre, bool final, void (*tostr)(char *buff, void*data))
{
	char buff[4096];
	char str[4096];

	fprintf(out, "%s",pre);
	strcpy(buff, pre);
	str[0] = '\0';
	tostr(str, n->data);
	if(final)
		fprintf(out, "└───");
	else
		fprintf(out, "├───");

	if (is_leaf(n))
		fprintf(out, "─\033[1m%s (%d)\033[0m\n", str, n->r);
	else
		fprintf(out, "┬\033[1m%s (%d)\033[0m\n", str, n->r);
	if(final)
		strcat(buff, "    ");
	else
		strcat(buff, "│   ");
	if (n->succ[0] != NULL)
		print_tree(out, n->succ[0], buff, n->succ[1] == NULL, tostr);
	if (n->succ[1] != NULL)
		print_tree(out, n->succ[1], buff, true, tostr);
}
void wavl_print(FILE *out, wavl_t *tree, void (*tostr)(char *buff, void*data))
{
	if (tree == NULL)
		return;
	print_tree(out, tree, "", true, tostr);
}