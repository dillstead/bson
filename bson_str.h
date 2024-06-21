#ifndef BSON_STR_H
#define BSON_STR_H

#include "bson.h"
#include "str.h"

struct bson_str
{
    enum btype type;
    struct str val;
};

struct bson_str *bson_str_new(struct arena *arena, struct str val);

#endif
