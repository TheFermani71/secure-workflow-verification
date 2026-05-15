import java.util.HashSet;
import java.util.Set;

public class Verifier {

    private WorkflowValidator workflowValidator;

    private ApiTimingValidator apiTimingValidator;

    private GapTimingValidator gapTimingValidator;

    public Verifier() {

        this.workflowValidator =
                new WorkflowValidator();

        this.apiTimingValidator =
                new ApiTimingValidator();

        this.gapTimingValidator =
                new GapTimingValidator();
    }

    /*
     * Workflow validation
     */
    public boolean verifyWorkflow(
            ExecutionTrace trace,
            WorkflowGraph graph
    ) {

        return workflowValidator.validate(
                trace,
                graph
        );
    }

    /*
     * Data-flow validation
     */
    public boolean verifyDataFlow(
            ExecutionTrace trace
    ) {

        Set<String> produced =
                new HashSet<>();

        for (TraceEntry e : trace.entries) {

            if (!e.in1Id.equals("00000000")
                    && !produced.contains(e.in1Id)) {

                return false;
            }

            if (!e.in2Id.equals("00000000")
                    && !produced.contains(e.in2Id)) {

                return false;
            }

            produced.add(e.outId);
        }

        return true;
    }

    /*
     * Trusted kernel timing
     */
    public boolean verifyApiTiming(
            ExecutionTrace trace
    ) {

        return apiTimingValidator.validate(
                trace
        );
    }

    /*
     * Application gap timing
     */
    public boolean verifyGapTiming(
            ExecutionTrace trace
    ) {

        return gapTimingValidator.validate(
                trace
        );
    }
}