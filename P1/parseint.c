#include "parseint.h"
#include <stdio.h>

/*
 * Returns the value of c or -1 on error
 */
int parseDecimalChar(char c)
{
    (void)c;
    //Subract the ASCII value of 0 from the character to get the integer value
    //ASCII range for [0-9] is [48-57]
    int i = c - '0';
    //if the output is an integer between 0-9 return that value, else return -1
    if (i >= 0 && i < 10) 
    { 
        return i; 
    }
    else 
    { 
        return -1; 
    }
}

/*
 * Parses a non-negative integer, interpreted as octal when starting with 0,
 * decimal otherwise. Returns -1 on error.
 */
int parseInt(char *string)
{
    (void)string;
    int i;
    int result = 10;
    int count = 0;

    //iterate through each characther in the string and check if subracting the ASCII value of 0 
    //returns a decimal number: [0-9]. If all the characters are a decimal numbers return 10, unless the
    //first character is a 0, then return 8. If a character outside of the scope of [0-9] is found
    //in the string than return -1 
    while(*string != '\0')
	{
        i = *string - '0';

        if (i == 0 && count == 0)
        {
            result = 8;
        }
        if (i < 0 || i > 9)
        {
            result = -1;
        }
        //increase the pointer value to iterate through the string
        string++;
        count++;
    }

    return result;
}
