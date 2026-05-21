import re
import json
import argparse

from graph_builder import WorkflowGraphBuilder

from coroutine_parser import (
    is_coroutine_macro,
    extract_coroutine_macro
)

from complexity_model import build_complexity

# ============================================
# ARGUMENTS
# ============================================

parser = argparse.ArgumentParser()

parser.add_argument(

    "--mode",

    choices=[

        "sequential",
        "coroutine"
    ],

    default="sequential"
)

args = parser.parse_args()

MODE = args.mode

# ============================================
# INPUT / OUTPUT
# ============================================

INPUT_FILE = "sketch.ino"

OUTPUT_FILE = "workflow_graph.json"

# ============================================
# API REGEX
# ============================================

api_pattern = re.compile(
    r'sv_kernel\.f_([a-zA-Z0-9_]+)'
)

# ============================================
# BRANCH KEYWORDS
# ============================================

branch_keywords = [

    "if",
    "else",
    "switch"
]

# ============================================
# LOOP KEYWORDS
# ============================================

loop_keywords = [

    "for",
    "while"
]

# ============================================
# NORMALIZATION
# ============================================

def normalize_api(name):

    name = name.upper()

    if "ADD" in name:
        return "ADD"

    if "GT" in name:
        return "GT"

    if "CONST" in name:
        return "CONST"

    if "AVG" in name:
        return "AVG_N"

    if "WRITE" in name:
        return "WRITE"

    if "ASSIGN" in name:
        return "ASSIGN"

    if "INIT" in name:
        return "INIT"

    return name


# ============================================
# GRAPH
# ============================================

graph = WorkflowGraphBuilder()

# ============================================
# PARSE SOURCE
# ============================================

with open(INPUT_FILE, "r") as f:

    lines = f.readlines()

for line in lines:

    line = line.strip()

    # ========================================
    # API EXTRACTION
    # ========================================

    api_match = api_pattern.search(line)

    if api_match:

        api_name = normalize_api(

            api_match.group(1)
        )

        node_id = graph.add_node(

            "API",

            api_name
        )

        graph.connect_node(node_id)

        print(f"[API] {api_name}")

    # ========================================
    # BRANCH EXTRACTION
    # ========================================

    for branch in branch_keywords:

        pattern = rf'\b{branch}\b'

        if re.search(pattern, line):

            node_id = graph.add_node(

                "BRANCH",

                branch.upper()
            )

            graph.connect_node(

                node_id,

                build_complexity(
                    branches=1
                )
            )

            print(f"[BRANCH] {branch}")

    # ========================================
    # LOOP EXTRACTION
    # ========================================

    for loop in loop_keywords:

        pattern = rf'\b{loop}\b'

        if re.search(pattern, line):

            node_id = graph.add_node(

                "LOOP",

                loop.upper()
            )

            graph.connect_node(

                node_id,

                build_complexity(
                    loops=1
                )
            )

            print(f"[LOOP] {loop}")

    # ========================================
    # COROUTINE MODE
    # ========================================

    if MODE == "coroutine":

        if is_coroutine_macro(line):

            macro = extract_coroutine_macro(
                line
            )

            node_id = graph.add_node(

                "COROUTINE",

                macro
            )

            graph.connect_node(

                node_id,

                build_complexity(
                    coroutines=1
                )
            )

            print(f"[COROUTINE] {macro}")

# ============================================
# EXPORT
# ============================================

workflow = graph.export()

with open(OUTPUT_FILE, "w") as f:

    json.dump(
        workflow,
        f,
        indent=4
    )

print()
print("===================================")
print(" Workflow Graph Generated")
print("===================================")

print(f"Mode  : {MODE}")

print(f"Nodes : {len(graph.nodes)}")

print(f"Edges : {len(graph.edges)}")

print()
print(f"Saved to: {OUTPUT_FILE}")