#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include "arena.h"

#define BITMASK(SHIFT, CNT) (((1ul << (CNT)) - 1) << (SHIFT))
#define PGSHIFT 0                          
#define PGBITS  12                         
#define PGSIZE  (1 << PGBITS)              
#define PGMASK  BITMASK(PGSHIFT, PGBITS)   

bool new_arena(struct arena *arena, int sz)
{
    if (sz <= 0)
    {
        return false;
    }
    arena->beg = mmap(NULL, (usize) sz, PROT_NONE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (arena->beg == MAP_FAILED)
    {
        return false;
    }
    arena->end = arena->beg + sz;
    arena->commit = arena->beg;
    return true;
}

static inline void *pg_round_up (void *addr)
{
    return (void *) (((uintptr_t) addr + PGSIZE - 1) & ~PGMASK);
}

void *alloc(struct arena *arena, size sz, size align, size count)
{
    size padding = -(iptr) arena->beg & (align - 1);
    size available = arena->end - arena->beg - padding;
    if (available < 0 || count >  available / sz)
    {
        return NULL;
    }
    byte *p = arena->beg + padding;
    arena->beg += padding + count * sz;
    byte *end = pg_round_up(arena->beg);
    if (end >= arena->commit)
    {
        if (mprotect(arena->commit, (size_t) (end - arena->commit),
                     PROT_READ | PROT_WRITE) == -1)
        {
            return NULL;
        }
        arena->commit = end;
    }
    return memset(p, 0, (size_t) (count * sz));
}
