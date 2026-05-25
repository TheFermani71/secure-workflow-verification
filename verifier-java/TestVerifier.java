public class TestVerifier {

    /*
     * SELECT GRAPH MODE
     */
    private static final boolean COROUTINE_MODE = false;

    public static void main(String[] args) {

        System.out.println(
                "========================================"
        );

        System.out.println(
                " SV-API v6 VERIFIER "
        );

        System.out.println(
                "========================================"
        );

        /*
         * GRAPH FILE
         */
        String graphFile;

        if (COROUTINE_MODE) {

            graphFile =
                    "../extractor/workflow_graph_coroutine.json";

        } else {

            graphFile =
                    "../extractor/workflow_graph_sequential.json";
        }

        /*
         * TRACE FILE
         */
        String traceFile =
                "../extractor/trace.json";

        /*
         * LOAD GRAPH
         */
        WorkflowGraph graph =
                WorkflowLoader.loadGraph(
                        graphFile
                );

        /*
         * LOAD TRACE
         */
        ExecutionTrace trace =
                TraceParser.parseTrace(
                        traceFile
                );

        /*
         * VERIFIER
         */
        Verifier verifier =
                new Verifier();

        /*
         * VALIDATION
         */
        boolean valid =
                verifier.validateTrace(
                        trace,
                        graph
                );

        /*
         * FINAL RESULT
         */
        System.out.println(
                "\n========================================"
        );

        System.out.println(
                " FINAL RESULT "
        );

        System.out.println(
                "========================================"
        );

        if (valid) {

            System.out.println(
                    "[TRACE] VALID"
            );

        } else {

            System.out.println(
                    "[TRACE] INVALID"
            );
        }
    }
}