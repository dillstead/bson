#include <stdlib.h>
#include <stdio.h>
#include "test_arena.h"
#include "test_tok.h"
#include "test_types.h"
#include "test_parse.h"

int main(void)
{
    if (test_toker() && test_arena() && test_types() && test_parse())
    {
        printf("All tests passed\n");
    }
    return EXIT_SUCCESS;
}
