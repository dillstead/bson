#include <setjmp.h>
#include "arena.h"
#include "str.h"
#include "bson_list.h"

#define push(arena, jbuf, list)                          \
    ((list)->len >= (list)->cap                          \
        ? grow(arena, jbuf,list, sizeof(*(list)->vals)), \
          (list)->vals + (list)->len++                   \
        : (list)->vals + (list)->len++)

static void grow(struct arena *arena, jmp_buf jbuf, void *list, int val_sz)
{    
    struct
    {
        enum btype type;
        struct bson_type **vals;
        int len;
        int cap;
    } tmp_list;
    
    memcpy(&tmp_list, list, sizeof(tmp_list));

    tmp_list.cap = tmp_list.cap ? tmp_list.cap : 1;
    int align = 16;
    void *vals = alloc(arena, 2 * val_sz, align, tmp_list.cap);
    if (!vals)
    {
        __builtin_longjmp(jbuf, 1);
    }
    tmp_list.cap *= 2;
    if (tmp_list.len > 0)
    {
        memcpy(vals, tmp_list.vals, (usize) (tmp_list.len * val_sz));
    }
    tmp_list.vals = vals;
    memcpy(list, &tmp_list, sizeof(tmp_list));
}

struct bson_list *bson_list_new(struct arena *arena)
{
    struct bson_list *list = new(arena, struct bson_list, 1);

    if (list)
    {
        list->type = BTYPE_LIST;
    }
    return list;
}

struct bson_type *bson_list_push(struct arena *arena, struct bson_list *list, struct bson_type *type)
{
    jmp_buf jbuf;
    
    if (__builtin_setjmp(jbuf))
    {
        return NULL;
    }
    *push(arena, jbuf, list) = type;
    return type;
}

int bson_list_len(struct bson_list *list)
{
    return list->len;
}

struct bson_type *bson_list_get(struct bson_list *list, int i)
{
    return i >= 0 && i < list->len ? list->vals[i] : NULL;
}
