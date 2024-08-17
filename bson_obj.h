#ifndef BSON_OBJ_H
#define BSON_OBJ_H

#include "./include/bson.h"

struct bson_obj
{
    enum btype type;
    struct str key;
    struct bson_type *val;
    struct bson_obj *child[4];
    union
    {
        struct bson_obj **tail;
        struct bson_obj *next;
    };
};

struct bson_obj *bson_obj_new(struct arena *arena);
struct bson_type *bson_obj_put(struct arena *arena, struct bson_obj *obj,
                               struct str key, struct bson_type *type);
#endif

