#ifndef STR_H
#define STR_H

#include <stdbool.h>
#include "types.h"
#include "arena.h"

#define s8(s) (struct s8){(u8 *) s, lengthof(s)}

struct s8
{
    u8  *data;
    size len;
};

bool xisspace(u8 c);
bool xisdigit(u8 c);
bool xisprint(u8 c);
bool s8cmp(struct s8 s1, struct s8 s2);

#endif


