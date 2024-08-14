#include <stdio.h>
#include "toker.h"
#include "test_utils.h"
#include "test_tok.h"

struct tok_res
{
    struct bson_res res;
    struct tok tok;
};

struct tok_test
{
    const char *name;
    struct str input;
    int num_res;
    struct tok_res(*res)[];
};

static const char *type_to_str(enum tok_type type)
{
    switch (type)
    {
    case TOK_TYPE_SYM:
    {
        return "sym";
    }
    case TOK_TYPE_INT:
    {
        return "int";
    }
    case TOK_TYPE_STR:
    {
        return "str";
    }
    default:
    {
        return "unknown";
    }
    }
}
    
static void log_res_tok(struct bson_res *res, struct tok *tok)
{
    fprintf(stderr, "%s %d %d", rc_to_str(res->rc), res->line_num, res->col_num);
    if (res->rc == BRC_SUCCESS)
    {
        fprintf(stderr, " { %s, ", type_to_str(tok->type));
        if (tok->type == TOK_TYPE_INT)
        {
            fprintf(stderr, "%d }", tok->int_val);
        }
        else
        {
            fprintf(stderr, "'%.*s' }", tok->str_val.len, tok->str_val.data);
        }
    }
}

static bool cmp_toks(struct tok *etok, struct tok *atok)
{
    return (etok->type == atok->type)
        && (etok->type == TOK_TYPE_INT ? (etok->int_val == atok->int_val) : strequals(etok->str_val, atok->str_val));
}

static bool test(struct tok_test *test)
{
    struct toker toker;
    struct tok tok;
    int len = test->num_res;
    int i = 0;

    init_toker(&toker, test->input);
    for (i = 0; i < len && has_next_tok(&toker); i++)
    {
        struct bson_res res = next_tok(&toker, &tok);
        struct tok_res *eres = *test->res;

        if (eres[i].res.rc != res.rc
            || eres[i].res.line_num != res.line_num
            || eres[i].res.col_num != res.col_num
            || (res.rc == BRC_SUCCESS && !cmp_toks(&eres[i].tok, &tok)))
        {
            fprintf(stderr, "%s: actual: ", test->name);
            log_res_tok(&res, &tok);
            fprintf(stderr, ", expected: ");
            log_res_tok(&eres[i].res, &eres[i].tok);
            fprintf(stderr, "\n");
            return false;
        }
    }
    return i == len && !has_next_tok(&toker);
}

bool test_toker(void)
{
    struct tok_test tests[] =
    {
        {
            "toker test 1",
            str((char []) { "{ : \"\" \"\\\\x\\\"\\n\", 123, \n\n -239 [ }  ] \"x\""    }),
            12,
            &(struct tok_res[])
            {
                {{BRC_SUCCESS, 1, 1 }, { .type = TOK_TYPE_SYM, .str_val = str("{")      }},
                {{BRC_SUCCESS, 1, 3 }, { .type = TOK_TYPE_SYM, .str_val = str(":")      }},
                {{BRC_SUCCESS, 1, 5 }, { .type = TOK_TYPE_STR, .str_val = str("")       }},
                {{BRC_SUCCESS, 1, 8 }, { .type = TOK_TYPE_STR, .str_val = str("\\x\"\n")}},
                {{BRC_SUCCESS, 1, 17}, { .type = TOK_TYPE_SYM, .str_val = str(",")      }},
                {{BRC_SUCCESS, 1, 19}, { .type = TOK_TYPE_INT, .int_val = 123           }},
                {{BRC_SUCCESS, 1, 22}, { .type = TOK_TYPE_SYM, .str_val = str(",")      }},
                {{BRC_SUCCESS, 3, 2 }, { .type = TOK_TYPE_INT, .int_val = -239          }},
                {{BRC_SUCCESS, 3, 7 }, { .type = TOK_TYPE_SYM, .str_val = str("[")      }},
                {{BRC_SUCCESS, 3, 9 }, { .type = TOK_TYPE_SYM, .str_val = str("}")      }},
                {{BRC_SUCCESS, 3, 12}, { .type = TOK_TYPE_SYM, .str_val = str("]")      }},
                {{BRC_SUCCESS, 3, 14}, { .type = TOK_TYPE_STR, .str_val = str("x")      }}
            },
        },
        {
            "toker test 2",
            str((char []) {"-2147483648 2147483647"                                     }),
            2,
            &(struct tok_res[])
            {
                {{BRC_SUCCESS, 1, 1}, {.type = TOK_TYPE_INT, .int_val = -2147483648     }},
                {{BRC_SUCCESS, 1, 13}, {.type = TOK_TYPE_INT, .int_val = 2147483647     }}
            },
        },
        {
            "toker test 3",
            str((char []) {"-2147483649"   }),
            1,
            &(struct tok_res[])
            {
                {{BRC_OUTRANGE, 1, 1}, { 0 }}
            },
        },
        {
            "toker test 4",
            str((char []) {"2147483648"}),
            1,
            &(struct tok_res[])
            {
                {{BRC_OUTRANGE, 1, 1}, { 0 }}
            },
        },
        {
            "toker test 5",
            str((char []) {"-x"}),
            1,
            &(struct tok_res[])
            {
                {{BRC_SYNERR, 1, 1}, { 0 }}
            },
        },
        {
            "toker test 6",
            str((char []) {"/k"}),
            1,
            &(struct tok_res[])
            {
                {{BRC_SYNERR, 1, 1}, { 0 }}
            },
        },
        {
            "toker test 7",
            str((char []) {"x"}),
            1,
            &(struct tok_res[])
            {
                {{BRC_SYNERR, 1, 1}, { 0 }}
            },
        },
        {
            "toker test 8",
            str((char []) {"\200"}),
            1,
            &(struct tok_res[])
            {
                {{BRC_BADCHAR, 1, 1}, { 0 }}
            },
        },
        {
            "toker test 9",
            str((char []) {"\177"}),
            1,
            &(struct tok_res[])
            {
                {{BRC_BADCHAR, 1, 1}, { 0 }}
            },
        },
        {
            "toker test 10",
            str((char []) {"\"hello\177"}),
            1,
            &(struct tok_res[])
            {
                {{BRC_BADCHAR, 1, 7}, { 0 }}
            },
        },
        {
            "toker test 11",
            str((char []) {"\n   /\n"}),
            1,
            &(struct tok_res[])
            {
                {{BRC_SYNERR, 2, 4}, { 0 }}
            },
        },
        {
            "toker test 12",
            str((char []) {"\"hello world"}),
            1,
            &(struct tok_res[])
            {
                {{BRC_UNTERM, 1, 1}, { 0 }}
            },
        },
        {
            "toker test 13",
            str((char []) {"        "}),
            0,
            NULL
        },
        {
            "toker test 14",
            str((char []) {"/* */   /*\n\n  */ ////    \n"}),
            0,
            NULL
        },
        {
            "toker test 15",
            str((char []) {"   \"\\k\""}),
            1,
            &(struct tok_res[])
            {
                {{BRC_BADESC, 1, 6}, { 0 }}
            },
        }
    };

    for (int i = 0; i < countof(tests); i++)
    {
        if (!test(tests + i))
        {
            return false;
        }
    }
    return true;
}
