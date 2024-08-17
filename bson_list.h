#ifndef BSON_LIST_H
#define BSON_LIST_H

#include "types.h"
#include "./include/bson.h"

struct bson_list
{
    enum btype type;
    struct bson_type **vals;
    int len;
    int cap;
};

struct bson_list *bson_list_new(struct arena *arena);
struct bson_type *bson_list_push(struct arena *arena, struct bson_list *list,
                                 struct bson_type *type);
#endif
