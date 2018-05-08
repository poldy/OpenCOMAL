/*
 * OpenComal -- a free Comal implementation
 *
 * This file is part of the OpenComal package.
 * (c) Copyright 1992-2002 Jos Visser <josv@osp.nl>
 *
 * The OpenComal package is covered by the GNU General Public
 * License. See doc/LICENSE for more information.
 */

/* OpenComal Memory Management */

#define _XOPEN_SOURCE 700

#include "mem.h"
#include "pdcglob.h"
#include "pdcexec.h"
#include "pdcmisc.h"
#include "pdcmem.h"

#define MEM_MARKER 	(0x2468)
#define CELL_MARKER	0xab00
#define CELL_IN_MEM	0xacf3
#define CELL_FULL	0xffff
#define CELL_POOLSIZE	40

PRIVATE int cell_size[NRCPOOLS] = { sizeof(long), sizeof(double) };

typedef struct a_cell {
	union {
		struct a_cell *next;
		unsigned marker;
	} c;
} CELL;

typedef struct {
	CELL *addr;
	CELL *root;
} CELL_HDR;

static inline CELL *
CELL_ADDR(CELL_HDR *h, unsigned int p, unsigned int i)
{
	return (CELL *)((char *)h->addr + i * (sizeof(CELL) + cell_size[p]));
}
static inline long
CELL_SIZE(unsigned int p)
{
	return (sizeof(CELL) + cell_size[p]);
}

PRIVATE CELL_HDR *cell_hdr[NRCPOOLS];
PRIVATE struct mem_pool mem_pool[NR_FIXED_POOLS];
PRIVATE int poolcount = 0;

PRIVATE void cell_init(unsigned pool)
{
	CELL_HDR *c = (CELL_HDR *)mem_alloc(MISC_POOL, sizeof(CELL_HDR));

	c->addr = (CELL *)mem_alloc(MISC_POOL, CELL_POOLSIZE * CELL_SIZE(pool));

	cell_hdr[pool] = c;
	cell_freepool(pool);
}

PRIVATE void pool_init(struct mem_pool *pool)
{
	pool->id = poolcount;
	poolcount++;
#ifndef NDEBUG
	pool->size = 0;
#endif
	pool->root = NULL;
}

PUBLIC void mem_init(void)
{
	int i;

	for (i = 0; i < NR_FIXED_POOLS; i++)
		pool_init(&mem_pool[i]);

	for (i = 0; i < NRCPOOLS; i++)
		cell_init(i);
}


PRIVATE void cell_tini(unsigned pool)
{
	mem_free(cell_hdr[pool]->addr);
	cell_hdr[pool]->addr = NULL;
	mem_free(cell_hdr[pool]);
	cell_hdr[pool] = NULL;
}


PUBLIC void mem_tini(void)
{
	int i;

	for (i = 0; i < NRCPOOLS; i++)
		cell_tini(i);

	for (i = 0; i < NR_FIXED_POOLS; i++)
		mem_freepool(i);
}


PUBLIC void *cell_alloc(unsigned pool)
{
	CELL_HDR *c = cell_hdr[pool];
	CELL *cell;

	DBG_PRINTF(false, "CELL alloc pool %d ", pool);

	if (c->root != (CELL *) CELL_FULL) {
		DBG_PRINTF(true, "handing out cell @ %p",
			  c->root);

		cell = c->root;
		c->root = cell->c.next;
		cell->c.marker = CELL_MARKER + pool;
	} else {
		DBG_PRINTF(true, " handing out from heap");

		cell = (CELL *)mem_alloc(RUN_POOL, CELL_SIZE(pool));
		cell->c.marker = CELL_IN_MEM;
	}

	return ++cell;
}

PUBLIC void *mem_alloc(unsigned pool, size_t size)
{
	if (pool >= NR_FIXED_POOLS)
		fatal("Invalid pool number in mem_alloc()");

	return mem_alloc_private(&mem_pool[pool], size);
}

PUBLIC void *mem_alloc_private(struct mem_pool *pool, size_t size)
{
	struct mem_block *p;

	DBG_PRINTF(false,
		  "Mem_alloc block in pool %d, size %D", pool->id,
		  size);

        if (size < sizeof(void *)) {
                size = sizeof(void *);  // Because mem_free() will dereference my_list->next
        }
        p = (struct mem_block *)CALLOC(1, size + sizeof(struct mem_block));

	p->marker = MEM_MARKER;
	p->pool = pool;
	p->next = pool->root;
	p->prev = NULL;
#ifndef NDEBUG
	p->size = size;
	pool->size += size;
#endif
	pool->root = p;

	if (p->next)
		p->next->prev = p;

	DBG_PRINTF(true, " at %p", p);

	return ++p;
}


PUBLIC void *mem_realloc(void *block, long newsize)
{
	struct mem_block *memblock = (struct mem_block *)block;

	--memblock;

	memblock = (struct mem_block *)Mem_resize(memblock, newsize + sizeof(struct mem_block), __FILE__, __LINE__);

#ifndef NDEBUG
	memblock->pool->size += newsize - memblock->size;
	memblock->size = newsize;
#endif

	if (memblock->next)
		memblock->next->prev = memblock;

	if (memblock->prev)
		memblock->prev->next = memblock;
	else
		memblock->pool->root = memblock;

	return ++memblock;
}


PUBLIC void cell_free(void *m)
{
	CELL *cell = (CELL *)m;
	CELL_HDR *c;

	--cell;

	DBG_PRINTF(true, "CELL free @ %p", cell);

	if (cell->c.marker == CELL_IN_MEM)
		mem_free(cell);
	else {
		if ((cell->c.marker & 0xff00) != CELL_MARKER)
			fatal("Cell_free() invalid marker");

		c = cell_hdr[cell->c.marker & 0xff];
		cell->c.next = c->root;
		c->root = cell;
	}
}


PUBLIC void *mem_free(void *m)
{
        if (m == NULL) {
                return NULL;
        }

	struct mem_block *memblock = (struct mem_block *)m;
	void *result = ((struct my_list *)m)->next;

	--memblock;

	DBG_PRINTF(true, "Memfree block at %p (pool %d)",
		  memblock, memblock->pool->id);

	if (memblock->marker != MEM_MARKER)
		fatal("Invalid marker in mem_free()");

	if (memblock->next)
		memblock->next->prev = memblock->prev;

	if (memblock->prev)
		memblock->prev->next = memblock->next;
	else
		memblock->pool->root = memblock->next;

#ifndef NDEBUG
	memblock->pool->size -= memblock->size;
#endif
	FREE(memblock);

	return result;
}


PUBLIC void cell_freepool(unsigned pool)
{
	unsigned i;
	CELL_HDR *c = cell_hdr[pool];

	if (c == NULL) {
		return;
	}

	c->root = c->addr;

	for (i = 0; i < CELL_POOLSIZE - 1; i++)
		CELL_ADDR(c, pool, i)->c.next = CELL_ADDR(c, pool, i + 1);

	CELL_ADDR(c, pool, CELL_POOLSIZE - 1)->c.next = (CELL *) CELL_FULL;
}


PUBLIC void mem_freepool(unsigned pool)
{
	if (pool >= NR_FIXED_POOLS)
		fatal("Invalid pool number in mem_alloc()");

	mem_freepool_private(&mem_pool[pool]);

}

PUBLIC void mem_freepool_private(struct mem_pool *pool)
{
	struct mem_block *work = pool->root;
	struct mem_block *next;

	DBG_PRINTF(true, "Freepool %d", pool->id);

	while (work) {
		DBG_PRINTF(true, "  Free block at %p",
			  work);

		next = work->next;

		if (work->marker != MEM_MARKER)
			fatal("Invalid marker in mem_freepool(%p)", pool);

		FREE(work);
		work = next;
	}

	pool->root = NULL;
#ifndef NDEBUG
	pool->size = 0;
#endif

	if (pool->id == RUN_POOL) {
		cell_freepool(INT_CPOOL);
		cell_freepool(FLOAT_CPOOL);
	}
}


PUBLIC void mem_shiftmem(unsigned _frompool, struct mem_pool *topool)
{
	struct mem_pool *frompool = &mem_pool[_frompool];
	struct mem_block *work = frompool->root;

	DBG_PRINTF(true,
		  "Shift mem from pool %d to pool %d",
		  frompool->id, topool->id);

	if (!work)
		return;

	while (work->next) {
		work->pool = topool;
		work = work->next;
	}

	work->pool = topool;
	work->next = topool->root;
	topool->root = frompool->root;
	frompool->root = NULL;
#ifndef NDEBUG
	topool->size += frompool->size;
	frompool->size = 0;
#endif

	if (work->next)
		work->next->prev = work;
}

#ifndef NDEBUG
PUBLIC void mem_debug(void)
{
	int i;

	for (i = 0; i < NR_FIXED_POOLS; i++)
		my_printf(MSG_DEBUG, true, "poolsize[%d]=%D", i,
			  mem_pool[i].size);
}
#endif

PUBLIC struct mem_pool *pool_new(void)
{
	struct mem_pool *work = GETCORE(MISC_POOL, struct mem_pool);

	pool_init(work);

	DBG_PRINTF(true, "Allocating new memory pool %d",
		  work->id);

	return work;
}
