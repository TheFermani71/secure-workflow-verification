class GraphBuilder:

    def __init__(self):

        pass

    # ==========================================
    # BUILD GRAPH
    # ==========================================

    def build(self, sequence):

        nodes = []
        edges = []

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
        # CREATE EDGES
        # --------------------------------------

        for i in range(len(sequence) - 1):

            edge = [

                sequence[i],
                sequence[i + 1]

            ]

            edges.append(edge)

        # --------------------------------------
        # FINAL GRAPH
        # --------------------------------------

        graph = {

            "nodes": nodes,
            "edges": edges

        }

        return graph