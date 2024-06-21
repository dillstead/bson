#include "bson_str.h"
#include "arena.h"

struct bson_str *bson_str_new(struct arena *arena, struct str val)
{
    struct bson_str *bstr = new(arena, struct bson_str, 1);

    if (bstr)
    {
        bstr->type = BTYPE_STR;
        bstr->val = val;
    }
    return bstr;
}

struct str bson_str_get_val(struct bson_str *bstr)
{
    return bstr->val;
}
