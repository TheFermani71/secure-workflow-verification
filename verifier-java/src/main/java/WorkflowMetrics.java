import java.util.HashSet;
import java.util.Set;

public class WorkflowMetrics {

    private WorkflowGraph graph;

    public WorkflowMetrics(
            WorkflowGraph graph
    ) {

        this.graph = graph;
    }

    public int getNodeCount() {

        return graph.nodes.size();
    }

    public int getEdgeCount() {

        int count = 0;

        for (String node : graph.nodes) {

            count +=
                    graph.adjacency
                            .get(node)
                            .size();
        }

        return count;
    }

    public int getCoroutineNodeCount() {

        int count = 0;

        for (String node : graph.nodes) {

            if (isCoroutineNode(node)) {

                count++;
            }
        }

        return count;
    }

    public int getBranchPointCount() {

        int count = 0;

        for (String node : graph.nodes) {

            if (
                    graph.adjacency.containsKey(node)
                            &&
                    graph.adjacency.get(node).size() > 1
            ) {

                count++;
            }
        }

        return count;
    }

    public int getGraphDepth() {

        int maxDepth = 0;

        for (String node : graph.nodes) {

            maxDepth =
                    Math.max(
                            maxDepth,
                            depth(node,
                                    new HashSet<>())
                    );
        }

        return maxDepth;
    }

    private int depth(
            String node,
            Set<String> visited
    ) {

        if (visited.contains(node)) {

            return 0;
        }

        visited.add(node);

        if (
                !graph.adjacency.containsKey(node)
                        ||
                graph.adjacency.get(node).isEmpty()
        ) {

            return 1;
        }

        int max = 0;

        for (
                String next :
                graph.adjacency.get(node)
        ) {

            max =
                    Math.max(
                            max,
                            depth(
                                    next,
                                    new HashSet<>(visited)
                            )
                    );
        }

        return max + 1;
    }

    private boolean isCoroutineNode(
            String node
    ) {

        return
                node.equals("COROUTINE_LOOP")
                        ||
                node.equals("COROUTINE_YIELD")
                        ||
                node.equals("COROUTINE_DELAY");
    }

    public void printAnalysis() {

        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " WORKFLOW ANALYSIS"
        );

        System.out.println(
                "========================================"
        );

        System.out.println(
                "Nodes           : "
                        + getNodeCount()
        );

        System.out.println(
                "Edges           : "
                        + getEdgeCount()
        );

        System.out.println(
                "Branch Points   : "
                        + getBranchPointCount()
        );

        System.out.println(
                "Coroutine Nodes : "
                        + getCoroutineNodeCount()
        );

        System.out.println(
                "Graph Depth     : "
                        + getGraphDepth()
        );
    }

    public void printCoroutineAnalysis() {

        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " COROUTINE ANALYSIS"
        );

        System.out.println(
                "========================================"
        );

        for (String node : graph.nodes) {

            if (isCoroutineNode(node)) {

                System.out.println(
                        " - " + node
                );
            }
        }
    }
}