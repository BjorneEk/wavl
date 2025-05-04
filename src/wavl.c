
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wavl.h"

#define TAG_MASK (0x1)
#define PTR_MASK (~(uintptr_t)TAG_MASK)

static void set_ptr(wavl_t *n, void *ptr, bool tag)
{
	n->ptr_with_par = ((uintptr_t)ptr & PTR_MASK) | (tag & TAG_MASK);
}


static wavl_t *node(void *data)
{
	wavl_t *res;

	res = calloc(1,sizeof(wavl_t));
	set_ptr(res, data, false);
	return res;
}

static bool get_par(wavl_t *n)
{
	return n == 0 ? true : n->ptr_with_par & TAG_MASK;
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
		n->ptr_with_par ^= 1;
}

static void dem(wavl_t *n)
{
	if (n != NULL)
		n->ptr_with_par ^= 1;
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
	if (v->succ[i] != NULL)
		v->succ[i]->parent = z;

	y->succ[i] = v->succ[!i];
	if (v->succ[!i] != NULL)
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
		d = cmp(data, (void*)((*n)->ptr_with_par & PTR_MASK));
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
void wavl_free(wavl_t **tree)
{
	int i;

	if (*tree == NULL)
		return;
	for (i = 0; i < 2; ++i)
		wavl_free(&(*tree)->succ[i]);
	free(*tree);
	*tree = NULL;
}

static wavl_t *xwavl_get(wavl_t **tree, void *data, int (*cmp)(void*,void*))
{
	wavl_t *n;
	int d;

	n = *tree;
	while (n != NULL) {
		d = cmp(data, (void*)(n->ptr_with_par & PTR_MASK));
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
	return (void*)(n->ptr_with_par & PTR_MASK);
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
	new->ptr_with_par = (new->ptr_with_par & PTR_MASK) | (old->ptr_with_par & TAG_MASK);
	old->parent = NULL;
}
static bool is_2_2_node(wavl_t *y)
{
	return get_par(y) == get_par(y->succ[0]) && get_par(y) == get_par(y->succ[1]);
}
/*
static void print_node(wavl_t *n)
{
	printf("%d(%d, %d) (%d)\n",
		n != NULL ? *(int*)n->data : -1,
		n != NULL ? n->succ[0] != NULL ? *(int*)n->succ[0]->data : -1 : -1,
		n != NULL ? n->succ[1] != NULL ? *(int*)n->succ[1]->data : -1 : -1, get_par(n));
}
*/

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
	//printf("x ");print_node(x);
	//printf("xp ");print_node(xp);
	do {
		xpp = xp->parent;

		new_3_node = xpp != NULL && is_2_child(xp);

		y = xp->succ[xp->succ[0] == x ? 1 : 0];
		//printf("y ");print_node(y);
		if (is_2_child(y)) {
			dem(xp);
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
	left = y == z->succ[0] ? 0 : 1;
	w = y->succ[left];

	//printf("rebalance 3 node: y ");
	//print_node(y);
	//printf("x ");print_node(x);
	//printf("z ");print_node(z);
	//printf("y ");print_node(y);
	//printf("w ");print_node(w);

	if (get_par(w) != get_par(y) && z ) {
		// 1 child of y
		//printf("single %s\n", left ? "left" : "right");
		single_rotate(tree, y, left);
		prom(y);
		dem(z);
		if (is_leaf(z))
			dem(z);
	} else {
		// 2 child of y
		//printf("double %s\n", left ? "left" : "right");
		double_rotate(tree, y, left);
		dem(y);
		//douple_prom(v); do notning
		//douple_dem(z); do nothing
	}
}

/*
static void strint(char *buff, void *ip)
{
	if (*(int*)ip < 0)
		sprintf(buff, "%d", *(int*)ip);
	else
		sprintf(buff, " %d", *(int*)ip);
}
*/
static void rebalance_2_2_leaf(wavl_t **tree, wavl_t *yp)
{
	wavl_t *x;
	bool xis_2_child;
	x = yp;

	xis_2_child = is_2_child(x);
	dem(x);
	//printf("rebalance 22 leaf: x ");
	//print_node(yp);
	if (xis_2_child && x->parent != NULL) {
		//printf("tree 2\n");
		//wavl_print(stdout, *tree, strint);
		rebalance_3_child(tree, x, x->parent);
	}
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
	xis_2_child = false;
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

	//printf("tree 1\n");
	//wavl_print(stdout, *tree, strint);
	if (x == *tree && is_leaf(x)) {
		free(n);
		return;
	}

	if (yp != NULL) {
		if (xis_2_child)
			rebalance_3_child(tree, x, yp);
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
	//printf("take %d\n", *(int*)data);
	n = xwavl_get(tree, data, cmp);

	if (n == NULL)
		return NULL;

	res = (void*)(n->ptr_with_par & PTR_MASK);

	if (n == *tree && is_leaf(n)) {
		free(n);
		*tree = NULL;
		return res;
	}


	y = get_swap_node(n);
	remove_node(tree, n, y);
	return res;
}
/*
static bool is_2_c(wavl_t *n, int i)
{
	bool res;
	if (n->parent == NULL)
		return false;
	res = is_2_child(n);
	res = res && !get_par(n->succ[i]);
	return res;
}
*/
#define MAX(A, B) ((A) > (B) ? (A) : (B))
static int max_rank(wavl_t *n)
{
	int i,r, rank;
	if (n == NULL)
		return -1;
	if (is_leaf(n))
		return 1;
	rank = -1;
	for (i = 0; i < 2; ++i) {
		r = max_rank(n->succ[i]);
		rank = MAX(rank, r);
	}
	return rank + 1;

}

static void print_tree(FILE *out, wavl_t *n, char *pre, bool final, bool is_2, void (*tostr)(char *buff, void*data))
{
	//┐
	char buff[4096];
	char str[4096];
	bool is_21 = false;
	bool is_22 = false;
	fprintf(out, "%s",pre);
	strcpy(buff, pre);
	if(final)
		strcat(buff, "    ");
	else
		strcat(buff, "│   ");

	str[0] = '\0';
	tostr(str, (void*)(n->ptr_with_par & PTR_MASK));
	if(final)
		fprintf(out, "└───");
	else
		fprintf(out, "├───");
	/*
	1.	non-root node is an i-child if its rank difference is i
		i = r(n->parent) − r(x).

	2. 	A node is i,j if its left and right children have
		rank differences i and j, respectively
	*/

	if (is_leaf(n))
		fprintf(out, "%s\033[1m\033[34m%s\033[0m (%d)\033[0m\n",is_2 ? "─────" : "─", str, (int)(n->ptr_with_par & TAG_MASK));
	else
		fprintf(out, "%s\033[1m\033[34m%s\033[0m (%d)\033[0m\n",is_2 ? "────┬" : "┬", str, (int)(n->ptr_with_par & TAG_MASK));

	if (is_2_child(n->succ[0]))
		is_21 = true;
	if (is_2_child(n->succ[1]))
		is_22 = true;
	//if (is_2_2_node(n)) {
	//	is_22 = true;
	//	is_21 = true;
	//}
	if(is_2)
		strcat(buff, "    ");

	if (n->succ[0] != NULL)
		print_tree(out, n->succ[0], buff, n->succ[1] == NULL,is_21, tostr);
	if (n->succ[1] != NULL)
		print_tree(out, n->succ[1], buff, true,is_22, tostr);
}
void wavl_print(FILE *out, wavl_t *tree, void (*tostr)(char *buff, void*data))
{
	if (tree == NULL)
		return;
	printf("H: %d\n", max_rank(tree));
	print_tree(out, tree, "", true, false, tostr);
}
