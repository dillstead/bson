#include <stdlib.h>
#include <stdio.h>
#include "test_tok.h"
#include "test_types.h"

int main(void)
{
    if (test_toker() && test_types())
    {
        printf("All tests passed\n");
    }
    return EXIT_SUCCESS;
}
