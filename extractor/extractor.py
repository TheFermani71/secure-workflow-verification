import json
import hashlib
import argparse

from workflow_extractor import WorkflowExtractor
from graph_builder import GraphBuilder
from merkle_tree import MerkleTree


# ==========================================
# SHA256
# ==========================================

def sha256(data):

    return hashlib.sha256(
        data.encode()
    ).hexdigest()


# ==========================================
# TRACE GENERATION
# ==========================================

def build_trace():

    trace_entries = []

    operations = [

        ("INIT", 131),
        ("INIT", 131),
        ("AVG_N", 325),
        ("CONST", 122),
        ("GT", 359),
        ("WRITE", 134)

    ]

    for i, (op_name, api_time) in enumerate(operations):

        # ----------------------------------
        # PREVIOUS HASH
        # ----------------------------------

        if len(trace_entries) == 0:

            prev_hash = "GENESIS"

        else:

            prev_hash = trace_entries[-1]["entry_hash"]

        # ----------------------------------
        # ENTRY
        # ----------------------------------

        entry = {

            "seq": i,

            "op": i,

            "op_name": op_name,

            "out_id": "out_" + str(i),

            "in1_id": "in1_" + str(i),

            "in2_id": "in2_" + str(i),

            "var_id": i,

            "var_name": "var_" + str(i),

            "delta_us": api_time + 100,

            "delta_api_us": api_time,

            "chain_tag": "CHAIN_A"
        }

        # ----------------------------------
        # PAYLOAD
        # ----------------------------------

        payload = (

            str(entry["seq"])
            + "|"

            + str(entry["op"])
            + "|"

            + entry["op_name"]
            + "|"

            + str(entry["delta_us"])
            + "|"

            + str(entry["delta_api_us"])
            + "|"

            + entry["chain_tag"]
        )

        # ----------------------------------
        # HASH CHAIN
        # ----------------------------------

        entry_hash = sha256(

            prev_hash + payload
        )

        entry["prev_hash"] = prev_hash

        entry["entry_hash"] = entry_hash

        trace_entries.append(entry)

    # --------------------------------------
    # MERKLE ROOT
    # --------------------------------------

    merkle_tree = MerkleTree()

    trace_merkle_root = (

        merkle_tree.build_root(
            trace_entries
        )
    )

    # --------------------------------------
    # FINAL TRACE
    # --------------------------------------

    trace = {

        "sv_api_version": 6,

        "device_id": "ATOM_GPS_V2",

        "cal_tick_us": 472,

        "trace_merkle_root": trace_merkle_root,

        "entries": trace_entries
    }

    return trace


# ==========================================
# SAVE TRACE
# ==========================================

def save_trace(trace):

    with open("trace.json", "w") as f:

        json.dump(
            trace,
            f,
            indent=4
        )

    print()

    print("[Extractor] Trace generated")

    print("Entries :", len(trace["entries"]))

    print(
        "Merkle Root :",
        trace["trace_merkle_root"]
    )

    print("Output  : trace.json")


# ==========================================
# BUILD WORKFLOW
# ==========================================

def build_workflow(mode):

    source_file = "sketch.ino"

    extractor = WorkflowExtractor(
        source_file,
        mode
    )

    sequence = extractor.extract_sequence()

    builder = GraphBuilder()

    graph = builder.build(sequence)

    if mode == "sequential":

        output_file = "workflow_graph_sequential.json"

    else:

        output_file = "workflow_graph_coroutine.json"

    with open(output_file, "w") as f:

        json.dump(
            graph,
            f,
            indent=4
        )

    print()

    print("[Workflow] Graph generated")

    print("Mode    :", mode)

    print("Nodes   :", len(graph["nodes"]))

    print("Edges   :", len(graph["edges"]))

    print("Output  :", output_file)


# ==========================================
# MAIN
# ==========================================

def main():

    parser = argparse.ArgumentParser()

    parser.add_argument(

        "--mode",

        choices=[

            "sequential",
            "coroutine",
            "both"
        ],

        default="both"
    )

    args = parser.parse_args()

    print()
    print("===================================")
    print(" SV-API EXTRACTION PIPELINE")
    print("===================================")

    # --------------------------------------
    # TRACE
    # --------------------------------------

    trace = build_trace()

    save_trace(trace)

    # --------------------------------------
    # WORKFLOW
    # --------------------------------------

    if args.mode == "sequential":

        build_workflow("sequential")

    elif args.mode == "coroutine":

        build_workflow("coroutine")

    else:

        build_workflow("sequential")

        build_workflow("coroutine")

    print()

    print("[DONE] Extraction pipeline completed")


if __name__ == "__main__":

    main()