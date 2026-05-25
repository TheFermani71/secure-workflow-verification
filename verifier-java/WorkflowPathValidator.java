import java.util.List;

public class WorkflowPathValidator {

    /*
     * Validate execution path
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
                " BEHAVIORAL WORKFLOW VALIDATION "
        );

        System.out.println(
                "========================================"
        );

        List<TraceEntry> entries =
                trace.getEntries();

        /*
         * Empty trace
         */
        if (entries.isEmpty()) {

            System.out.println(
                    "[WORKFLOW] Empty trace"
            );

            return false;
        }

        /*
         * Validate transitions
         */
        for (int i = 0; i < entries.size() - 1; i++) {

            TraceEntry current =
                    entries.get(i);

            TraceEntry next =
                    entries.get(i + 1);

            String from =
                    current.opName;

            String to =
                    next.opName;

            /*
             * Transition check
             */
            if (!graph.hasEdge(from, to)) {

                System.out.println();

                System.out.println(
                        "[WORKFLOW] INVALID TRANSITION"
                );

                System.out.println(
                        " from : " + from
                );

                System.out.println(
                        " to   : " + to
                );

                return false;
            }

            System.out.println(
                    "[OK] "
                            + from
                            + " -> "
                            + to
            );
        }

        System.out.println();

        System.out.println(
                "[WORKFLOW] VALID"
        );

        return true;
    }
}