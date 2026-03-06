#ifndef VERIFIER_H
#define VERIFIER_H

#include <stdint.h>

#define MAX_TRACE 1024
#define MAX_API   16

typedef struct
{
    uint32_t api_sequence[MAX_TRACE];
    uint32_t length;

} execution_trace;


void verifier_init();

void verifier_add_edge(uint32_t from, uint32_t to);

int verifier_check_trace(execution_trace *trace);

#endif