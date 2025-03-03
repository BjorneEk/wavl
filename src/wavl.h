#ifndef _WAVL_H_
#define _WAVL_H_

#include "dla.h"
#include "types.h"
#include <stdio.h>

typedef struct wavl {
	struct wavl	*succ[2];
	struct wavl	*parent;
	uintptr_t	ptr_with_par;
} wavl_t;


bool wavl_put(wavl_t **tree, void *data, int (*cmp)(void*, void*));
void *wavl_get(wavl_t **tree, void *data, int (*cmp)(void*, void*));
void *wavl_take(wavl_t **tree, void *data, int (*cmp)(void*,void*));
void wavl_print(FILE *out, wavl_t *tree, void (*tostr)(char *,void*));

void double_rotate(wavl_t **tree, wavl_t *y, bool left);
void single_rotate(wavl_t **tree, wavl_t *x, bool left);
void wavl_free(wavl_t **tree);
#endif