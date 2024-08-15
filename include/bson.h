#ifndef BSON_H
#define BSON_H

#include <stdbool.h>

enum btype
{
    BTYPE_OBJ,
    BTYPE_LIST,
    BTYPE_INT,
    BTYPE_STR
};

enum brc
{
    BRC_SUCCESS,
    BRC_BADSTATE,
    BRC_NOMEM,
    BRC_OUTRANGE,
    BRC_UNTERM,
    BRC_BADESC,
    BRC_BADCHAR,
    BRC_EOS,
    BRC_SYNERR
};

struct arena;
struct bson_obj;
struct bson_list;
struct bson_int;
struct bson_str;

struct str
{
    char *data;
    int len;
};

struct arena
{
    char *beg;
    char *end;
    char *commit;
};

struct bson_type
{
    enum btype type;
};

struct bson_res
{
    int rc;
    int line_num;
    int col_num;
};

bool new_arena(struct arena *arena, int sz);

struct bson_type *bson_obj_get(struct bson_obj *obj, struct str key);
struct bson_obj *bson_obj_iter_begin(struct bson_obj *obj);
struct bson_obj *bson_obj_iter_next(struct bson_obj *obj);
struct str bson_obj_iter_key(struct bson_obj *obj);

struct bson_type *bson_list_get(struct bson_list *list, int i);
int bson_list_len(struct bson_list *list);

int bson_int_get_val(struct bson_int *bint);
struct str bson_str_get_val(struct bson_str *str);

const char *brc_to_str(enum brc rc);
struct bson_res bson_parse(struct arena *arena, struct str str,
                           struct bson_obj **obj);
#endif
