#ifndef _WAVL_H_
#define _WAVL_H_

#include "dla.h"
#include "types.h"
#include <stdio.h>
typedef struct wawlnode {
	bool	p : 1;
	u32_t	d : 31;
	u32_t	succ[2];
	u32_t	parent;
} wavlnode_t;

typedef struct owavl {
	u32_t root;
	dla_t nodes;
	dla_t data;
	dla_t free_n;
	dla_t free_d;
} owavl_t;
void owavl_init(owavl_t *tree);
bool owavl_put(owavl_t *tree, void *data, int (*cmp)(void*, void*));
void *owavl_take(owavl_t *tree, void *data, int (*cmp)(void*,void*));
void owavl_print(FILE *out, owavl_t *tree, void (*tostr)(char *,void*));
void *owavl_get(owavl_t *tree, void *data, int (*cmp)(void*,void*));

#endif