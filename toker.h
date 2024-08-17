#ifndef TOKER_H
#define TOKER_H

#include "./include/bson.h"
#include "types.h"
#include "str.h"

enum tok_type
{
    TOK_TYPE_SYM,
    TOK_TYPE_INT,
    TOK_TYPE_STR
};

enum tok_state
{
    TOK_STATE_ERR,
    TOK_STATE_WS,
    TOK_STATE_MCMNT,
    TOK_STATE_SLCMNT,
    TOK_STATE_MLCMNT,
    TOK_STATE_TOK,
    TOK_STATE_INT,
    TOK_STATE_STR,
};

struct tok
{
    enum tok_type type;
    int int_val;
    struct str str_val;
};

struct toker
{
    enum tok_state state;
    size pos;
    int line_num;
    int next_col_num;
    int col_num;
    struct str str;
};

void init_toker(struct toker *toker, struct str str);
bool has_next_tok(struct toker *toker);
struct bson_res next_tok(struct toker *toker, struct tok *tok);

#endif
