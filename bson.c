#include "arena.h"
#include "toker.h"
#include "./include/bson.h"
#include "bson_int.h"
#include "bson_str.h"
#include "bson_obj.h"
#include "bson_list.h"

static int max_depth = 100;

struct start_tok
{
    struct tok tok;
    struct bson_res res;
};

static struct bson_res parse_entity(struct arena *arena, int depth, struct toker *toker,
                                    struct start_tok *start, struct bson_type **type);
    
static bool cmp_toks(struct tok *etok, struct tok *atok)
{
    return (etok->type == atok->type)
        && (etok->type == TOK_TYPE_INT
            ? (etok->int_val == atok->int_val) : strequals(etok->str_val, atok->str_val));
}

static struct bson_res expect_sym(struct toker *toker, struct str sym)
{
    struct tok etok = { .type = TOK_TYPE_SYM, .str_val = sym };
    struct tok atok;
    struct bson_res res = { 0 };

    if (!has_next_tok(toker))
    {
        res.rc = BRC_EOS;
        return res;
    }
    res = next_tok(toker, &atok);
    if (res.rc == BRC_SUCCESS)
    {
        if (!cmp_toks(&etok, &atok))
        {
            res.rc = BRC_SYNERR;
        }
    }
    return res;
}

static struct bson_res parse_obj(struct arena *arena, int depth, struct toker *toker,
                                 struct bson_obj **obj)
{
    struct tok rbrace = { .type = TOK_TYPE_SYM, .str_val = str("}") };
    struct tok comma = { .type = TOK_TYPE_SYM, .str_val = str(",") };
    bool comma_needed = false;
    struct tok tok;
    struct bson_res res = { BRC_SUCCESS, 0, 0 };

    if (depth > max_depth)
    {
        res.rc = BRC_TOODEEP;
        return res;
    }
    
    *obj = bson_obj_new(arena);
    if (!*obj)
    {
        res.rc = BRC_NOMEM;
        return res;
    }

    while (res.rc == BRC_SUCCESS && has_next_tok(toker))
    {
        res = next_tok(toker, &tok);
        if (res.rc == BRC_SUCCESS)
        {
            if (cmp_toks(&rbrace, &tok))
            {
                return res;
            }
            else if (cmp_toks(&comma, &tok))
            {
                if (comma_needed)
                {
                    comma_needed = false;
                }
                else
                {
                    res.rc = BRC_SYNERR;
                }
            }
            else if (tok.type == TOK_TYPE_STR && !comma_needed)
            {
                struct tok field = tok;
                struct bson_type *type;

                res = expect_sym(toker, str(":"));
                if (res.rc == BRC_SUCCESS)
                {
                    res = parse_entity(arena, depth, toker, NULL, &type);
                    if (res.rc == BRC_SUCCESS)
                    {
                        if (!bson_obj_put(arena, *obj, field.str_val, type))
                        {
                            res.rc = BRC_NOMEM;
                        }
                        comma_needed = true;
                    }
                }
            }
            else
            {
                res.rc = BRC_SYNERR;
            }
        }
    }
    if (res.rc == BRC_SUCCESS)
    {
        res.rc = BRC_EOS;
    }
    return res;
}

static struct bson_res parse_list(struct arena *arena, int depth, struct toker *toker,
                                  struct bson_list **list)
{
    struct tok rbracket = { .type = TOK_TYPE_SYM, .str_val = str("]") };
    struct tok comma = { .type = TOK_TYPE_SYM, .str_val = str(",") };
    bool comma_needed = false;
    struct tok tok;
    struct bson_type *type;
    struct bson_res res = { BRC_SUCCESS, 0, 0 };

    if (depth > max_depth)
    {
        res.rc = BRC_TOODEEP;
        return res;
    }
    
    *list = bson_list_new(arena);
    if (!*list)
    {
        res.rc = BRC_NOMEM;
        return res;
    }

    while (res.rc == BRC_SUCCESS && has_next_tok(toker))
    {
        res = next_tok(toker, &tok);
        if (res.rc == BRC_SUCCESS)
        {
            if (cmp_toks(&rbracket, &tok))
            {
                return res;
            }
            else if (cmp_toks(&comma, &tok))
            {
                if (comma_needed)
                {
                    comma_needed = false;
                }
                else
                {
                    res.rc = BRC_SYNERR;
                }
            }
            else if (!comma_needed)
            {
                res = parse_entity(arena, depth, toker, &(struct start_tok) {tok, res}, &type);
                if (res.rc == BRC_SUCCESS)
                {
                    if (!bson_list_push(arena, *list, type))
                    {
                        res.rc = BRC_NOMEM;
                    }
                    comma_needed = true;
                }
            }
            else
            {
                res.rc = BRC_SYNERR;
            }
        }
    }
    if (res.rc == BRC_SUCCESS)
    {
        res.rc = BRC_EOS;
    }
    return res;
}

static struct bson_res parse_entity(struct arena *arena, int depth, struct toker *toker,
                                    struct start_tok *start, struct bson_type **type)
{
    struct tok tok;
    struct bson_res res = { 0 };

    if (start)
    {
        tok = start->tok;
        res = start->res;
    }
    else
    {
        if (!has_next_tok(toker))
        {
            res.rc = BRC_EOS;
            return res;
        }
        res = next_tok(toker, &tok);
        if (res.rc != BRC_SUCCESS)
        {
            return res;
        }
    }
    if (tok.type == TOK_TYPE_INT)
    {
        *type = (struct bson_type *) bson_int_new(arena, tok.int_val);
        if (!*type)
        {
            res.rc = BRC_NOMEM;
        }
    }
    else if (tok.type == TOK_TYPE_STR)
    {
        *type = (struct bson_type *) bson_str_new(arena, tok.str_val);
        if (!*type)
        {
            res.rc = BRC_NOMEM;
        }
    }
    else if (tok.type == TOK_TYPE_SYM)
    {
        struct tok lbrace = { .type = TOK_TYPE_SYM, .str_val = str("{") };
        struct tok lbracket = { .type = TOK_TYPE_SYM, .str_val = str("[") };

        if (cmp_toks(&lbrace, &tok))
        {
            res = parse_obj(arena, ++depth, toker, (struct bson_obj **) type);
        }
        else if (cmp_toks(&lbracket, &tok))
        {
            res = parse_list(arena, ++depth, toker, (struct bson_list **) type);
        }
        else
        {
            res.rc = BRC_SYNERR;
        }
    }
    else
    {
        res.rc = BRC_SYNERR;
    }
    return res;
}

void set_max_depth(int depth)
{
    max_depth = depth;
}

struct bson_res bson_parse(struct arena *arena, struct str str,
                           struct bson_obj **obj)
{
    struct bson_res res = { 0 };
    struct str cstr;
    struct toker toker;

    cstr.len = str.len;
    cstr.data = new(arena, char, str.len);
    if (!cstr.data)
    {
        res.rc = BRC_NOMEM;
        return res;
    }
    memcpy(cstr.data, str.data, (size_t) str.len);
    
    init_toker(&toker, cstr);

    if (!has_next_tok(&toker))
    {
        res.rc = BRC_EOS;
        return res;
    }    
    res = expect_sym(&toker, str("{"));
    if (res.rc == BRC_SUCCESS)
    {
        res = parse_obj(arena, 1, &toker, obj);
        if (res.rc == BRC_SUCCESS && has_next_tok(&toker))
        {
            res = next_tok(&toker, &(struct tok) { 0 });
            if (res.rc == BRC_SUCCESS)
            {
                res.rc = BRC_SYNERR;
            }
        }
    }
    return res;
}
