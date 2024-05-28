#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include "arena.h"

// 1 GB 
#define CAPACITY (1 << 30)

bool new_arena(struct arena *arena)
{
    arena->beg = mmap(NULL, CAPACITY, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (arena->beg == MAP_FAILED)
    {
        return false;
    }
    arena->end = arena->beg + CAPACITY;
    return true;
}

void *alloc(struct arena *arena, size sz, size align, size count)
{
    size padding = -(iptr) arena->beg & (align - 1);
    size available = arena->end - arena->beg - padding;
    if (available < 0 || count >  available / sz)
    {
        return NULL;
    }
    void *p = arena->beg + padding;
    arena->beg += padding + count * sz;
    return memset(p, 0, (size_t) (count * sz));
}
