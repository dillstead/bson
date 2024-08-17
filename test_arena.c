#include <stdio.h>
#include "./include/bson.h"
#include "arena.h"
#include "test_arena.h"

static bool test(void)
{
    struct arena arena;

    if (new_arena(&arena, 0) || new_arena(&arena, -1) ||
        !new_arena(&arena, 1 << 14) || !new(&arena, char, 0))
    {
        return false;
    }
    /*for (int i = 0; i < (1 << 14); i++)
    {
        if (!new(&arena, char, 1))
        {
            return false;
        }
    }
    return new(&arena, char, 1) ? false : true;
    */
    return new(&arena, char, 1 << 14) ? true : false;
}

bool test_arena(void)
{
    if (!test())
    {
        fprintf(stderr, "type test arenas: failed\n");
        return false;        
    }
    return true;
}
