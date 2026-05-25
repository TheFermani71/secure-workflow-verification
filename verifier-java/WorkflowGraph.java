import java.util.*;

public class WorkflowGraph {

    /*
     * Nodes
     */
    public List<String> nodes;

    /*
     * Adjacency list
     */
    public Map<String, Set<String>> adjacency;

    public WorkflowGraph() {

        nodes =
                new ArrayList<>();

        adjacency =
                new HashMap<>();
    }

    /*
     * Add node
     */
    public void addNode(
            String node
    ) {

        if (!nodes.contains(node)) {

            nodes.add(node);
        }

        if (!adjacency.containsKey(node)) {

            adjacency.put(
                    node,
                    new HashSet<>()
            );
        }
    }

    /*
     * Add edge
     */
    public void addEdge(
            String from,
            String to
    ) {

        addNode(from);

        addNode(to);

        adjacency
                .get(from)
                .add(to);
    }

    /*
     * Check edge
     */
    public boolean hasEdge(
            String from,
            String to
    ) {

        if (!adjacency.containsKey(from)) {

            return false;
        }

        return adjacency
                .get(from)
                .contains(to);
    }

    /*
     * Debug
     */
    @Override
    public String toString() {

        return "[WorkflowGraph nodes="
                + nodes.size()
                + "]";
    }
}