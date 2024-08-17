#include "test_utils.h"

const char *rc_to_str(enum brc rc)
{
    switch (rc)
    {
    case BRC_SUCCESS:
    {
        return "success";
    }
    case BRC_NOMEM:
    {
        return "out of memory";
    }
    case BRC_BADSTATE:
    {
        return "bad state";
    }
    case BRC_BADESC:
    {
        return "bad escape";
    }
    case BRC_BADCHAR:
    {
        return "bad char";
    }
    case BRC_UNTERM:
    {
        return "unterminated string";
    }
    case BRC_OUTRANGE:
    {
        return "out of range";
    }
    case BRC_EOS:
    {
        return "end of string";
    }
    case BRC_SYNERR:
    {
        return "syntax error";
    }
    case BRC_TOODEEP:
    {
        return "maximun nesting exceeded";
    }
    default:
    {
        return "unknown";
    }
    }
}
