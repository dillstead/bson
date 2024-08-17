#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "./include/bson.h"

#define APPEND_STR(b, s) append(b, s.data, s.len)
#define MEMBUF(buf, cap) { buf, cap, 0, 0 }
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define lengthof(s)  (countof(s) - 1)
#define str(s) (struct str) {(char *) s, (int) lengthof(s)}

struct buf
{
    char *buf;
    int cap;
    int len;
    bool error;
};

static const char *rc_to_str(enum brc rc)
{
    switch (rc)
    {
    case BRC_SUCCESS:
    {
        return "Success";
    }
    case BRC_NOMEM:
    {
        return "Out of memory";
    }
    case BRC_BADSTATE:
    {
        return "Bad state";
    }
    case BRC_BADESC:
    {
        return "Bad escape";
    }
    case BRC_BADCHAR:
    {
        return "Bad char";
    }
    case BRC_UNTERM:
    {
        return "Unterminated string";
    }
    case BRC_OUTRANGE:
    {
        return "Out of range";
    }
    case BRC_EOS:
    {
        return "End of string";
    }
    case BRC_SYNERR:
    {
        return "Syntax error";
    }
    case BRC_TOODEEP:
    {
        return "maximun nesting exceeded";
    }
    default:
    {
        return "Unknown";
    }
    }
}

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
        append_int(buf, (bson_int_get_val((struct bson_int *) btype)));
        break;
    }
    case BTYPE_STR:
    {
        APPEND_STR(buf, str("\""));
        APPEND_STR(buf, (bson_str_get_val((struct bson_str *) btype)));
        APPEND_STR(buf, str("\""));
        break;
    }
    }
}

int main(int argc, char **argv)
{
    FILE *file = NULL;
    char *buffer = NULL;
    char *output = NULL;
    struct arena arena;
    int rc = EXIT_FAILURE;

    if (!new_arena(&arena, 1 << 14))
    {
        fprintf(stderr, "Out of memory\n");
        return EXIT_FAILURE;
    }
    
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long file_sz = ftell(file);
    if (file_sz < 0)
    {
        perror("Error getting file size");
        goto cleanup;
    }
    if (file_sz > INT_MAX)
    {
        fprintf(stderr, "File too large\n");
        goto cleanup;
    }
    fseek(file, 0, SEEK_SET);

    buffer = malloc((size_t) file_sz);
    output = malloc((size_t) file_sz);
    if (buffer == NULL || output == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        goto cleanup;
    }

    size_t bytes_read = fread(buffer, 1, (size_t) file_sz, file);
    if (bytes_read != (size_t) file_sz)
    {
        fprintf(stderr, "Error reading file\n");
        goto cleanup;
    }

    struct bson_obj *obj;
    struct bson_res res;
    struct buf buf = MEMBUF(output, (int) file_sz);

    res = bson_parse(&arena, (struct str) { buffer, (int) bytes_read }, &obj);
    if (res.rc == BRC_SUCCESS)
    {
        bson_obj_to_str(obj, &buf);
        if (!buf.error)
        {
            fprintf(stdout, "%.*s\n", buf.len, buf.buf);
        }
        else
        {
            fprintf(stderr, "Error writing object\n");
            goto cleanup;
        }
    }
    else
    {
        fprintf(stderr, "%s row %d col %d\n",
                rc_to_str(res.rc), res.line_num, res.col_num);
    }
    rc = EXIT_SUCCESS;

cleanup:
    free(output);
    free(buffer);
    fclose(file);
    return rc;
}
