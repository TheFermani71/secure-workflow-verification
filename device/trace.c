#include "trace.h"
#include "crypto.h"

#include <string.h>
#include <time.h>

static trace_entry trace_buffer[TRACE_MAX_ENTRIES];

static uint32_t trace_index = 0;


/*
Inizializza la trace.
*/
void trace_init()
{
    trace_index = 0;
}


/*
Aggiunge una entry alla trace.
*/
void trace_add(
    uint32_t api_id,
    const char *var1,
    const char *var2,
    const char *result
)
{
    if(trace_index >= TRACE_MAX_ENTRIES)
        return;

    trace_entry *e = &trace_buffer[trace_index];

    e->api_id = api_id;

    memset(e->var1,0,IDE_SIZE);
    memset(e->var2,0,IDE_SIZE);
    memset(e->result,0,IDE_SIZE);

    if(var1) strncpy(e->var1,var1,IDE_SIZE-1);
    if(var2) strncpy(e->var2,var2,IDE_SIZE-1);
    if(result) strncpy(e->result,result,IDE_SIZE-1);

    e->timestamp = (uint64_t)time(NULL);

    trace_index++;
}


/*
Esporta la trace cifrata.
*/
int trace_export(unsigned char *buffer, int max_size)
{
    int size = trace_index * sizeof(trace_entry);

    if(size > max_size)
        return -1;

    crypto_encrypt(
        (unsigned char*)trace_buffer,
        size,
        buffer,
        max_size
    );

    return size;
}