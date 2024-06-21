#ifndef BSON_INT_H
#define BSON_INT_H

#include "bson.h"

struct bson_int
{
    enum btype type;
    int val;
};

struct bson_int *bson_int_new(struct arena *arena, int val);

#endif

