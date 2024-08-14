#include <stdio.h>
#include "bson.h"
#include "bson_obj.h"
#include "bson_list.h"
#include "bson_str.h"
#include "bson_int.h"
#include "arena.h"
#include "test_types.h"

static bool setup_arenas(struct arena *arena, struct arena *low_arena)
{
    if (!new_arena(low_arena, 1) || !new_arena(arena, 1 << 12))
    {
        fprintf(stderr, "type test arenas: failed\n");
        return false;
    }
    return true;
}
    
static bool test_no_mem(struct arena low_arena)
{
    if (bson_int_new(&low_arena, 0) || bson_str_new(&low_arena, str(""))
        || bson_list_new(&low_arena) || bson_obj_new(&low_arena))
    {
        fprintf(stderr, "type test no_mem: failed\n");
        return false;
    }
    return true;
}

static bool test_int(struct arena arena)
{
    struct bson_int *bint;
    int val = 10;
    
    bint = bson_int_new(&arena, val);
    if (bint->type != BTYPE_INT || bson_int_get_val(bint) != val)
    {
        fprintf(stderr, "type test int: failed\n");
        return false;
    }
    return true;
}

static bool test_str(struct arena arena)
{
    struct bson_str *bstr;
    struct str val = str("1001001 SOS");
    struct str empty = str("");

    bstr = bson_str_new(&arena, val);
    if (bstr->type != BTYPE_STR || !strequals(bson_str_get_val(bstr), val))
    {
        fprintf(stderr, "type test str: failed\n");
        return false;
    }
    bstr = bson_str_new(&arena, empty);
    if (bstr->type != BTYPE_STR || !strequals(bson_str_get_val(bstr), empty))
    {
        fprintf(stderr, "type test str: failed\n");
        return false;
    }
    return true;
}

static bool test_list(struct arena low_arena, struct arena arena)
{
    struct bson_list *list = bson_list_new(&arena);
    struct bson_int *bints[4];

    if (list->type != BTYPE_LIST || bson_list_len(list) != 0)
    {
        fprintf(stderr, "type test list: failed\n");
        return false;
    }
    if (bson_list_push(&low_arena, list, (struct bson_type *) list))
    {
        fprintf(stderr, "type test list: failed\n");
        return false;
    }
    for (int i = 0; i < countof(bints); i++)
    {
        bints[i] = bson_int_new(&arena, i);
        bson_list_push(&arena, list, (struct bson_type *) bints[i]);
    }
    if (bson_list_len(list) != countof(bints))
    {
        fprintf(stderr, "type test list: failed\n");
        return false;
    }
    for (int i = 0; i < countof(bints); i++)
    {
        struct bson_type *btype = bson_list_get(list, i);
        if (btype->type != BTYPE_INT)
        {
            fprintf(stderr, "type test list: failed\n");
        }
        struct bson_int *bint = (struct bson_int *) btype;
        if (bint != bints[i] || bson_int_get_val((struct bson_int *) btype) != i)
        {
            fprintf(stderr, "type test list: failed\n");
            return false;
        }
    }
    return true;
}

static bool test_obj(struct arena low_arena, struct arena arena)
{
    struct bson_obj *obj = bson_obj_new(&arena);
    struct str strs[4] = { str("one"), str("two"), str("three"), str("four") };
    struct bson_int *bints[4];

    if (obj->type != BTYPE_OBJ)
    {
        fprintf(stderr, "type test obj: failed\n");
        return false;
    }
    for (struct bson_obj *cur = bson_obj_iter_begin(obj); cur; cur = bson_obj_iter_next(cur))
    {
        fprintf(stderr, "type test obj: failed\n");
        return false;
    }
    if (bson_obj_put(&low_arena, obj, strs[1], (struct bson_type *) obj))
    {
        fprintf(stderr, "type test obj: failed\n");
        return false;
    }
    for (int i = 0; i < countof(strs); i++)
    {
        bints[i] = bson_int_new(&arena, i);
        bson_obj_put(&arena, obj, strs[i], (struct bson_type *) bints[i]);
    }
    if (bson_obj_get(obj, str("five")))
    {
        fprintf(stderr, "type test obj: failed\n");
        return false;
    }
    for (int i = 0; i < countof(strs); i++)
    {
        struct bson_type *btype = bson_obj_get(obj, strs[i]);
        if (btype->type != BTYPE_INT)
        {
            fprintf(stderr, "type test obj: failed\n");
        }
        struct bson_int *bint = (struct bson_int *) btype;
        if (bint != bints[i])
        {
            fprintf(stderr, "type test obj: failed\n");
            return false;
        }
    }
    int count = 0;
    for (struct bson_obj *cur = bson_obj_iter_begin(obj); cur; cur = bson_obj_iter_next(cur))
    {
        count++;
    }
    if (count != countof(strs))
    {
        fprintf(stderr, "type test obj: failed\n");
        return false;
    }
    int i = 0;
    for (struct bson_obj *cur = bson_obj_iter_begin(obj); cur; cur = bson_obj_iter_next(cur))
    {
        struct bson_type *btype = bson_obj_get(obj, bson_obj_iter_key(cur));
        if (btype->type != BTYPE_INT)
        {
            fprintf(stderr, "type test obj: failed\n");
        }
        struct bson_int *bint = (struct bson_int *) btype;
        if (bint != bints[i])
        {
            fprintf(stderr, "type test obj: failed\n");
            return false;
        }
        i++;
    }
    return true;
}

bool test_types(void)
{
    struct arena low_arena;
    struct arena arena;

    return setup_arenas(&arena, &low_arena) && test_no_mem(low_arena)
        && test_int(arena) && test_str(arena) && test_list(low_arena, arena)
        && test_obj(low_arena, arena);
}
