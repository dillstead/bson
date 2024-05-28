#include <stdio.h>
#include "test_tok.h"
#include "str.h"
#include "arena.h"
#include "toker.h"

static void log_tok(struct tok *tok)
{
    fprintf(stderr, "{ %s, %td, ", type_to_str(tok->type), tok->line_num);
    if (tok->type == T_INT)
    {
        fprintf(stderr, "%d }", tok->int_val);
    }
    else
    {
        fprintf(stderr, "'%.*s' }", (int) tok->str_val.len, tok->str_val.data);
    }
}

static bool cmp_toks(struct tok *etok, struct tok *atok)
{
    return (etok->type == atok->type)
        && (etok->line_num == atok->line_num)
        && (etok->type == T_INT ? (etok->int_val == atok->int_val) : s8cmp(etok->str_val, atok->str_val));
}

static bool test(const char *name, struct s8 str, struct tok *expected, size len)
{
    struct toker toker;
    size i = 0;

    init_toker(&toker, str);
    for (i = 0; i < len && has_next_tok(&toker); i++)
    {
        struct tok tok = next_tok(&toker);
        if (!cmp_toks(expected + i, &tok))
        {
            fprintf(stderr, "%s actual ", name);
            log_tok(&tok);
            fprintf(stderr, ", expected ");
            log_tok(expected + i);
            fprintf(stderr, "\n");
            return false;
        }
    }
    return i == len && !has_next_tok(&toker);
}

bool test_toker(void)
{
    struct s8 test1 = s8("{ : \"\" \"x\", 123, \n\n -239 [ }  ]");
    struct tok expected1[] = {
        { .type = T_SYM, .line_num = 1, .str_val = s8("{") },
        { .type = T_SYM, .line_num = 1, .str_val = s8(":") },
        { .type = T_STR, .line_num = 1, .str_val = s8("")  },
        { .type = T_STR, .line_num = 1, .str_val = s8("x") },
        { .type = T_SYM, .line_num = 1, .str_val = s8(",") },
        { .type = T_INT, .line_num = 1, .int_val = 123     },
        { .type = T_SYM, .line_num = 1, .str_val = s8(",") },
        { .type = T_INT, .line_num = 3, .int_val = -239    },
        { .type = T_SYM, .line_num = 3, .str_val = s8("[") },
        { .type = T_SYM, .line_num = 3, .str_val = s8("}") },
        { .type = T_SYM, .line_num = 3, .str_val = s8("]") }};
    struct s8 test2 = s8("-2147483648 2147483647 -2147483649 2147483648");
    struct tok expected2[] = {
        { .type = T_INT, .line_num = 1, .int_val = -2147483648 },
        { .type = T_INT, .line_num = 1, .int_val = 2147483647  },
        { .type = T_ERR, .line_num = 1, .int_val = E_TOOLARGE  },
        { .type = T_ERR, .line_num = 1, .int_val = E_TOOLARGE  }};
    struct s8 test3 = s8("-x");
    struct tok expected3[] = {
        { .type = T_ERR, .line_num = 1, .int_val = E_UNEXCHAR }};
    struct s8 test4 = s8("x");
    struct tok expected4[] = {
        { .type = T_ERR, .line_num = 1, .int_val = E_UNEXCHAR }};
    struct s8 test5 = s8("\"hello world");
    struct tok expected5[] = {
        { .type = T_ERR, .line_num = 1, .int_val = E_UNTERM }};
    struct s8 test6 = s8("");
    struct s8 test7 = s8("   \n\n    \n");
    return test("test1", test1, expected1, countof(expected1))
        && test("test2", test2, expected2, countof(expected2))
        && test("test3", test3, expected3, countof(expected3))
        && test("test4", test4, expected4, countof(expected4))
        && test("test5", test5, expected5, countof(expected5))
        && test("test6", test6, expected5, 0)
        && test("test7", test7, expected5, 0);
}
