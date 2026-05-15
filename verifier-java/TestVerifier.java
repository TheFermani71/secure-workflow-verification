public class TestVerifier {

    public static void main(String[] args) {

        System.out.println();
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
         * Load workflow graph
         */
        WorkflowGraph graph =
                WorkflowLoader.loadGraph(
                        "../extractor/workflow_graph.json"
                );

        /*
         * Load runtime trace
         */
        ExecutionTrace trace =
                TraceParser.parse(
                        "trace.json"
                );

        /*
         * Verifier
         */
        Verifier verifier =
                new Verifier();

        /*
         * Workflow validation
         */
        System.out.println();
        System.out.println(
                "========================================"
        );
        System.out.println(
                " WORKFLOW VALIDATION"
        );
        System.out.println(
                "========================================"
        );

        boolean workflowOk =
                verifier.verifyWorkflow(
                        trace,
                        graph
                );

        System.out.println(
                workflowOk
                        ? "[WORKFLOW] VALID"
                        : "[WORKFLOW] INVALID"
        );

        /*
         * Data flow validation
         */
        System.out.println();
        System.out.println(
                "========================================"
        );
        System.out.println(
                " DATA FLOW VALIDATION"
        );
        System.out.println(
                "========================================"
        );

        boolean dataOk =
                verifier.verifyDataFlow(
                        trace
                );

        System.out.println(
                dataOk
                        ? "[DATA FLOW] VALID"
                        : "[DATA FLOW] INVALID"
        );

        /*
         * API timing validation
         */
        boolean apiTimingOk =
                verifier.verifyApiTiming(
                        trace
                );

        /*
         * GAP timing analysis
         */
        boolean gapOk =
                verifier.verifyGapTiming(
                        trace
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

        if (workflowOk
                && dataOk
                && apiTimingOk
                && gapOk) {

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