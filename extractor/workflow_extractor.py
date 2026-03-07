import re
from api_ids import API_IDS


"""
====================================================================
Workflow Extractor
====================================================================

This module extracts the API workflow from a C source file.

The goal is to reconstruct the sequence of secure API calls
performed by the program and derive a workflow graph.

The resulting graph will later be used by the verifier
to validate execution traces.

The extractor performs the following steps:

1) Parse the C source code
2) Detect API calls
3) Build the ordered sequence of APIs
4) Generate the workflow graph
"""


def extract_api_calls(source_code):
    """
    Extracts all API calls from the C source code.

    The function searches for calls to:

        api_init(...)
        api_assign(...)
        api_add(...)
        api_div(...)

    Returns an ordered list of API identifiers.
    """

    api_sequence = []

    for api_name in API_IDS.keys():

        pattern = rf"{api_name}\s*\("

        matches = re.finditer(pattern, source_code)

        for match in matches:
            api_sequence.append((match.start(), api_name))

    # sort calls according to their position in the file
    api_sequence.sort(key=lambda x: x[0])

    return [API_IDS[name] for _, name in api_sequence]


def build_workflow_graph(api_sequence):
    """
    Builds the workflow graph from the API sequence.

    Each pair of consecutive API calls generates
    a valid transition in the graph.
    """

    edges = set()

    for i in range(len(api_sequence) - 1):

        a = api_sequence[i]
        b = api_sequence[i + 1]

        edges.add((a, b))

    return edges


def generate_graph_matrix(edges):
    """
    Converts the edge list into an adjacency matrix
    compatible with the verifier module.
    """

    MAX_API = 16

    matrix = [[0 for _ in range(MAX_API)] for _ in range(MAX_API)]

    for a, b in edges:
        matrix[a][b] = 1

    return matrix


def main():

    source_file = "program.c"

    with open(source_file, "r") as f:
        source_code = f.read()

    api_sequence = extract_api_calls(source_code)

    print("Detected API sequence:")
    print(api_sequence)

    edges = build_workflow_graph(api_sequence)

    print("\nWorkflow edges:")
    print(edges)

    matrix = generate_graph_matrix(edges)

    print("\nWorkflow matrix:")

    for row in matrix:
        print(row)


if __name__ == "__main__":
    main()