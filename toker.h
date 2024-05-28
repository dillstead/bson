#ifndef TOKER_H
#define TOKER_H

#include "str.h"
#include "arena.h"
#include "bson.h"

enum tok_error
{
    E_STATE,
    E_UNEXCHAR,
    E_TOOLARGE,
    E_UNTERM
};

enum tok_type
{
    T_ERR,
    T_SYM,
    T_INT,
    T_STR
};

enum tok_state
{
    S_WS,
    S_MCMNT,
    S_SLCMNT,
    S_MLCMNT,
    S_TOK,
    S_INT,
    S_STR,
};

struct tok
{
    enum tok_type type;
    size line_num;
    int int_val;
    struct s8 str_val;
};

struct toker
{
    enum tok_state state;
    size pos;
    size line_num;
    struct s8 str;
};

void init_toker(struct toker *toker, struct s8 str);
bool has_next_tok(struct toker *toker);
struct tok next_tok(struct toker *toker);
const char *type_to_str(enum tok_type type);

#endif 
