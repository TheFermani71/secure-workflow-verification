public class TestVerifier {

    public static void main(
            String[] args
    ) {

        System.out.println(
                "========================================"
        );

        System.out.println(
                " SV-API v6 VERIFIER"
        );

        System.out.println(
                "========================================"
        );

        String mode = "coroutine";

        if (args.length > 0) {

            mode = args[0];
        }

        /*
         * Files
         */
        String workflowFile;

        String traceFile;

        if (mode.equals("sequential")) {

            workflowFile =
                    "../extractor/workflow_graph_sequential.json";

            traceFile =
                    "../extractor/trace.json";

        } else {

            workflowFile =
                    "../extractor/workflow_graph_coroutine.json";

            traceFile =
                    "../extractor/trace.json";
        }

        /*
         * Load workflow graph
         */
        WorkflowLoader loader =
                new WorkflowLoader();

        WorkflowGraph graph =
                loader.loadGraph(
                        workflowFile
                );

        /*
         * Parse trace
         */
        ExecutionTrace trace =
                TraceParser.parseTrace(
                        traceFile
                );

        /*
         * Verifier
         */
        Verifier verifier =
                new Verifier(
                        graph
                );

        VerificationResult result =
                verifier.validateTrace(
                        trace,
                        workflowFile,
                        traceFile,
                        mode
                );

        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " FINAL RESULT"
        );

        System.out.println(
                "========================================"
        );

        if (result.valid) {

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