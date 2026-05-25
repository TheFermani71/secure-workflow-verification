import java.util.HashSet;
import java.util.Set;

public class Verifier {

    /*
     * Behavioral workflow validator
     */
    private WorkflowPathValidator workflowValidator;

    /*
     * Timing validator
     */
    private ApiTimingValidator apiTimingValidator;

    /*
     * Gap timing validator
     */
    private GapTimingValidator gapTimingValidator;

    public Verifier() {

        this.workflowValidator =
                new WorkflowPathValidator();

        this.apiTimingValidator =
                new ApiTimingValidator();

        this.gapTimingValidator =
                new GapTimingValidator();
    }

    // =========================================
    // DATA FLOW VALIDATION
    // =========================================

    public boolean validateDataFlow(
            ExecutionTrace trace
    ) {

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

        Set<String> produced =
                new HashSet<>();

        for (TraceEntry e : trace.entries) {

            /*
             * Validate input 1
             */
            if (!e.in1Id.equals("00000000")
                    && !produced.contains(
                    e.in1Id
            )) {

                System.out.println(

                        "[DATA FLOW] Invalid input: "
                                + e.in1Id
                );

                return false;
            }

            /*
             * Validate input 2
             */
            if (!e.in2Id.equals("00000000")
                    && !produced.contains(
                    e.in2Id
            )) {

                System.out.println(

                        "[DATA FLOW] Invalid input: "
                                + e.in2Id
                );

                return false;
            }

            /*
             * Register produced value
             */
            produced.add(e.outId);
        }

        System.out.println();
        System.out.println(
                "[DATA FLOW] VALID"
        );

        return true;
    }

    // =========================================
    // COMPLETE TRACE VALIDATION
    // =========================================

    public boolean validateTrace(

            ExecutionTrace trace,

            WorkflowGraph graph
    ) {

        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " TRACE VALIDATION START"
        );

        System.out.println(
                "========================================"
        );

        // =====================================
        // WORKFLOW VALIDATION
        // =====================================

        boolean workflowOk =
                workflowValidator.validate(
                        trace,
                        graph
                );

        if (!workflowOk) {

            System.out.println();

            System.out.println(
                    "[TRACE] INVALID WORKFLOW"
            );

            return false;
        }

        // =====================================
        // DATA FLOW VALIDATION
        // =====================================

        boolean dataFlowOk =
                validateDataFlow(trace);

        if (!dataFlowOk) {

            System.out.println();

            System.out.println(
                    "[TRACE] INVALID DATA FLOW"
            );

            return false;
        }

        // =====================================
        // API TIMING VALIDATION
        // =====================================

        boolean timingOk =
                apiTimingValidator.validate(
                        trace
                );

        if (!timingOk) {

            System.out.println();

            System.out.println(
                    "[TRACE] INVALID API TIMING"
            );

            return false;
        }

        // =====================================
        // GAP ANALYSIS
        // =====================================

        gapTimingValidator.analyze(trace);

        // =====================================
        // FINAL RESULT
        // =====================================

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

        System.out.println();

        System.out.println(
                "[TRACE] VALID"
        );

        return true;
    }
}