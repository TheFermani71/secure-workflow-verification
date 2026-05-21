from cfg_nodes import WorkflowNode
from cfg_nodes import WorkflowEdge

from complexity_model import build_complexity


class WorkflowGraphBuilder:

    def __init__(self):

        self.nodes = []

        self.edges = []

        self.node_counter = 0

        self.prev_node = None

    # ========================================
    # ADD NODE
    # ========================================

    def add_node(
            self,
            node_type,
            name
    ):

        node = WorkflowNode(

            self.node_counter,

            node_type,

            name
        )

        self.nodes.append(node)

        self.node_counter += 1

        return node.id

    # ========================================
    # ADD EDGE
    # ========================================

    def add_edge(
            self,
            from_id,
            to_id,
            complexity=None
    ):

        edge = WorkflowEdge(

            from_id,

            to_id,

            complexity
        )

        self.edges.append(edge)

    # ========================================
    # CONNECT NODE
    # ========================================

    def connect_node(
            self,
            node_id,
            complexity=None
    ):

        if self.prev_node is not None:

            self.add_edge(

                self.prev_node,

                node_id,

                complexity
            )

        self.prev_node = node_id

    # ========================================
    # EXPORT JSON
    # ========================================

    def export(self):

        return {

            "nodes": [

                n.to_dict()

                for n in self.nodes
            ],

            "edges": [

                e.to_dict()

                for e in self.edges
            ]
        }