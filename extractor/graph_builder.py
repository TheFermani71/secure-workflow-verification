class GraphBuilder:

    def __init__(self):

        pass

    # ==========================================
    # BUILD GRAPH
    # ==========================================

    def build(
            self,
            sequence,
            mode="sequential"
    ):

        nodes = []

        edges = []

        edge_set = set()

        # --------------------------------------
        # CREATE NODES
        # --------------------------------------

        for i, api in enumerate(sequence):

            node = {

                "id": i,
                "label": api

            }

            nodes.append(node)

        # --------------------------------------
        # CREATE DIRECT EDGES
        # --------------------------------------

        for i in range(len(sequence) - 1):

            src = sequence[i]

            dst = sequence[i + 1]

            edge = (src, dst)

            if edge not in edge_set:

                edge_set.add(edge)

                edges.append([src, dst])

        # --------------------------------------
        # SEQUENTIAL SHORTCUT EDGES
        # --------------------------------------

        if mode == "sequential":

            for i in range(len(sequence) - 2):

                src = sequence[i]

                dst = sequence[i + 2]

                edge = (src, dst)

                if edge not in edge_set:

                    edge_set.add(edge)

                    edges.append([src, dst])

        # --------------------------------------
        # FINAL GRAPH
        # --------------------------------------

        graph = {

            "nodes": nodes,

            "edges": edges

        }

        return graph