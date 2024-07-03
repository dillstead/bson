#include <stdio.h>
#include "toker.h"
#include "test_tok.h"

struct toker_res
{
    struct bson_res res;
    struct tok tok;
};

static const char *rc_to_str(enum brc rc)
{
    switch (rc)
    {
    case BRC_SUCCESS:
    {
        return "success";
    }
    case BRC_NOMEM:
    {
        return "out of memory";
    }
    case BRC_BADSTATE:
    {
        return "bad state";
    }
    case BRC_BADESC:
    {
        return "bad escape";
    }
    case BRC_UNEXCHAR:
    {
        return "unexpected character";
    }
    case BRC_UNTERM:
    {
        return "unterminated string";
    }
    case BRC_OUTRANGE:
    {
        return "out of range";
    }
    default:
    {
        return "unkown";
    }
    }
}

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
        return "unkown";
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

static bool test(const char *name, struct str str, struct toker_res *expected,
                 size len)
{
    struct toker toker;
    struct tok tok;
    size i = 0;

    init_toker(&toker, str);
    for (i = 0; i < len && has_next_tok(&toker); i++)
    {
        struct bson_res res = next_tok(&toker, &tok);

        if (expected[i].res.rc != res.rc
            || expected[i].res.line_num != res.line_num
            || expected[i].res.col_num != res.col_num
            || (res.rc == BRC_SUCCESS && !cmp_toks(&expected[i].tok, &tok)))
        {
            fprintf(stderr, "%s: actual: ", name);
            log_res_tok(&res, &tok);
            fprintf(stderr, ", expected: ");
            log_res_tok(&expected[i].res, &expected[i].tok);
            fprintf(stderr, "\n");
            return false;
        }
    }
    return i == len && !has_next_tok(&toker);
}

bool test_toker(void)
{
    struct str test1 = str((char []) { "{ : \"\" \"\\\\x\\\"\\n\", 123, \n\n -239 [ }  ] \"x\"" });
    struct toker_res expected1[] = {
        {{BRC_SUCCESS, 1, 1 }, { .type = TOK_TYPE_SYM, .str_val = str("{") }},
        {{BRC_SUCCESS, 1, 3 }, { .type = TOK_TYPE_SYM, .str_val = str(":") }},
        {{BRC_SUCCESS, 1, 5 }, { .type = TOK_TYPE_STR, .str_val = str("")  }},
        {{BRC_SUCCESS, 1, 8 }, { .type = TOK_TYPE_STR, .str_val = str("\\x\"\n") }},
        {{BRC_SUCCESS, 1, 17}, { .type = TOK_TYPE_SYM, .str_val = str(",") }},
        {{BRC_SUCCESS, 1, 19}, { .type = TOK_TYPE_INT, .int_val = 123      }},
        {{BRC_SUCCESS, 1, 22}, { .type = TOK_TYPE_SYM, .str_val = str(",") }},
        {{BRC_SUCCESS, 3, 2 }, { .type = TOK_TYPE_INT, .int_val = -239     }},
        {{BRC_SUCCESS, 3, 7 }, { .type = TOK_TYPE_SYM, .str_val = str("[") }},
        {{BRC_SUCCESS, 3, 9 }, { .type = TOK_TYPE_SYM, .str_val = str("}") }},
        {{BRC_SUCCESS, 3, 12}, { .type = TOK_TYPE_SYM, .str_val = str("]") }},
        {{BRC_SUCCESS, 3, 14}, { .type = TOK_TYPE_STR, .str_val = str("x") }}};
    struct str test2 = str((char []) {"-2147483648 2147483647"}); 
    struct toker_res expected2[] = {
        {{BRC_SUCCESS, 1, 1 }, { .type = TOK_TYPE_INT, .int_val = -2147483648 }},
        {{BRC_SUCCESS, 1, 13}, { .type = TOK_TYPE_INT, .int_val = 2147483647  }}};
    struct str test3 = str((char []) {"-2147483649"});
    struct str test4 = str((char []) {"2147483648"});
    struct toker_res outrange[] = {
        {{BRC_OUTRANGE, 1, 1}, { 0 }}};
    struct str test5 = str((char []) {"-x"});
    struct str test6 = str((char []) {"/k"});
    struct str test7 = str((char []) {"x"});
    struct str test8 = str((char []) {"\200"});
    struct str test9 = str((char []) {"\177"});
    struct str test10 = str((char []) {"\"hello\177"});
    struct str test11 = str((char []) {"\n   /\n"});
    struct toker_res unexpected1[] = {
        {{BRC_UNEXCHAR, 1, 1}, { 0 }}};
    struct toker_res unexpected2[] = {
        {{BRC_UNEXCHAR, 1, 7}, { 0 }}};
    struct toker_res unexpected3[] = {
        {{BRC_UNEXCHAR, 2, 4}, { 0 }}};
    struct str test12 = str((char []) {"\"hello world"});
    struct toker_res unterm[] = {
        {{BRC_UNTERM, 1, 1}, { 0 }}};
    struct str test13 = str((char []) {"        "});
    struct str test14 = str((char []) {"/* */   /*\n\n  */ ////    \n"});
    struct str test15 = str((char []) {"   \"\\k\""});
    struct toker_res badesc[] = {
        {{BRC_BADESC, 1, 6}, { 0 }}};
    return test("tok test 1", test1, expected1, countof(expected1))
        && test("tok test 2", test2, expected2, countof(expected2))
        && test("tok test 3", test3, outrange, countof(outrange))
        && test("tok test 4", test4, outrange, countof(outrange))
        && test("tok test 5", test5, unexpected1, countof(unexpected1))
        && test("tok test 6", test6, unexpected1, countof(unexpected1))
        && test("tok test 7", test7, unexpected1, countof(unexpected1))
        && test("tok test 8", test8, unexpected1, countof(unexpected1))
        && test("tok test 9", test9, unexpected1, countof(unexpected1))
        && test("tok test 10", test10, unexpected2, countof(unexpected2))
        && test("tok test 11", test11, unexpected3, countof(unexpected3))
        && test("tok test 12", test12, unterm, countof(unterm))
        && test("tok test 13", test13, expected1, 0)
        && test("tok test 14", test14, expected1, 0)
        && test("tok test 15", test15, badesc, countof(badesc));
}
