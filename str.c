#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "str.h"

bool xisspace(u8 c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

bool xisdigit(u8 c)
{
    return c >= '0' && c <= '9';
}

bool xisprint(u8 c)
{
    return c >= ' ' && c <= '~';
}

static bool cmp(u8 *data1, u8 *data2, size len)
{
    for (size i = 0; i < len; i++)
    {
        if (data1[i] != data2[i])
        {
            return false;
        }
    }
    return true;
}
bool s8cmp(struct s8 s1, struct s8 s2)
{
    assert(s1.len >= 0 && s2.len >= 0);
    
    return s1.len == s2.len && cmp(s1.data, s2.data, s1.len);
}
