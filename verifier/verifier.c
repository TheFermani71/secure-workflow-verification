#include "verifier.h"

static int workflow_graph[MAX_API][MAX_API];

void verifier_init()
{
    for(int i=0;i<MAX_API;i++)
        for(int j=0;j<MAX_API;j++)
            workflow_graph[i][j] = 0;
}

void verifier_add_edge(uint32_t from, uint32_t to)
{
    if(from < MAX_API && to < MAX_API)
        workflow_graph[from][to] = 1;
}

int verifier_check_trace(execution_trace *trace)
{
    for(uint32_t i=0;i<trace->length-1;i++)
    {
        uint32_t a = trace->api_sequence[i];
        uint32_t b = trace->api_sequence[i+1];

        if(workflow_graph[a][b] == 0)
            return 0;
    }

    return 1;
}