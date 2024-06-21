#include <assert.h>
#include "str.h"

bool isspacec(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f'
        || c == '\r';
}

bool isdigitc(char c)
{
    return c >= '0' && c <= '9';
}

bool isprintc(char c)
{
    return c >= ' ' && c <= '~';
}

bool strequals(struct str s1, struct str s2)
{
    return s1.len == s2.len
        && (!s1.len || !memcmp(s1.data, s2.data, (usize) s1.len));
}

u64 strhash(struct str s)
{
    u64 h = 0x100;
    
    for (int i = 0; i < s.len; i++)
    {
        h ^= (u8) s.data[i];
        h *= 1111111111111111111u;
    }
    return h;
}
