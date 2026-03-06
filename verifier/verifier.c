#include "verifier.h"

/*
=====================================================================
Workflow Graph Representation
=====================================================================

The workflow graph is internally represented using an adjacency
matrix.

workflow_graph[i][j] = 1

means that the transition:

    API_i → API_j

is valid.

If the value is 0, the transition is not allowed.
*/

static int workflow_graph[MAX_API][MAX_API];


/*
=====================================================================
verifier_init()

Initializes the workflow graph.

All transitions are set to invalid (0).
This corresponds to an empty graph.
*/

void verifier_init()
{
    for(int i = 0; i < MAX_API; i++)
    {
        for(int j = 0; j < MAX_API; j++)
        {
            workflow_graph[i][j] = 0;
        }
    }
}


/*
=====================================================================
verifier_add_edge()

Adds a valid transition to the workflow graph.

Parameters:

    from → source API
    to   → destination API

Example:

    verifier_add_edge(API_INIT, API_ASSIGN)

means that ASSIGN can follow INIT in the workflow.
*/

void verifier_add_edge(uint32_t from, uint32_t to)
{
    if(from < MAX_API && to < MAX_API)
    {
        workflow_graph[from][to] = 1;
    }
}


/*
=====================================================================
verifier_check_trace()

Checks whether the execution trace is valid with respect
to the workflow graph.

Algorithm:

For every pair of consecutive API calls:

    api_sequence[i]
    api_sequence[i+1]

check whether the transition exists in the graph.

If any transition is not allowed → return 0
If all transitions are valid     → return 1

Time complexity:

    O(n)

where n is the trace length.
*/

int verifier_check_trace(execution_trace *trace)
{
    for(uint32_t i = 0; i < trace->length - 1; i++)
    {
        uint32_t current_api = trace->api_sequence[i];
        uint32_t next_api    = trace->api_sequence[i+1];

        /*
        If the transition is not defined in the graph,
        the trace is considered invalid.
        */

        if(workflow_graph[current_api][next_api] == 0)
        {
            return 0;
        }
    }

    /*
    All transitions are valid.
    */

    return 1;
}