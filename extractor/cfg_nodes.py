# ============================================
# CFG NODE DEFINITIONS
#
# SV-API v6
# Workflow Extractor v2
# ============================================

class WorkflowNode:

    def __init__(
            self,
            node_id,
            node_type,
            name
    ):

        self.id = node_id

        self.type = node_type

        self.name = name

    def to_dict(self):

        return {
            "id": self.id,
            "type": self.type,
            "name": self.name
        }


class WorkflowEdge:

    def __init__(
            self,
            from_id,
            to_id,
            complexity=None
    ):

        self.from_id = from_id

        self.to_id = to_id

        if complexity is None:

            complexity = {
                "branches": 0,
                "loops": 0,
                "coroutines": 0
            }

        self.complexity = complexity

    def to_dict(self):

        return {
            "from": self.from_id,
            "to": self.to_id,
            "complexity": self.complexity
        }