#ifndef TRACE_H
#define TRACE_H

#include <stdint.h>

#define TRACE_MAX_ENTRIES 1024
#define IDE_SIZE 32

typedef struct
{
    uint32_t api_id;

    char var1[IDE_SIZE];
    char var2[IDE_SIZE];
    char result[IDE_SIZE];

    uint64_t timestamp;

} trace_entry;

void trace_init();

void trace_add(
    uint32_t api_id,
    const char *var1,
    const char *var2,
    const char *result
);

int trace_export(unsigned char *buffer, int max_size);

#endif