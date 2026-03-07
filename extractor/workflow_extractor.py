import json
import re
from api_ids import API_IDS

"""
Workflow Extractor

This module extracts the workflow graph from a C program.

Steps performed:

1. Parse the source code
2. Detect API calls
3. Build the API execution sequence
4. Generate workflow edges
5. Export the graph to JSON

The generated JSON file will be used by the Java verifier.
"""

def extract_api_calls(source_code):

    api_sequence = []

    for api_name in API_IDS.keys():

        pattern = rf"{api_name}\s*\("

        matches = re.finditer(pattern, source_code)

        for match in matches:

            api_sequence.append((match.start(), api_name))

    api_sequence.sort(key=lambda x: x[0])

    return [API_IDS[name] for _, name in api_sequence]


def build_workflow_graph(api_sequence):

    edges = set()

    for i in range(len(api_sequence)-1):

        a = api_sequence[i]
        b = api_sequence[i+1]

        edges.add((a,b))

    return list(edges)


def export_graph(edges):

    graph = {
        "edges": edges
    }

    with open("workflow_graph.json","w") as f:

        json.dump(graph,f,indent=4)


def main():

    source_file = "program.c"

    with open(source_file,"r") as f:

        source_code = f.read()

    api_sequence = extract_api_calls(source_code)

    edges = build_workflow_graph(api_sequence)

    export_graph(edges)

    print("Workflow graph exported to workflow_graph.json")


if __name__ == "__main__":

    main()