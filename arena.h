#ifndef ARENA_H
#define ARENA_H

#include "types.h"

#define new(a, t, n) (t *) alloc(a, sizeof(t), _Alignof(t), n)

struct arena
{
    byte *beg;
    byte *end;
    byte *commit;
};

void *alloc(struct arena *arena, size sz, size align, size count);

#endif
