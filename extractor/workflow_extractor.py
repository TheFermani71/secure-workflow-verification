import json
import re
from api_ids import API_IDS

"""
Workflow Extractor with Temporal Model

This module extracts the workflow graph from a C program
and assigns a temporal weight to each transition.
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


def get_operation_weight(api_id):

    weights = {
        1: 1,  # INIT
        2: 1,  # ASSIGN
        3: 2,  # ADD
        4: 3   # DIV
    }

    return weights.get(api_id, 1)


def build_workflow_graph(api_sequence):

    edges = []

    for i in range(len(api_sequence)-1):

        a = api_sequence[i]
        b = api_sequence[i+1]

        weight = get_operation_weight(b)

        edges.append({
            "from": a,
            "to": b,
            "time": weight
        })

    return edges


def export_graph(edges):

    graph = {
        "edges": edges
    }

    with open("workflow_graph.json", "w") as f:
        json.dump(graph, f, indent=4)


def main():

    source_file = "program.c"

    with open(source_file, "r") as f:
        source_code = f.read()

    api_sequence = extract_api_calls(source_code)

    edges = build_workflow_graph(api_sequence)

    export_graph(edges)

    print("Temporal workflow graph exported")


if __name__ == "__main__":
    main()