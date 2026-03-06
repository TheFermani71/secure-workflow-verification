#ifndef API_H
#define API_H

#include "int_var.h"

/*
API identifiers used in the execution trace
*/

#define API_INIT   1
#define API_ASSIGN 2
#define API_ADD    3
#define API_DIV    4


/*
Initializes a secure variable
*/

void api_init(int_var v, const char *ide);


/*
Assigns a constant value
*/

void api_assign(int_var v, int value);


/*
Adds two secure variables
*/

void api_add(int_var a, int_var b, int_var result);


/*
Divides two secure variables
*/

void api_div(int_var a, int_var b, int_var result);


#endif