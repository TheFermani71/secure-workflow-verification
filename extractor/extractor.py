import json
import sys

from workflow_extractor import WorkflowExtractor
from graph_builder import GraphBuilder

# ==========================================
# CLI MODE
# ==========================================

if len(sys.argv) < 2:

    print("\nUsage:")
    print(" python3 extractor.py sequential")
    print(" python3 extractor.py coroutine\n")

    sys.exit(1)

MODE = sys.argv[1]

if MODE not in ["sequential", "coroutine"]:

    print("\n[ERROR] Invalid mode")
    print("Use: sequential | coroutine\n")

    sys.exit(1)

# ==========================================
# CONFIG
# ==========================================

SOURCE_FILE = "sketch.ino"

# ==========================================
# EXTRACTOR
# ==========================================

print("========================================")
print(" SV-API Extractor v2 ")
print("========================================")

print(f"[MODE] {MODE}")

extractor = WorkflowExtractor(
    SOURCE_FILE,
    MODE
)

sequence = extractor.extract_sequence()

print("\nExtracted sequence:\n")
print(" -> ".join(sequence))

# ==========================================
# GRAPH BUILD
# ==========================================

builder = GraphBuilder()

graph = builder.build(sequence)

# ==========================================
# OUTPUT FILE
# ==========================================

if MODE == "sequential":

    output_file = "workflow_graph_sequential.json"

else:

    output_file = "workflow_graph_coroutine.json"

# ==========================================
# SAVE GRAPH
# ==========================================

with open(output_file, "w") as f:

    json.dump(
        graph,
        f,
        indent=4
    )

print("\n========================================")
print(" Graph generated ")
print("========================================")

print(f"[OUTPUT] {output_file}")

print(f"[EDGES] {len(graph['edges'])}")