import java.util.HashSet;
import java.util.Set;

public class WorkflowPathValidator {

    /*
     * Validation mode
     */
    private ValidationMode mode;

    /*
     * Maximum async depth
     */
    private static final int MAX_RELAXED_DEPTH = 4;

    /*
     * Constructor
     */
    public WorkflowPathValidator(
            ValidationMode mode
    ) {

        this.mode = mode;
    }

    /*
     * Validate workflow path
     */
    public boolean validate(
            ExecutionTrace trace,
            WorkflowGraph graph
    ) {

        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " BEHAVIORAL WORKFLOW VALIDATION"
        );

        System.out.println(
                "========================================"
        );

        if (trace.entries.size() < 2) {

            System.out.println(
                    "[WORKFLOW] Empty trace"
            );

            return false;
        }

        for (int i = 1; i < trace.entries.size(); i++) {

            TraceEntry previous =
                    trace.entries.get(i - 1);

            TraceEntry current =
                    trace.entries.get(i);

            String from =
                    previous.opName;

            String to =
                    current.opName;

            /*
             * STRICT EDGE
             */
            if (graph.hasEdge(from, to)) {

                System.out.println(
                        "[OK] "
                                + from
                                + " -> "
                                + to
                );

                continue;
            }

            /*
             * RELAXED / COROUTINE
             */
            if (
                    mode == ValidationMode.RELAXED_ASYNC
                    ||
                    mode == ValidationMode.COROUTINE_AWARE
            ) {

                boolean reachable =
                        isReachable(
                                graph,
                                from,
                                to,
                                MAX_RELAXED_DEPTH
                        );

                if (reachable) {

                    System.out.println(
                            "[RELAXED OK] "
                                    + from
                                    + " -> "
                                    + to
                    );

                    continue;
                }
            }

            /*
             * INVALID
             */
            System.out.println();

            System.out.println(
                    "[WORKFLOW] INVALID TRANSITION"
            );

            System.out.println(
                    " from : "
                            + from
            );

            System.out.println(
                    " to   : "
                            + to
            );

            return false;
        }

        System.out.println();

        switch (mode) {

            case STRICT:

                System.out.println(
                        "[WORKFLOW] VALID (STRICT MODE)"
                );

                break;

            case RELAXED_ASYNC:

                System.out.println(
                        "[WORKFLOW] VALID (RELAXED ASYNC MODE)"
                );

                break;

            case COROUTINE_AWARE:

                System.out.println(
                        "[WORKFLOW] VALID (COROUTINE AWARE MODE)"
                );

                break;
        }

        return true;
    }

    /*
     * Graph reachability
     */
    private boolean isReachable(
            WorkflowGraph graph,
            String current,
            String target,
            int depth
    ) {

        Set<String> visited =
                new HashSet<>();

        return dfs(
                graph,
                current,
                target,
                depth,
                visited
        );
    }

    /*
     * DFS
     */
    private boolean dfs(
            WorkflowGraph graph,
            String current,
            String target,
            int depth,
            Set<String> visited
    ) {

        if (depth <= 0) {

            return false;
        }

        if (current.equals(target)) {

            return true;
        }

        visited.add(current);

        if (!graph.adjacency.containsKey(current)) {

            return false;
        }

        for (String next :
                graph.adjacency.get(current)) {

            if (visited.contains(next)) {

                continue;
            }

            boolean reachable =
                    dfs(
                            graph,
                            next,
                            target,
                            depth - 1,
                            visited
                    );

            if (reachable) {

                return true;
            }
        }

        return false;
    }
}