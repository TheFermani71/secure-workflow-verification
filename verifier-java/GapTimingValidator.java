public class GapTimingValidator {

    /*
     * Gap threshold
     *
     * Per ora:
     * solo logging/analysis.
     *
     * In futuro:
     * complexity-aware validation.
     */
    private static final long GAP_WARNING_US = 5000;

    public GapTimingValidator() {

    }

    /*
     * Analyze application-layer gaps
     */
    public void analyze(
            ExecutionTrace trace
    ) {

        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " GAP TIMING ANALYSIS (SV-API v6)"
        );

        System.out.println(
                "========================================"
        );

        /*
         * Start from second entry
         */
        for (int i = 1;
             i < trace.entries.size();
             i++) {

            TraceEntry previous =
                    trace.entries.get(i - 1);

            TraceEntry current =
                    trace.entries.get(i);

            /*
             * delta_gap_us =
             * total_delta - api_internal_time
             */
            long gapUs =
                    current.deltaUs
                            - current.deltaApiUs;

            System.out.println();

            System.out.println(
                    "Transition: "
                            + previous.opName
                            + " -> "
                            + current.opName
            );

            System.out.println(
                    "Gap time : "
                            + gapUs
                            + " us"
            );

            /*
             * Temporary heuristic analysis
             */
            if (gapUs > GAP_WARNING_US) {

                System.out.println(

                        "[WARNING] Large asynchronous gap detected"
                );

            } else {

                System.out.println(

                        "[OK] Gap within nominal range"
                );
            }
        }

        System.out.println();

        System.out.println(
                "[GAP TIMING] ANALYSIS COMPLETED"
        );
    }
}