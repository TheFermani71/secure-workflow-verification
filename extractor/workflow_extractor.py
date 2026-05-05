import re
import json

INPUT_FILE = "sketch.ino"
OUTPUT_FILE = "workflow_graph.json"

# API che vogliamo davvero considerare (quelle del verifier)
TARGET_APIS = {
    "INIT",
    "ASSIGN",
    "ADD",
    "DIV",
    "AVG_N",
    "CONST",
    "GT",
    "WRITE",
    "READ"
}


# Estrae chiamate API reali dal codice firmware
def extract_api_calls(code):
    pattern = r'\bsv_kernel\.f_([a-zA-Z0-9_]+)\s*\('
    matches = re.finditer(pattern, code)

    calls = []

    for m in matches:
        raw = m.group(1).upper()

        # SOLO quelle che compaiono nella trace reale
        if "INIT" in raw:
            calls.append("INIT")

        elif "AVG" in raw:
            calls.append("AVG_N")

        elif "CONST" in raw:
            calls.append("CONST")

        elif raw == "GT":
            calls.append("GT")

        elif "WRITE" in raw:
            calls.append("WRITE")

        # IGNORA tutto il resto COMPLETAMENTE

    return calls


# Costruisce il workflow (transizioni)
def build_edges(sequence):
    edges = []

    for i in range(len(sequence) - 1):
        edge = [sequence[i], sequence[i + 1]]

        # evita duplicati consecutivi
        if len(edges) == 0 or edges[-1] != edge:
            edges.append(edge)

    return edges


def main():

    # Legge il codice
    with open(INPUT_FILE, "r") as f:
        code = f.read()

    # Estrae le API rilevanti
    sequence = extract_api_calls(code)

    print("\n=== DEBUG EXTRACTOR ===")
    print("Sequence length:", len(sequence))
    print("Extracted sequence:")

    if len(sequence) == 0:
        print("Nessuna API trovata!")
    else:
        print(" -> ".join(sequence))

    # Costruisce il grafo
    edges = build_edges(sequence)

    graph = {
        "edges": edges
    }

    # Salva JSON
    with open(OUTPUT_FILE, "w") as f:
        json.dump(graph, f, indent=2)

    print("\nWorkflow graph generated.")
    print("Edges count:", len(edges))


if __name__ == "__main__":
    main()