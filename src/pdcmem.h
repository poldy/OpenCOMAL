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

#include <stddef.h>
#include "compat_cdefs.h"

#define NR_FIXED_POOLS		4

#define PARSE_POOL		0
#define RUN_POOL		1
#define MISC_POOL		2


#define NRCPOOLS		2
#define INT_CPOOL		0
#define FLOAT_CPOOL		1

#define GETCORE(p,a) (a *)mem_alloc((p),sizeof(a))

/** Tracking info before the start of each allocated block */
struct mem_block {
	struct mem_block *next;
	struct mem_block *prev;
	int marker;
#ifndef NDEBUG
	long size;
#endif
	struct mem_pool *pool;
};

/** A pool of related allocated memory blocks */
struct mem_pool {
#ifndef NDEBUG
	long size;
#endif
	struct mem_block *root;
	int id;
};

/** Initialise all pools & cells */
extern void mem_init(void);

/** Free all allocated pools & cells */
extern void mem_tini(void);

/** Allocate a cell */
extern __malloc void *cell_alloc(unsigned int pool);

/**
 * Private interface to allocate a memory block.
 * @see STR_ALLOC_PRIVATE
 */
extern __malloc void *mem_alloc_private(struct mem_pool *pool, size_t size);

/** Allocate a memory block in a pool */
extern __malloc void *mem_alloc(unsigned int pool, size_t size);

/** Change the size of a memory block */
extern void *mem_realloc(void *block, long newsize);

/** Free a cell */
extern void cell_free(void *m);

/** Free a memory block */
extern void *mem_free(void *m);

/** Free a cell pool */
extern void cell_freepool(unsigned int pool);

/* Free a pool of memory blocks */
extern void mem_freepool(unsigned int pool);

/**
 * Private interface to free a memory block.
 * @see prog_new
 */
extern void mem_freepool_private(struct mem_pool *pool);

/** Move a memory block from one pool to another */
extern void mem_shiftmem(unsigned int frompool, struct mem_pool *topool);

#ifndef NDEBUG
/** Print the size of all pools */
extern void mem_debug(void);
#endif

/** Allocate a new memory pool */
extern struct mem_pool *pool_new(void);

#endif
