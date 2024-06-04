#include "print.h"
#include <stdio.h>
#include <inttypes.h>

void print_line(int64_t number, char *string)
{
    (void) number;
    (void) string;
    //print out the number and the string using cross platform format for the integer
    printf("%" PRId64 " %s \n", number, string);
}
