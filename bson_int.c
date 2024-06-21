#include "bson_int.h"
#include "arena.h"

struct bson_int *bson_int_new(struct arena *arena, int val)
{
    struct bson_int *bint = new(arena, struct bson_int, 1);

    if (bint)
    {
        bint->type = BTYPE_INT;
        bint->val = val;
    }
    return bint;
}

int bson_int_get_val(struct bson_int *bint)
{
    return bint->val;
}
