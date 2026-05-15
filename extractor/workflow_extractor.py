import re
import json

# ============================================
# SV-API v6 Workflow Extractor
#
# Extractor v2 Architecture Preparation
#
# Current phase:
# - API extraction
# - branch hooks
# - coroutine hooks
# - async-aware graph model
#
# NOT YET:
# - full CFG
# - AST parsing
# - loop analysis
# ============================================

INPUT_FILE = "sketch.ino"

OUTPUT_FILE = "workflow_graph.json"

# --------------------------------------------
# API regex
# --------------------------------------------

api_pattern = re.compile(
    r'sv_kernel\.f_([a-zA-Z0-9_]+)'
)

# --------------------------------------------
# Coroutine hooks
# --------------------------------------------

coroutine_hooks = [
    "COROUTINE_YIELD",
    "COROUTINE_DELAY",
    "COROUTINE_LOOP",
    "COROUTINE_BEGIN",
]

# --------------------------------------------
# Branch hooks
# --------------------------------------------

branch_keywords = [
    "if",
    "else",
    "switch",
]

# --------------------------------------------
# Loop hooks
# --------------------------------------------

loop_keywords = [
    "for",
    "while",
]

# --------------------------------------------
# API normalization
# --------------------------------------------

def normalize_api(name):

    name = name.upper()

    if "ADD" in name:
        return "ADD"

    if "SUB" in name:
        return "SUB"

    if "MUL" in name:
        return "MUL"

    if "DIV" in name:
        return "DIV"

    if "GT" in name:
        return "GT"

    if "LT" in name:
        return "LT"

    if "EQ" in name:
        return "EQ"

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


# --------------------------------------------
# Graph structures
# --------------------------------------------

nodes = []
edges = []

node_counter = 0
prev_node_id = None

# --------------------------------------------
# Add graph node
# --------------------------------------------

def add_node(node_type, name):

    global node_counter

    node = {
        "id": node_counter,
        "type": node_type,
        "name": name
    }

    nodes.append(node)

    node_counter += 1

    return node["id"]


# --------------------------------------------
# Add graph edge
# --------------------------------------------

def add_edge(from_id, to_id):

    edges.append({
        "from": from_id,
        "to": to_id
    })


# --------------------------------------------
# Parse firmware
# --------------------------------------------

with open(INPUT_FILE, "r") as f:

    lines = f.readlines()

for line in lines:

    line = line.strip()

    # ----------------------------------------
    # API extraction
    # ----------------------------------------

    api_match = api_pattern.search(line)

    if api_match:

        api_name = normalize_api(
            api_match.group(1)
        )

        node_id = add_node(
            "API",
            api_name
        )

        print(f"[API] {api_name}")

        if prev_node_id is not None:

            add_edge(prev_node_id, node_id)

        prev_node_id = node_id

    # ----------------------------------------
    # Coroutine hooks
    # ----------------------------------------

    for coro in coroutine_hooks:

        if coro in line:

            node_id = add_node(
                "COROUTINE",
                coro
            )

            print(f"[COROUTINE] {coro}")

            if prev_node_id is not None:

                add_edge(prev_node_id, node_id)

            prev_node_id = node_id

    # ----------------------------------------
    # Branch hooks
    # ----------------------------------------

    for branch in branch_keywords:

        pattern = rf'\\b{branch}\\b'

        if re.search(pattern, line):

            node_id = add_node(
                "BRANCH",
                branch.upper()
            )

            print(f"[BRANCH] {branch}")

            if prev_node_id is not None:

                add_edge(prev_node_id, node_id)

            prev_node_id = node_id

    # ----------------------------------------
    # Loop hooks
    # ----------------------------------------

    for loop in loop_keywords:

        pattern = rf'\\b{loop}\\b'

        if re.search(pattern, line):

            node_id = add_node(
                "LOOP",
                loop.upper()
            )

            print(f"[LOOP] {loop}")

            if prev_node_id is not None:

                add_edge(prev_node_id, node_id)

            prev_node_id = node_id


# --------------------------------------------
# Export graph
# --------------------------------------------

graph = {
    "nodes": nodes,
    "edges": edges
}

with open(OUTPUT_FILE, "w") as f:

    json.dump(graph, f, indent=4)

print()
print("===================================")
print(" Workflow Graph Generated (v2)")
print("===================================")

print(f"Nodes : {len(nodes)}")
print(f"Edges : {len(edges)}")

print()
print(f"Saved to: {OUTPUT_FILE}")