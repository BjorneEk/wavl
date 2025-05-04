
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "owavl.h"

DLA_GEN(static, wavlnode_t, nodes, init, getp, new, len, deinit, data)
DLA_GEN(static, u32_t, free, init, len, pop, push, deinit)
//DLA_GEN(static, void*, data, init, push, len, get, getp, deinit, data)

#define MAX_LEN (0x7FFFFFFF)
#ifndef NODEBUG
#define wavl_assert(s) do {	\
	if (!(s)){ fprintf(stderr, "wavl assertion failed for %s (%s: %d)\n", #s, __func__, __LINE__);exit(-1);}	\
	} while(0);
#else
#define wavl_assert(s)
#endif
#define TAG_MASK (0x1)
#define PTR_MASK (~(uintptr_t)TAG_MASK)
static void set_ptr(wavlnode_t *n, void *ptr, bool tag)
{
	n->ptr_with_par = ((uintptr_t)ptr & PTR_MASK) | (tag & TAG_MASK);
}

void owavl_init(owavl_t *t)
{
	nodes_init(&t->nodes, 10);
	free_init(&t->free, 10);
	// allocate first node as empry so index 0 is the NULL representation
	nodes_new(&t->nodes);
	t->root = 0;
}

inline static void *get_data(owavl_t *t, u32_t n)
{

	return (void *)(nodes_data(&t->nodes)[n].ptr_with_par & PTR_MASK);
}

static inline u32_t *succ(owavl_t *t, u32_t n, int i)
{
	return &nodes_data(&t->nodes)[n].succ[i];
}

static inline u32_t *parent(owavl_t *t, u32_t n)
{
	return &nodes_data(&t->nodes)[n].parent;
}
static void free_node(owavl_t *t, u32_t n)
{
	free_push(&t->free, n);
}

static u32_t new_node(owavl_t *t, void *data)
{
	u32_t res;
	wavlnode_t *n;

	if (free_len(&t->free) > 0) {
		res = free_pop(&t->free);
		n = nodes_getp(&t->nodes, res);
	} else {
		n = nodes_new(&t->nodes);
		res = nodes_len(&t->nodes) - 1;
	}

	set_ptr(n, data, false);
	n->succ[1] = 0;
	n->succ[0] = 0;
	n->parent = 0;
	return res;
}

static bool get_par(owavl_t *t, u32_t n)
{
	return n == 0 ? true : nodes_data(&t->nodes)[n].ptr_with_par & TAG_MASK;
}

static u32_t sibling(owavl_t *t, u32_t n)
{
	wavlnode_t	*np,
			*pp;
	if (n == 0)
		return 0;
	np = &nodes_data(&t->nodes)[n];

	if (np->parent == 0)
		return 0;
	pp = &nodes_data(&t->nodes)[np->parent];

	return pp->succ[n == pp->succ[0] ? 1 : 0];
}

static bool is_leaf(owavl_t *t, u32_t n)
{
	wavlnode_t *np;

	wavl_assert(n != 0);
	np = &nodes_data(&t->nodes)[n];
	return np->succ[0] == 0 && np->succ[1] == 0;
}

static void prom(owavl_t *t, u32_t n)
{
	wavlnode_t *np;
	if (n != 0) {
		np = &nodes_data(&t->nodes)[n];
		np->ptr_with_par ^= 1;
	}
}

static void dem(owavl_t *t, u32_t n)
{
	wavlnode_t *np;
	if (n != 0) {
		np = &nodes_data(&t->nodes)[n];
		np->ptr_with_par ^= 1;
	}
}

static void odouble_rotate(owavl_t *t, u32_t y, bool left)
{
	u32_t	z,
		v;
	wavlnode_t	*yp,
			*zp,
			*vp,
			*zpp;
	int	zpi,
		i;

	wavl_assert(y != 0);

	i = left ? 0 : 1;
	yp = &nodes_data(&t->nodes)[y];
	z = yp->parent;
	v = yp->succ[i];

	wavl_assert(z != 0);
	wavl_assert(v != 0);
	zp = &nodes_data(&t->nodes)[z];
	vp = &nodes_data(&t->nodes)[v];


	if (z == t->root) {
		t->root = v;
		vp->parent = 0;
	} else {
		zpp = &nodes_data(&t->nodes)[zp->parent];
		zpi = z == zpp->succ[0] ? 0 : 1;
		zpp->succ[zpi] = v;
		vp->parent = zp->parent;
	}

	zp->succ[!i] = vp->succ[i];
	if (vp->succ[i] != 0) {
		nodes_data(&t->nodes)[vp->succ[i]].parent = z;
	}

	yp->succ[i] = vp->succ[!i];
	if (vp->succ[!i] != 0)
		nodes_data(&t->nodes)[vp->succ[!i]].parent = y;

	vp->succ[i] = z;
	zp->parent = v;

	vp->succ[!i] = y;
	yp->parent = v;
}
static void osingle_rotate(owavl_t *t, u32_t x, bool left)
{
	u32_t y;
	wavlnode_t	*xp,
			*yp,
			*ypp;
	int	i,
		yid;

	i = left? 0 : 1;
	wavl_assert(x != 0);
	xp = &nodes_data(&t->nodes)[x];
	y = xp->parent;
	wavl_assert(y != 0);
	yp = &nodes_data(&t->nodes)[y];

	yp->succ[!i] = xp->succ[i];
	if (yp->succ[!i] != 0)
		nodes_data(&t->nodes)[yp->succ[!i]].parent = y;

	xp->succ[i] = y;
	if (yp->parent == 0) {
		t->root = x;
		xp->parent = 0;
	} else {
		ypp = &nodes_data(&t->nodes)[yp->parent];
		yid = ypp->succ[0] == y ? 0 : 1;
		ypp->succ[yid] = x;
		xp->parent = yp->parent;
	}
	yp->parent = x;
}

static bool is_0_1_node(owavl_t *t, u32_t x)
{
	wavlnode_t *xp;
	bool	par,
		parp,
		pars;

	par = get_par(t, x);
	xp = (x != 0) ? &nodes_data(&t->nodes)[x] : NULL;
	if (xp == NULL)
		parp = true;
	else {
		parp = get_par(t, xp->parent);
	}
	pars = get_par(t, sibling(t, x));
	return (!par && !parp && pars) || (par && parp && !pars);
}

static bool is_0_2_node(owavl_t *t, u32_t x)
{
	wavlnode_t *xp;
	bool	par,
		parp,
		pars;

	par = get_par(t, x);
	xp = (x != 0) ? &nodes_data(&t->nodes)[x] : NULL;
	if (xp == NULL)
		parp = true;
	else {
		parp = get_par(t, xp->parent);
	}
	pars = get_par(t, sibling(t, x));
	return (par && parp && pars) || (!par && !parp && !pars);
}

static bool is_2_child(owavl_t *t, u32_t x)
{
	wavlnode_t *xp;
	bool	parp;
	xp = (x != 0) ? &nodes_data(&t->nodes)[x] : NULL;
	parp = xp != NULL ? get_par(t, xp->parent) : true;
	return get_par(t, x) == parp;
}
/*
static void print_node(owavl_t *t, u32_t n)
{
	printf("%d(%d, %d) (%d)\n",
		n != 0 ? *(int*)get_data(t, n) : -1,
		n != 0 ? (*succ(t, n, 0) != 0 ? *(int*)get_data(t, *succ(t, n, 0)) : -1) : -1,
		n != 0 ? (*succ(t, n, 1) != 0 ? *(int*)get_data(t, *succ(t, n, 1)) : -1) : -1, get_par(t, n));
}
*/
bool owavl_put(owavl_t *t, void *data, int (*cmp)(void*, void*))
{
	u32_t n,
		p,
		px,
		x,
		z,
		y;
	int	d,
		i,
		wsucc;
	bool unbalanced;
	wavlnode_t *np;

	if (t->root == 0) {
		t->root = new_node(t, data);
		return true;
	}


	n = t->root;
	np = &nodes_data(&t->nodes)[n];
	p = 0;

	while (n != 0) {
		d = cmp(data, (void*)(np->ptr_with_par & PTR_MASK));
		p = n;
		n = np->succ[d < 0 ? 0 : 1];
		if (d == 0)
			return false;
		np = &nodes_data(&t->nodes)[n];
	}
	unbalanced = is_leaf(t, p);
	wsucc = cmp(data, get_data(t, p)) < 0 ? 0 : 1;
	*succ(t, p, wsucc) = new_node(t, data);
	n = *succ(t, p, wsucc);
	*parent(t, n) = p;


	if (!unbalanced)
		return true;

	px = p;
	x = 0;
	do {
		prom(t, px);
		x = px;
		px = *parent(t, x);

		if (px == 0)
			return true;
	} while(is_0_1_node(t, x));

	if (!is_0_2_node(t, x))
		return true;

	z = *parent(t, x);
	i = x == *succ(t, z, 0) ? 0 : 1;
	y = *succ(t, x, !i);

	if (y == 0 || is_2_child(t, y)) {
		osingle_rotate(t, x, i);
		dem(t, z);
	} else {
		odouble_rotate(t, x, i);
		prom(t, y);
		dem(t, x);
		dem(t, z);
	}

	return true;
}

static u32_t xwavl_get(owavl_t *t, void *data, int (*cmp)(void*,void*))
{
	u32_t n;
	int d;
	wavlnode_t *nodes;

	nodes = t->nodes.data;
	n = t->root;
	while (n != 0) {
		d = cmp(data, (void*)(nodes[n].ptr_with_par & PTR_MASK));
		if (d == 0)
			return n;
		n = nodes[n].succ[d < 0 ? 0 : 1];
	}
	return 0;
}

void *owavl_get(owavl_t *t, void *data, int (*cmp)(void*,void*))
{
	u32_t n;
	int d;
	void *p;
	wavlnode_t *nodes;

	nodes = t->nodes.data;
	n = t->root;
	while (n != 0) {
		p = (void*)(nodes[n].ptr_with_par & PTR_MASK);
		d = cmp(data, p);
		if (d == 0)
			return p;
		n = nodes[n].succ[d < 0 ? 0 : 1];
	}
	return NULL;
}

void owavl_free(owavl_t *t)
{
	nodes_deinit(&t->nodes);
	free_deinit(&t->free);
	t->root = 0;
}

static void swap_in(owavl_t *t, u32_t new, u32_t old)
{
	u32_t xsucc[2],
		nsucc,
		p;
	wavlnode_t *op, *ps, *np;
	int i;

	wavl_assert(old != 0);
	wavl_assert(new != 0);

	np = nodes_getp(&t->nodes, new);
	op = nodes_getp(&t->nodes, old);

	xsucc[0] = *succ(t, old, 0);
	xsucc[1] = *succ(t, old, 1);
	p = *parent(t, old);

	*parent(t, new) = p;

	if (p != 0) {
		ps = nodes_getp(&t->nodes, p);
		ps->succ[old == ps->succ[0] ? 0 : 1] = new;
	} else
		t->root = new;


	for (i = 1; i >= 0; --i) {
		np->succ[i] = xsucc[i];
		if (np->succ[i] != 0) {
			nsucc = np->succ[i];
			*parent(t, nsucc) = new;
		}
		op->succ[i] = 0;
	}
	op->succ[0] = 0;
	np->ptr_with_par = (np->ptr_with_par & PTR_MASK) | (op->ptr_with_par & TAG_MASK);
	op->parent = 0;
}
static bool is_2_2_node(owavl_t *t, u32_t y)
{
	bool yp, lp, rp;
	yp = get_par(t, y);
	lp = y == 0 ? true : get_par(t, *succ(t, y, 0));
	rp = y == 0 ? true : get_par(t, *succ(t, y, 1));
	return (yp && lp && rp) || (!yp && !lp && !rp);
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
static void rebalance_3_child(owavl_t *t, u32_t n, u32_t p)
{
	u32_t	x,
		xp,
		xpp,
		y,
		z,
		w;
	bool	new_3_node,
		done;
	int left;

	new_3_node = false;
	done = true;

	x = n;
	xp = p;
	//printf("x ");print_node(x);
	//printf("xp ");print_node(xp);
	do {
		xpp = *parent(t,xp);

		new_3_node = xpp != 0 && is_2_child(t, xp);

		y = *succ(t, xp, *succ(t, xp, 0) == x ? 1 : 0);
		//printf("y ");print_node(y);
		if (is_2_child(t, y)) {
			dem(t, xp);
		} else if (is_2_2_node(t, y)) {
			dem(t, xp);
			dem(t, y);
		} else {
			done = false;
			break;
		}
		x = xp;
		xp = xpp;
	} while (xp != 0 && new_3_node);

	if (done)
		return;

	z = xp;
	left = y == *succ(t, z, 0) ? 0 : 1;
	w = *succ(t, y, left);

	//printf("rebalance 3 node: y ");
	//print_node(y);
	//printf("x ");print_node(x);
	//printf("z ");print_node(z);
	//printf("y ");print_node(y);
	//printf("w ");print_node(w);

	if (get_par(t,w) != get_par(t, y) && z ) {
		// 1 child of y
		//printf("single %s\n", left ? "left" : "right");
		osingle_rotate(t, y, left);
		prom(t, y);
		dem(t, z);
		if (is_leaf(t, z))
			dem(t, z);
	} else {
		// 2 child of y
		//printf("double %s\n", left ? "left" : "right");
		odouble_rotate(t, y, left);
		dem(t, y);
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
static void rebalance_2_2_leaf(owavl_t *t, u32_t yp)
{
	u32_t x;
	bool xis_2_child;
	x = yp;

	xis_2_child = is_2_child(t, x);
	dem(t, x);
	//printf("rebalance 22 leaf: x ");
	//print_node(yp);
	if (xis_2_child && *parent(t, x) != 0) {
		//printf("tree 2\n");
		//wavl_print(stdout, *tree, strint);
		rebalance_3_child(t, x, *parent(t, x));
	}
}

static void remove_node(owavl_t *t, u32_t n, u32_t y)
{
	u32_t	x,
		yp;
	bool xis_2_child;


	x = *succ(t, y, 0) == 0 ? *succ(t, y, 1) : *succ(t, y, 0);
	if (x != 0)
		*parent(t, x) = *parent(t, y);

	yp = *parent(t, y);
	xis_2_child = false;
	if (yp == 0) {
		t->root = x;
	} else {
		xis_2_child = is_2_child(t, y);
		*succ(t, yp, *succ(t, yp, 0) == y ? 0 : 1) = x;
	}

	if (y != n) {
		swap_in(t, y, n);
		if (n == yp)
			yp = y;
	}

	if (x == t->root && is_leaf(t,x)) {
		free_node(t, n);
		return;
	}

	if (yp != 0) {
		if (xis_2_child)
			rebalance_3_child(t, x, yp);
		else if (x == 0 && *succ(t, yp, 0) == *succ(t, yp, 1))
			rebalance_2_2_leaf(t, yp);
	}

	free_node(t, n);
}

static u32_t get_swap_node(owavl_t *t, u32_t n)
{
	u32_t y;

	if (*succ(t, n, 0) == 0 || *succ(t, n, 1) == 0)
		return n;

	y = *succ(t, n, 1);

	while (*succ(t, y, 0) != 0)
		y = *succ(t, y, 0);

	return y;
}

void *owavl_take(owavl_t *t, void *data, int (*cmp)(void*,void*))
{
	u32_t	n,
		y;
	void *res;
	//printf("take %d\n", *(int*)data);
	if (t->root == 0)
		return NULL;
	n = xwavl_get(t, data, cmp);

	if (n == 0)
		return NULL;

	res = get_data(t, n);

	if (n == t->root && is_leaf(t, n)) {
		free_node(t, n);
		t->root = 0;
		return res;
	}


	y = get_swap_node(t, n);
	remove_node(t, n, y);
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

static void print_tree(FILE *out, owavl_t *t, u32_t n, char *pre, bool final, bool is_2, void (*tostr)(char *buff, void*data))
{
	//┐
	char buff[4096];
	char str[4096];
	bool is_21 = false;
	bool is_22 = false;
	if (n == 0)
		return;
	fprintf(out, "%s",pre);
	strcpy(buff, pre);
	if(final)
		strcat(buff, "    ");
	else
		strcat(buff, "│   ");

	str[0] = '\0';
	tostr(str, get_data(t, n));
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

	if (is_leaf(t, n))
		fprintf(out, "%s\033[1m\033[34m%s\033[0m (%d)\033[0m\n",is_2 ? "─────" : "─", str, get_par(t,n));
	else
		fprintf(out, "%s\033[1m\033[34m%s\033[0m (%d)\033[0m\n",is_2 ? "────┬" : "┬", str, get_par(t,n));

	if (is_2_child(t, *succ(t, n, 0)))
		is_21 = true;
	if (is_2_child(t, *succ(t, n, 1)))
		is_22 = true;
	//if (is_2_2_node(n)) {
	//	is_22 = true;
	//	is_21 = true;
	//}
	if(is_2)
		strcat(buff, "    ");

	if (*succ(t, n, 0) != 0)
		print_tree(out, t, *succ(t, n, 0), buff, *succ(t, n, 1) == 0,is_21, tostr);
	if (*succ(t, n, 1) != 0)
		print_tree(out, t, *succ(t, n, 1), buff, true,is_22, tostr);
}
void owavl_print(FILE *out, owavl_t *tree, void (*tostr)(char *buff, void*data))
{
	if (tree == NULL)
		return;
	print_tree(out, tree, tree->root, "", true, false, tostr);
}
