#include <stdio.h>
#include "arena.h"
#include "str.h"
#include "bson.h"
#include "bson_int.h"
#include "bson_str.h"
#include "test_utils.h"
#include "test_parse.h"

#define LOG_RES(r)       fprintf(stderr, "%s %d %d", rc_to_str((r).rc), (r).line_num, (r).col_num)
#define APPEND_STR(b, s) append(b, s.data, s.len)
#define MEMBUF(buf, cap) { buf, cap, 0, 0 }

struct buf
{
    char *buf;
    int cap;
    int len;
    bool error;
};

struct parse_test
{
    const char *name;
    struct str input;
    struct bson_res res;
    bool ignore_rc;
};

static void bson_entity_to_str(struct bson_type *btype, struct buf *buf);

static void append(struct buf *buf, char *src, int len)
{
    int avail = buf->cap - buf->len;
    int amount = avail < len ? avail : len;
    for (int i = 0; i < amount; i++)
    {
        buf->buf[buf->len + i] = src[i];
    }
    buf->len += amount;
    buf->error |= amount < len;
}

static void append_int(struct buf *buf, int x)
{
    char tmp[12];
    char *end = tmp + sizeof(tmp);
    char *beg = end;
    int t = x > 0 ? -x : x;
    do
    {
        *--beg = (char) ('0' - t % 10);
    } while (t /= 10);
    if (x < 0)
    {
        *--beg = '-';
    }
    append(buf, beg, (int) (end - beg));
}

static void bson_obj_to_str(struct bson_obj *obj, struct buf *buf)
{
    APPEND_STR(buf, str("{"));
    for (struct bson_obj *cur = bson_obj_iter_begin(obj); cur;)
    {
        struct  str key = bson_obj_iter_key(cur);
        APPEND_STR(buf, str("\""));
        APPEND_STR(buf, key);
        APPEND_STR(buf, str("\":"));
        bson_entity_to_str(bson_obj_get(obj, key), buf);
        cur = bson_obj_iter_next(cur);
        if (cur)
        {
            APPEND_STR(buf, str(","));
        }
    }
    APPEND_STR(buf, str("}"));
    
}

static void bson_list_to_str(struct bson_list *list, struct buf *buf)
{
    APPEND_STR(buf, str("["));
    for (int i = 0; i < bson_list_len(list); i++)
    {
        bson_entity_to_str(bson_list_get(list, i), buf);
        if (i < bson_list_len(list) - 1)
        {
            APPEND_STR(buf, str(","));
        }
    }
    APPEND_STR(buf, str("]"));
}

static void bson_entity_to_str(struct bson_type *btype, struct buf *buf)
{
    switch (btype->type)
    {
    case BTYPE_OBJ:
    {
        bson_obj_to_str((struct bson_obj *) btype, buf);
        break;
    }
    case BTYPE_LIST:
    {
        bson_list_to_str((struct bson_list *) btype, buf);
        break;
    }
    case BTYPE_INT:
    {
        append_int(buf, ((struct bson_int *) btype)->val);
        break;
    }
    case BTYPE_STR:
    {
        APPEND_STR(buf, str("\""));
        APPEND_STR(buf, ((struct bson_str *) btype)->val);
        APPEND_STR(buf, str("\""));
        break;
    }
    }
}
    
static bool test(struct arena *arena, struct parse_test *test)
{
    struct bson_obj *obj;
    struct bson_res res;
    char output[1024];
    struct buf buf = MEMBUF(output, sizeof(output));
    
    res = bson_parse(arena, test->input, &obj);
    if (test->res.rc != res.rc
        || (!test->ignore_rc
            && (test->res.line_num != res.line_num || test->res.col_num != res.col_num)))
    {
        fprintf(stderr, "%s: actual: ", test->name);
        LOG_RES(res);
        fprintf(stderr, ", expected: ");
        LOG_RES(test->res);
        fprintf(stderr, "\n");
        return false;
    }
    if (res.rc == BRC_SUCCESS)
    {
        bson_obj_to_str(obj, &buf);
        if (!strequals(test->input, (struct str) {buf.buf, buf.len}))
        {
            fprintf(stderr, "%s: actual: %.*s, expected: %.*s\n", test->name,
                    buf.len, buf.buf,
                    test->input.len, test->input.data);
            return false;
        }
    }
    return true;
}

static bool test_no_mem(void)
{
    int sz;
    struct bson_res res;
    struct bson_obj *obj;
    struct arena arena;
    
    for (sz = 1; sz < 361; sz++)
    {
        if (!new_arena(&arena, sz))
        {
            fprintf(stderr, "parse test: out of memory\n");
            return false;
        }
        res = bson_parse(&arena, str("{\"a\":[{\"b\":1}]}"), &obj);
        if (res.rc != BRC_NOMEM)
        {
            break;
        }
    }
    return res.rc == BRC_SUCCESS && sz == 360;
}

bool test_parse(void)
{
    struct parse_test tests[] = 
    {
        {"parse test 1",  str("{}"),                      {BRC_SUCCESS, 1, 2 }, false},
        {"parse test 2",  str("{\"a\":\"b\"}"),           {BRC_SUCCESS, 1, 9 }, false},
        {"parse test 3",  str("{\"a\":1}"),               {BRC_SUCCESS, 1, 7 }, false},
        {"parse test 4",  str("{\"a\":{\"a\":\"b\"}}"),   {BRC_SUCCESS, 1, 15}, false},
        {"parse test 5",  str("{\"a\":[\"b\"]}"),         {BRC_SUCCESS, 1, 11}, false},
        {"parse test 6",  str("{\"a\":[1]}"),             {BRC_SUCCESS, 1, 9 }, false},
        {"parse test 7",  str("{\"a\":[\"b\"]}"),         {BRC_SUCCESS, 1, 11}, false},
        {"parse test 8",  str("{\"a\":[{\"b\":\"c\"}]}"), {BRC_SUCCESS, 1, 17}, false},
        {"parse test 9",  str("{\"a\":[[\"b\"]]}"),       {BRC_SUCCESS, 1, 13}, false},
        {"parse test 10", str("{\"a\":1,\"b\":2}"),       {BRC_SUCCESS, 1, 13}, false},
        {"parse test 11", str("{\"a\":[1,2]}"),           {BRC_SUCCESS, 1, 11}, false},
        {"parse test 12", str(""),                        {BRC_EOS,     0, 0 }, true },
        {"parse test 13", str("x"),                       {BRC_SYNERR,  1, 1 }, false},
        {"parse test 14", str("}"),                       {BRC_SYNERR,  1, 1 }, false},
        {"parse test 15", str("1"),                       {BRC_SYNERR,  1, 1 }, false},
        {"parse test 16", str("\"a\""),                   {BRC_SYNERR,  1, 1 }, false},
        {"parse test 17", str("{"),                       {BRC_EOS,     0, 0 }, true },
        {"parse test 18", str("{x"),                      {BRC_SYNERR,  1, 2 }, false},
        {"parse test 19", str("{]"),                      {BRC_SYNERR,  1, 2 }, false},
        {"parse test 20", str("{1"),                      {BRC_SYNERR,  1, 2 }, false},
        {"parse test 21", str("{\"a\""),                  {BRC_EOS,     0, 0 }, false},
        {"parse test 22", str("{\"a\"x"),                 {BRC_SYNERR,  1, 5 }, false},
        {"parse test 23", str("{\"a\"["),                 {BRC_SYNERR,  1, 5 }, false},
        {"parse test 24", str("{\"a\"1"),                 {BRC_SYNERR,  1, 5 }, false},
        {"parse test 25", str("{\"a\"\"b\""),             {BRC_SYNERR,  1, 5 }, false},
        {"parse test 26", str("{\"a\":"),                 {BRC_EOS,     0, 0 }, true },
        {"parse test 27", str("{\"a\":x"),                {BRC_SYNERR,  1, 6 }, false},
        {"parse test 28", str("{\"a\":]"),                {BRC_SYNERR,  1, 6 }, false},
        {"parse test 29", str("{\"a\":1"),                {BRC_EOS,     0, 0 }, true },
        {"parse test 30", str("{\"a\":\"b\""),            {BRC_EOS,     0, 0 }, true },
        {"parse test 31", str("{\"a\":\"b\"x"),           {BRC_SYNERR,  1, 9 }, false},
        {"parse test 32", str("{\"a\":\"b\"]"),           {BRC_SYNERR,  1, 9 }, false},
        {"parse test 33", str("{\"a\":\"b\"1"),           {BRC_SYNERR,  1, 9 }, false},
        {"parse test 34", str("{\"a\":\"b\",\"c\""),      {BRC_EOS,     0, 0 }, true },
        {"parse test 35", str("{\"a\":\"b\"}x"),          {BRC_SYNERR,  1, 10}, false},
        {"parse test 36", str("{\"a\":\"b\"}{"),          {BRC_SYNERR,  1, 10}, false},
        {"parse test 37", str("{\"a\":\"b\"}1"),          {BRC_SYNERR,  1, 10}, false},
        {"parse test 38", str("{\"a\":\"b\"}\"c\""),      {BRC_SYNERR,  1, 10}, false},
        {"parse test 39", str("{\"a\":["),                {BRC_EOS,     0, 0 }, true },
        {"parse test 40", str("{\"a\":[x"),               {BRC_SYNERR,  1, 7 }, false},
        {"parse test 41", str("{\"a\":[}"),               {BRC_SYNERR,  1, 7 }, false},
        {"parse test 42", str("{\"a\":[1"),               {BRC_EOS,     0, 0 }, true },
        {"parse test 43", str("{\"a\":[\"b\""),           {BRC_EOS,     0, 0 }, true },
        {"parse test 44", str("{\"a\":[\"b\"x"),          {BRC_SYNERR,  1, 10}, false},
        {"parse test 45", str("{\"a\":[\"b\"["),          {BRC_SYNERR,  1, 10}, false},
        {"parse test 46", str("{\"a\":[\"b\"1"),          {BRC_SYNERR,  1, 10}, false},
        {"parse test 47", str("{\"a\":[\"b\"\"c\""),      {BRC_SYNERR,  1, 10}, false},
        {"parse test 48", str("{\"a\":[\"b\","),          {BRC_EOS,     0, 0 }, true},
        {"parse test 49", str("{\"a\":[\"b\",x"),         {BRC_SYNERR,  1, 11}, false},
        {"parse test 50", str("{\"a\":[\"b\",}"),         {BRC_SYNERR,  1, 11}, false},
        {"parse test 51", str("{\"a\":[\"b\",1"),         {BRC_EOS,     0, 0 }, true },
        {"parse test 52", str("{\"a\":[\"b\",\"c\""),     {BRC_EOS,     0, 0 }, true },
        {"parse test 53", str("{\"a\":[\"b\"]"),          {BRC_EOS,     0, 0 }, true },
        {"parse test 54", str("{\"a\":[\"b\"]x"),         {BRC_SYNERR,  1, 11}, false},
        {"parse test 55", str("{\"a\":[\"b\"]{"),         {BRC_SYNERR,  1, 11}, false},
        {"parse test 56", str("{\"a\":[\"b\"]1"),         {BRC_SYNERR,  1, 11}, false},
        {"parse test 57", str("{\"a\":[\"b\"]\"c\""),     {BRC_SYNERR,  1, 11}, false},
        {"parse test 58", str("{\"a"),                    {BRC_UNTERM,  1, 2 }, false},
        {"parse test 59", str("{\"a\":\"b"),              {BRC_UNTERM,  1, 6 }, false},
        {"parse test 60", str("{\"a\":2147483648"),       {BRC_OUTRANGE,1, 6 }, false},
        {"parse test 61", str("{\"a\":[\"b\""),           {BRC_EOS,     0, 0 }, true },
        {"parse test 62", str("{\"a\":[2147483648"),      {BRC_OUTRANGE,1, 7 }, false},
        {"parse test 63", str("{\"a\":[\"b\177\""),       {BRC_BADCHAR, 1, 9 }, false},
        {"parse test 64", str("{\"a\":[\"b\\t\""),        {BRC_BADESC,  1, 10}, false},
        {"out of memory", str("{\"a\":[{\"b\":1}]}"),     {BRC_SUCCESS, 1, 15}, false},
    };        
    struct arena arena;

    if (!new_arena(&arena, 1 << 14))
    {
        fprintf(stderr, "parse test: out of memory\n");
        return false;
    }

    for (int i = 0; i < countof(tests) - 1; i++)
    {
        if (!test(&arena, tests + i))
        {
            return false;
        }
    }
    return (test_no_mem());
}
