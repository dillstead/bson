#include <limits.h>
#include "toker.h"

static bool symbol_set[128] = {
    ['{'] = true,
    ['}'] = true,
    ['['] = true,
    [']'] = true,
    [':'] = true,
    [','] = true
};

static char escape_set[128] = {
    ['\\'] = '\\',
    ['n'] = '\n',
    ['"'] = '"'
};

static bool next_char(struct toker *toker, char *c)
{
    if (toker->pos < toker->str.len)
    {
        toker->col_num = toker->next_col_num;
        *c = toker->str.data[toker->pos++];
        if (*c == '\n')
        {
            toker->line_num++;
            toker->next_col_num = 1;
        }
        else
        {
            toker->next_col_num++;
        }
        return true;
    }
    return false;
}

static bool peek_char(struct toker *toker, char *c)
{
    if (toker->pos < toker->str.len)
    {
        *c = toker->str.data[toker->pos];
        return true;
    }
    return false;
}

void init_toker(struct toker *toker, struct str str)
{
    memset(toker, 0, sizeof *toker);
    toker->str = str;
    toker->state = TOK_STATE_WS;
    toker->line_num = 1;
    toker->col_num = 1;
    toker->next_col_num = 1;
}

bool has_next_tok(struct toker *toker)
{
    char prevc = '\0';
    char c;

    if (toker->state == TOK_STATE_ERR)
    {
        return false;
    }
    while (toker->state != TOK_STATE_TOK && next_char(toker, &c))
    {
        switch (toker->state)
        {
        case TOK_STATE_WS:
        {
            if (!isspacec(c))
            {
                if (c == '/')
                {
                    toker->state = TOK_STATE_MCMNT;
                }
                else
                {
                    toker->pos--;
                    toker->next_col_num--;
                    toker->state = TOK_STATE_TOK;
                }
            }
            break;
        }
        case TOK_STATE_MCMNT:
        {
            if (c == '/')
            {
                toker->state = TOK_STATE_SLCMNT;
            }
            else if (c == '*')
            {
                toker->state = TOK_STATE_MLCMNT;
            }
            else
            {
                if (c == '\n')
                {
                    toker->line_num--;
                    toker->next_col_num = toker->col_num - 1;
                }
                else
                {
                    toker->next_col_num -= 2;
                }
                toker->pos -= 2;
                toker->state = TOK_STATE_TOK;
            }
            break;
        }
        case TOK_STATE_SLCMNT:
        {
            if (c == '\n')
            {
                toker->state = TOK_STATE_WS;
            }
            break;
        }
        case TOK_STATE_MLCMNT:
        {
            if (prevc == '*' && c == '/')
            {
                toker->state = TOK_STATE_WS;
            }
            break;
        }
        default:
        {
            break;
        }
        }
        prevc = c;
    }
    return toker->state == TOK_STATE_TOK;
}

struct bson_res next_tok(struct toker *toker, struct tok *tok)
{
    bool is_neg = false;
    i64 int_val = 0;
    char c = '\0';
    bool is_escape = false;
    int start = 0;
    size str_pos;
    struct bson_res res = { BRC_BADSTATE, 0, 0 };

    if (toker->state != TOK_STATE_TOK)
    {
        res.line_num = toker->line_num;
        res.col_num = toker->col_num;
        return res;
    }

    memset(tok, 0, sizeof *tok);
    // State is TOK, there's at least one more character left
    next_char(toker, &c);
    res.line_num = toker->line_num;
    res.col_num = toker->col_num;

    if (!isprintc(c))
    {
        toker->state = TOK_STATE_ERR;
        res.rc = BRC_BADCHAR;
        return res;
    }

    if (symbol_set[(int) c])
    {
        tok->type = TOK_TYPE_SYM;
        tok->str_val.data = toker->str.data + toker->pos - 1;
        tok->str_val.len = 1;
        toker->state = TOK_STATE_WS;
    }
    else if (c == '-' || isdigitc(c))
    {
        start = toker->col_num;
        if (c == '-')
        {
            is_neg = true;
            if (!peek_char(toker, &c) || !isdigitc(c))
            {
                toker->state = TOK_STATE_ERR;
                res.rc = isprintc(c) ? BRC_SYNERR : BRC_BADCHAR;
                return res;
            }
            next_char(toker, &c);
        }
        tok->type = TOK_TYPE_INT;
        int_val = c - '0';
        toker->state = TOK_STATE_INT;
    }
    else if (c == '"')
    {
        start = toker->col_num;
        str_pos = 0;
        tok->type = TOK_TYPE_STR;
        tok->str_val.data = toker->str.data + toker->pos;
        toker->state = TOK_STATE_STR;
    }
    else
    {
        toker->state = TOK_STATE_ERR;
        res.rc = isprintc(c) ? BRC_SYNERR: BRC_BADCHAR;
        return res;
    }

    while (toker->state != TOK_STATE_WS)
    {
        switch (toker->state)
        {
        case TOK_STATE_INT:
        {
            if (peek_char(toker, &c) && isdigitc(c))
            {
                next_char(toker, &c);
                int_val *= 10;
                int_val += c - '0';

                if (int_val > (i64) INT_MAX + 1
                    || (!is_neg && int_val > INT_MAX))
                {
                    toker->state = TOK_STATE_ERR;
                    res.rc = BRC_OUTRANGE;
                    res.col_num = start;
                    return res;
                }
            }
            else
            {
                tok->int_val = (int) (is_neg ? -int_val : int_val);
                toker->state = TOK_STATE_WS;
                res.col_num = start;
            }
            break;
        }
        case TOK_STATE_STR:
        {
            if (!next_char(toker, &c))
            {
                toker->state = TOK_STATE_ERR;
                res.rc = BRC_UNTERM;
                res.col_num = start;
                return res;
            }
            if (is_escape)
            {
                char esc = escape_set[(int) c];
                if (!esc)
                {
                    toker->state = TOK_STATE_ERR;
                    res.rc = BRC_BADESC;
                    res.col_num = toker->col_num;
                    return res;
                }
                tok->str_val.data[str_pos++] = esc;
                tok->str_val.len++;
                is_escape = false;
            }
            else if (c == '\\')
            {
                is_escape = true;
            }
            else if (c == '"')
            {
                toker->state = TOK_STATE_WS;
                res.col_num = start;
            }
            else if (isprintc(c))
            {
                tok->str_val.data[str_pos++] = c;
                tok->str_val.len++;
                res.col_num = toker->col_num;
            }
            else
            {
                toker->state = TOK_STATE_ERR;
                res.rc = BRC_BADCHAR;
                res.col_num = toker->col_num;
                return res;
            }
            break;
        }
        default:
        {
            break;
        }
        }
    }

    res.rc = BRC_SUCCESS;
    return res;
}
