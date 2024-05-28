#include "bson.h"

struct bson_type *bson_obj_get(struct bson_obj *obj, const char *key)
{
    return NULL;
}

int bson_obj_len(struct bson_obj *obj)
{
    return 0;
}

void bson_obj_iter_begin(struct bson_obj *obj)
{
    return;
}

struct bson_obj *bson_obj_iter_next(struct bson_obj *obj)
{
    return NULL;
}

struct bson_str *bson_obj_iter_key(struct bson_obj *obj)
{
    return NULL;
}

struct bson_type *bson_list_get(struct bson_list *list, int i)
{
    return NULL;
}

int bson_list_len(struct bson_list *list)
{
    return 0;
}

struct bson_obj *bson_parse(struct arena *arena, const char *str)
{
    if (arena == NULL || str == NULL)
    {
        return NULL;
    }
    
    return NULL;
}
