#include <string.h>
#include <limits.h>
#include "toker.h"

static bool symbol_set[] = {
    ['{'] = true,
    ['}'] = true,
    ['['] = true,
    [']'] = true,
    [':'] = true,
    [','] = true
};

static bool next_char(struct toker *toker, u8 *c)
{
    if (toker->pos < toker->str.len)
    {
        *c = toker->str.data[toker->pos++];
        if (*c == '\n')
        {
            toker->line_num++;
        }
        return true;
    }
    return false;
}

static bool peek_char(struct toker *toker, u8 *c)
{
    if (toker->pos < toker->str.len)
    {
        *c = toker->str.data[toker->pos];
        return true;
    }
    return false;
}

const char *type_to_str(enum tok_type type)
{
    switch (type)
    {
    case T_ERR:
    {
        return "error";
    }
    case T_SYM:
    {
        return "sym";
    }
    case T_INT:
    {
        return "int";
    }
    case T_STR:
    {
        return "str";
    }
    default:
    {
        return "unkown";
    }
    }
}

void init_toker(struct toker *toker, struct s8 str)
{
    memset(toker, 0, sizeof *toker);
    toker->str = str;
    toker->state = S_WS;
    toker->line_num = 1;
}

bool has_next_tok(struct toker *toker)
{
    u8 prevc = '\0';
    u8 c;
    
    while (toker->state != S_TOK && next_char(toker, &c))
    {
        switch (toker->state)
        {
        case S_WS:
        {
            if (!xisspace(c))
            {
                if (c == '/')
                {
                    toker->state = S_MCMNT;
                }
                else
                {
                    toker->pos--;
                    toker->state = S_TOK;
                }
            }
            break;
        }
        case S_MCMNT:
        {
            if (c == '/')
            {
                toker->state = S_SLCMNT;
            }
            else if (c == '*')
            {
                toker->state = S_MLCMNT;
            }
            else
            {
                toker->pos -= 2;
                toker->state = S_TOK;
            }
            break;
        }
        case S_SLCMNT:
        {
            if (c == '\n')
            {
                toker->state = S_WS;
            }
            break;
        }
        case S_MLCMNT:
        {
            if (prevc == '*' && c == '/')
            {
                toker->state = S_WS;
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
    return toker->state == S_TOK;
}

struct tok next_tok(struct toker *toker)
{
    bool is_neg = false;
    i64 int_val = 0;
    struct tok tok = { .type = T_ERR };
    u8 c = 0;

    if (toker->state != S_TOK)
    {
        tok.line_num = toker->line_num;
        tok.int_val = E_STATE;
        return tok;
    }

    // State is TOK, there's at least one more character left
    next_char(toker, &c);
    tok.line_num = toker->line_num;

    if (symbol_set[c])
    {
        tok.type = T_SYM;
        tok.str_val.data = toker->str.data + toker->pos - 1;
        tok.str_val.len = 1;
        toker->state = S_WS;
    }
    else if (c == '-' || xisdigit(c))
    {
        if (c == '-')
        {
            is_neg = true;
            if (!peek_char(toker, &c) || !xisdigit(c))
            {
                tok.int_val = E_UNEXCHAR;
                toker->state = S_WS;
                return tok;
            }
            next_char(toker, &c);
        }
        tok.type = T_INT;
        int_val = c - '0';
        toker->state = S_INT;
    }
    else if (c == '"')
    {
        tok.type = T_STR;
        tok.str_val.data = toker->str.data + toker->pos;
        toker->state = S_STR;
    }
    else
    {
        tok.int_val = E_UNEXCHAR;
        toker->state = S_WS;
        return tok;
    }

    while (toker->state != S_WS)
    {
        switch (toker->state)
        {
        case S_INT:
        {
            if (next_char(toker, &c) && xisdigit(c))
            {
                int_val *= 10;
                int_val += c - '0';

                if (int_val > (i64) INT_MAX + 1
                    || (!is_neg && int_val > INT_MAX))
                {
                    tok.type = T_ERR;
                    tok.int_val = E_TOOLARGE;
                    tok.line_num = toker->line_num;
                    toker->state = S_WS;
                }
            }
            else
            {
                toker->pos--;
                tok.int_val = (int) (is_neg ? -int_val : int_val);
                tok.line_num = toker->line_num;
                toker->state = S_WS;
            }
            break;
        }
        case S_STR:
        {
            if (!next_char(toker, &c))
            {
                tok.type = T_ERR;
                tok.int_val = E_UNTERM;
                tok.line_num = toker->line_num;
                toker->state = S_WS;
            }
            if (c == '"')
            {
                tok.line_num = toker->line_num;
                toker->state = S_WS;
            }
            else
            {
                tok.str_val.len++;
            }
            break;
        }
        default:
        {
            break;
        }
        }
    }
    return tok;
}
