import java.util.Set;

public class WorkflowNavigator {

    private WorkflowGraph graph;

    public WorkflowNavigator(
            WorkflowGraph graph
    ) {

        this.graph = graph;
    }

    /*
     * Check transition
     */
    public boolean isValidTransition(
            String from,
            String to
    ) {

        return graph.hasEdge(
                from,
                to
        );
    }

    /*
     * Print graph
     */
    public void printGraph() {

        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " WORKFLOW GRAPH "
        );

        System.out.println(
                "========================================"
        );

        for (String node : graph.nodes) {

            System.out.println(
                    "\n[" + node + "]"
            );

            Set<String> next =
                    graph.adjacency.get(node);

            for (String target : next) {

                System.out.println(
                        "  -> " + target
                );
            }
        }
    }
}