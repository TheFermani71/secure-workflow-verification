import java.util.ArrayList;
import java.util.List;

public class WorkflowGraph {

    /*
     * Workflow node
     */
    public static class Node {

        public int id;

        public String type;

        public String name;

        public Node(
                int id,
                String type,
                String name
        ) {

            this.id = id;
            this.type = type;
            this.name = name;
        }
    }

    /*
     * Workflow edge
     */
    public static class Edge {

        public int from;

        public int to;

        public Edge(
                int from,
                int to
        ) {

            this.from = from;
            this.to = to;
        }
    }

    public List<Node> nodes;

    public List<Edge> edges;

    public WorkflowGraph() {

        nodes = new ArrayList<>();

        edges = new ArrayList<>();
    }

    public void addNode(
            int id,
            String type,
            String name
    ) {

        nodes.add(
                new Node(id, type, name)
        );
    }

    public void addEdge(
            int from,
            int to
    ) {

        edges.add(
                new Edge(from, to)
        );
    }
}