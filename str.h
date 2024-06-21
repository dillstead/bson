#ifndef STR_H
#define STR_H

#include <string.h>
#include "types.h"
#include "bson.h"

#define str(s) (struct str) {(char *) s, (int) lengthof(s)}

bool isspacec(char c);
bool isdigitc(char c);
bool isprintc(char c);
bool strequals(struct str s1, struct str s2);
bool strisempty(struct str s);
u64 strhash(struct str s);

#endif


