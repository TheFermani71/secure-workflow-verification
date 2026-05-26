import java.io.File;

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

        /*
         * Default configuration
         */
        String workflowMode =
                "sequential";

        boolean relaxedMode =
                false;

        /*
         * Parse arguments
         */
        if (args.length >= 1) {

            workflowMode =
                    args[0];
        }

        if (args.length >= 2) {

            String validationMode =
                    args[1];

            if (
                    validationMode.equalsIgnoreCase(
                            "relaxed"
                    )
            ) {

                relaxedMode = true;
            }
        }

        /*
         * Base path
         */
        String basePath =
                System.getProperty(
                        "user.dir"
                );

        /*
         * Extractor path
         */
        String extractorPath =
                basePath
                        + "/../extractor/";

        /*
         * File selection
         */
        String workflowFile;
        String traceFile;

        if (
                workflowMode.equals(
                        "coroutine"
                )
        ) {

            workflowFile =
                    extractorPath
                            + "workflow_graph_coroutine.json";

            traceFile =
                    extractorPath
                            + "trace.json";
        } else {

            workflowFile =
                    extractorPath
                            + "workflow_graph_sequential.json";

            traceFile =
                    extractorPath
                            + "trace.json";
        }

        /*
         * Debug paths
         */
        System.out.println();

        System.out.println(
                "[DEBUG] workflow file:"
        );

        System.out.println(
                workflowFile
        );

        System.out.println();

        System.out.println(
                "[DEBUG] trace file:"
        );

        System.out.println(
                traceFile
        );

        /*
         * File existence check
         */
        File wf =
                new File(workflowFile);

        File tf =
                new File(traceFile);

        if (!wf.exists()) {

            System.out.println();

            System.out.println(
                    "[ERROR] Workflow file not found"
            );

            return;
        }

        if (!tf.exists()) {

            System.out.println();

            System.out.println(
                    "[ERROR] Trace file not found"
            );

            return;
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
         * Create verifier
         */
        Verifier verifier =
                new Verifier(
                        graph,
                        relaxedMode
                );

        /*
         * Validate
         */
        VerificationResult result =
                verifier.validateTrace(
                        trace,
                        workflowMode,
                        workflowFile,
                        traceFile
                );

        /*
         * Final result
         */
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