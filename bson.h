#ifndef BSON_H
#define BSON_H

#include "types.h"
#include "arena.h"

enum btype
{
    OBJ,
    LIST,
    INT,
    STR
};

struct bson_obj;
struct bson_list;

struct bson_type
{
    enum btype type;
};

struct bson_obj
{
    enum btype type;
};

struct bson_list
{
    enum btype type;
};

struct bson_int
{
    enum btype type;
    int val;
};

struct bson_str
{
    enum btype type;
    const char *str;
};

struct bson_type *bson_obj_get(struct bson_obj *obj, const char *key);
int bson_obj_len(struct bson_obj *obj);
void bson_obj_iter_begin(struct bson_obj *obj);
struct bson_obj *bson_obj_iter_next(struct bson_obj *obj);
struct bson_str *bson_obj_iter_key(struct bson_obj *obj);
struct bson_type *bson_list_get(struct bson_list *list, int i);
int bson_list_len(struct bson_list *list);
struct bson_obj *bson_parse(struct arena *arena, const char *str);

#endif
