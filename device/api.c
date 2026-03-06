#include "api.h"
#include "trace.h"
#include "int_var.h"

#include <stddef.h>

/*
Initializes a variable and logs the operation
*/

void api_init(int_var v, const char *ide)
{
    int_var_init(v, ide);

    trace_add(
        API_INIT,
        ide,
        NULL,
        NULL
    );
}


/*
Assigns a value to a variable
*/

void api_assign(int_var v, int value)
{
    int_var_assign(v, value);

    trace_add(
        API_ASSIGN,
        NULL,
        NULL,
        NULL
    );
}


/*
Adds two encrypted variables
*/

void api_add(int_var a, int_var b, int_var result)
{
    int value_a = int_var_get(a);
    int value_b = int_var_get(b);

    int sum = value_a + value_b;

    int_var_assign(result, sum);

    trace_add(
        API_ADD,
        "a",
        "b",
        "result"
    );
}


/*
Divides two encrypted variables
*/

void api_div(int_var a, int_var b, int_var result)
{
    int value_a = int_var_get(a);
    int value_b = int_var_get(b);

    if(value_b == 0)
        return;

    int res = value_a / value_b;

    int_var_assign(result, res);

    trace_add(
        API_DIV,
        "a",
        "b",
        "result"
    );
}