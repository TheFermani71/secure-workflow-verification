#ifndef INT_VAR_H
#define INT_VAR_H

#include <stdint.h>

#define IDE_SIZE 32

#define INTERNAL_VAR_SIZE 44
#define INT_VAR_SIZE 72

typedef unsigned char int_var[INT_VAR_SIZE];

void int_var_init(int_var v, const char *ide);

void int_var_assign(int_var v, int value);

int int_var_get(const int_var v);

#endif