public class WorkflowValidator {

    /*
     * Normalize API names
     */
    private String normalize(
            String name
    ) {

        name = name.toUpperCase();

        /*
         * INIT variants
         */
        if (name.contains("INIT")) {
            return "INIT";
        }

        /*
         * ASSIGN variants
         */
        if (name.contains("ASSIGN")) {
            return "ASSIGN";
        }

        /*
         * CONST variants
         */
        if (name.contains("CONST")) {
            return "CONST";
        }

        /*
         * AVG variants
         */
        if (name.contains("AVG")) {
            return "AVG_N";
        }

        /*
         * ADD variants
         */
        if (name.contains("ADD")) {
            return "ADD";
        }

        /*
         * GT variants
         */
        if (name.contains("GT")) {
            return "GT";
        }

        /*
         * WRITE variants
         */
        if (name.contains("WRITE")) {
            return "WRITE";
        }

        return name;
    }

    public boolean validate(
            ExecutionTrace trace,
            WorkflowGraph graph
    ) {

        System.out.println();

        for (TraceEntry entry : trace.entries) {

            String traceOp =
                    normalize(entry.opName);

            boolean found = false;

            for (WorkflowGraph.Node node : graph.nodes) {

                /*
                 * Validate only API nodes
                 */
                if (!node.type.equals("API")) {
                    continue;
                }

                String graphOp =
                        normalize(node.name);

                if (graphOp.equals(traceOp)) {

                    found = true;
                    break;
                }
            }

            if (!found) {

                System.out.println(
                        "[WORKFLOW] Missing node for API: "
                                + traceOp
                );

                return false;
            }
        }

        return true;
    }
}