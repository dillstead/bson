#include <setjmp.h>
#include "arena.h"
#include "str.h"
#include "bson_obj.h"

static struct bson_type **upsert(struct arena *arena, jmp_buf jbuf, struct bson_obj **obj,
                                 struct str key)
{
    struct bson_obj **root = obj;
    
    obj = &(*obj)->child[0];
    for (uint64_t h = strhash(key); *obj; h <<= 2)
    {
        if (strequals(key, (*obj)->key))
        {
            return &(*obj)->val;
        }
        obj = &(*obj)->child[h >> 62];
    }
    if (!arena)
    {
        return NULL;
    }
    *obj = new(arena, struct bson_obj, 1);
    if (!*obj)
    {
        __builtin_longjmp(jbuf, 1);
    }
    (*obj)->key = key;
    *(*root)->tail = *obj;
    (*root)->tail = &(*obj)->next;
    return &(*obj)->val;
}

struct bson_obj *bson_obj_new(struct arena *arena)
{
    struct bson_obj *obj = new(arena, struct bson_obj, 1);

    if (obj)
    {
        obj->type = BTYPE_OBJ;
        obj->tail = &obj->child[0];
    }
    return obj;
}

struct bson_type *bson_obj_put(struct arena *arena, struct bson_obj *obj, struct str key,
                               struct bson_type *type)
{
    jmp_buf jbuf;
    
    if (__builtin_setjmp(jbuf))
    {
        return NULL;
    }
    *upsert(arena, jbuf, &obj, key) = type;
    return type;
}

struct bson_type *bson_obj_get(struct bson_obj *obj, struct str key)
{
    jmp_buf jbuf;
    
    struct bson_type **type = upsert(NULL, jbuf, &obj, key);

    return type ? *type : NULL;
}

struct bson_obj *bson_obj_iter_begin(struct bson_obj *obj)
{
    return obj ? obj->child[0] : obj;
}

struct bson_obj *bson_obj_iter_next(struct bson_obj *obj)
{
    return obj ? obj->next : obj;
}

struct str bson_obj_iter_key(struct bson_obj *obj)
{
    return obj->key;
}
