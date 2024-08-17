#include <stdlib.h>
#include <stdio.h>
#include "./include/bson.h"
#include "test_arena.h"
#include "test_tok.h"
#include "test_types.h"
#include "test_parse.h"

int main(int argc, char **argv)
{
    (void) argv;
    
    if (argc == 1)
    {
        if (test_toker() && test_arena() && test_types() && test_parse())
        {
            printf("All tests passed\n");
        }
    }
    else
    {
        struct arena arena;

        if (!new_arena(&arena, 1 << 14))
        {
            return EXIT_FAILURE;
        }
        while (!feof(stdin))
        {
            char buffer[1 << 20];
            struct bson_obj *obj;
            size_t bytes_read = fread(buffer, 1, sizeof buffer, stdin);

            if (bytes_read == 0)
            {
                if (ferror(stdin))
                {
                    return EXIT_FAILURE;
                }
                break;
            }
            bson_parse(&arena, (struct str) { buffer, (int) bytes_read }, &obj);
        }
    }
    return EXIT_SUCCESS;
}
