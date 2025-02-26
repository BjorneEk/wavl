
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
	wavl_t *z,
		*v;
	int	zpi,
		i;

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
	int	i,
		yid;

	i = left? 0 : 1;
	y = x->parent;

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
}

static bool is_0_1_node(wavl_t *x)
{
	bool	par,
		parp,
		pars;

	par = get_par(x);
	parp = (x != NULL ? get_par(x->parent) : true);
	pars = get_par(sibling(x));
	return (!par && !parp && pars) || (par && parp && !pars);
}

static bool is_0_2_node(wavl_t *x)
{
	bool	par,
		parp,
		pars;

	par = get_par(x);
	parp = (x != NULL ? get_par(x->parent) : true);
	pars = get_par(sibling(x));
	return (par && parp && pars) || (!par && !parp && !pars);
}

static bool is_2_child(wavl_t *y)
{
	return get_par(y) == (y != NULL ? get_par(y->parent) : true);
}
bool wavl_put(wavl_t **tree, void *data, int (*cmp)(void*, void*))
{
	wavl_t **n,
		*p,
		*x,
		*y,
		*z,
		*px;
	int	d,
		i;
	bool unbalanced;


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
	unbalanced = is_leaf(p);

	*n = node(data);
	(*n)->parent = p;

	if (!unbalanced)
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

	if (y == NULL || is_2_child(y)) {
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

static void swap_in(wavl_t **tree, wavl_t *new, wavl_t *old)
{
	wavl_t *succ[2],
		*nsucc,
		*p;
	int i;

	succ[0] = old->succ[0];
	succ[1] = old->succ[1];
	p = old->parent;

	new->parent = p;

	if (p != NULL)
		p->succ[p->succ[0] == old ? 0 : 1] = new;
	else
		*tree = new;

	for (i = 1; i >= 0; --i) {
		new->succ[i] = succ[i];
		if (new->succ[i] != NULL) {
			nsucc = new->succ[i];
			nsucc->parent = new;
		}
		old->succ[i] = NULL;
	}
	old->succ[0] = NULL;
	new->r = old->r;
	old->parent = NULL;
}
static bool is_2_2_node(wavl_t *y)
{
	return get_par(y) == get_par(y->succ[0]) && get_par(y) == get_par(y->succ[1]);
}

static void print_node(wavl_t *n)
{
	printf("%d(%d, %d)\n",
		n != NULL ? *(int*)n->data : -1,
		n != NULL ? n->succ[0] != NULL ? *(int*)n->succ[0]->data : -1 : -1,
		n != NULL ? n->succ[1] != NULL ? *(int*)n->succ[1]->data : -1 : -1);
}

static void rebalance_3_child(wavl_t **tree, wavl_t *n, wavl_t *np)
{
	wavl_t	*x,
		*xp,
		*xpp,
		*y,
		*z,
		*w;
	bool	new_3_node,
		done;
	int left;

	new_3_node = false;
	done = true;

	x = n;
	xp = np;
	printf("n ");print_node(n);
	printf("np ");print_node(np);
	do {
		xpp = xp->parent;

		new_3_node = xpp != NULL && is_2_child(xp);

		y = xp->succ[xp->succ[0] == x ? 1 : 0];

		if (is_2_child(y)) {
			dem(x);
		} else if (is_2_2_node(y)) {
			dem(xp);
			dem(y);
		} else {
			done = false;
			break;
		}
		x = xp;
		xp = xpp;
	} while (xp != NULL && new_3_node);

	if (done)
		return;

	z = xp;
	left = x == xp->succ[0] ? 1 : 0;
	w = y->succ[left];
	printf("x ");print_node(x);
	printf("z ");print_node(z);
	printf("y ");print_node(y);
	printf("w ");print_node(w);
	if (get_par(w) != get_par(y)) {
		// 1 child of y
		single_rotate(tree, y, left);
		prom(y);
		dem(z);
		if (is_leaf(z))
			dem(z);
	} else {
		// 2 child of y
		double_rotate(tree, y, left);
		dem(y);
		//douple_prom(v); do notning
		//douple_dem(z); do nothing
	}
}

static void rebalance_2_2_leaf(wavl_t **tree, wavl_t *yp)
{
	wavl_t *x;
	bool xis_2_child;
	x = yp;

	xis_2_child = is_2_child(x);
	dem(x);
	if (xis_2_child)
		rebalance_3_child(tree, x, x->parent);
}

static void remove_node(wavl_t **tree, wavl_t *n, wavl_t *y)
{
	wavl_t	*x,
		*yp;
	bool xis_2_child;

	x = y->succ[0] == NULL ? y->succ[1] : y->succ[0];
	if (x != NULL)
		x->parent = y->parent;

	yp = y->parent;
	if (yp == NULL) {
		*tree = x;
	} else {
		xis_2_child = is_2_child(y);
		yp->succ[y == yp->succ[0] ? 0 : 1] = x;
	}

	if (y != n) {
		swap_in(tree, y, n);
		if (n == yp)
			yp = y;
	}

	if (yp != NULL) {
		if (xis_2_child)
			rebalance_3_child(tree, n, yp);
		else if (x == NULL && yp->succ[0] == yp->succ[1])
			rebalance_2_2_leaf(tree, yp);
	}

	free(n);
}

static wavl_t *get_swap_node(wavl_t *n)
{
	wavl_t *y;

	if (n->succ[0] == NULL || n->succ[1] == NULL)
		return n;

	y = n->succ[1];

	while (y->succ[0] != NULL)
		y = y->succ[0];

	return y;
}

void *wavl_take(wavl_t **tree, void *data, int (*cmp)(void*,void*))
{
	wavl_t	*n,
		*y;
	void *res;

	n = xwavl_get(tree, data, cmp);

	if (n == NULL)
		return NULL;

	res = n->data;

	y = get_swap_node(n);
	remove_node(tree, n, y);
	return res;
}

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