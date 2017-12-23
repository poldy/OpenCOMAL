/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/** @file
 * OpenComal memory management header file
 */

#ifndef PDCMEM_H
#define PDCMEM_H

#define NR_FIXED_POOLS		4

#define PARSE_POOL		0
#define RUN_POOL		1
#define MISC_POOL		2


#define NRCPOOLS		2
#define INT_CPOOL		0
#define FLOAT_CPOOL		1

#define GETCORE(p,a) (a *)mem_alloc((p),sizeof(a))

struct mem_block {
	struct mem_block *next;
	struct mem_block *prev;
	int marker;
	long size;
	struct mem_pool *pool;
};

struct mem_pool {
	long size;
	struct mem_block *root;
	int id;
};

extern void mem_init(void);
extern void mem_tini(void);
extern void *cell_alloc(unsigned int pool);
extern void *mem_alloc_private(struct mem_pool *pool, long size);
extern void *mem_alloc(unsigned int pool, long size);
extern void *mem_realloc(void *block, long newsize);
extern void cell_free(void *m);
extern void *mem_free(void *m);
extern void cell_freepool(unsigned int pool);
extern void mem_freepool(unsigned int pool);
extern void mem_freepool_private(struct mem_pool *pool);
extern void mem_shiftmem(unsigned int frompool, struct mem_pool *topool);
extern void mem_debug(int level);
extern struct mem_pool *pool_new();

#endif
