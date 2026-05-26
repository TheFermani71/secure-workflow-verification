public class Verifier {

    private WorkflowGraph graph;

    private WorkflowPathValidator pathValidator;

    private ApiTimingValidator apiTimingValidator;

    private GapTimingValidator gapTimingValidator;

    private TraceIntegrityValidator integrityValidator;

    private BlockchainConnector blockchainConnector;

    private boolean relaxedMode;

    public Verifier(
            WorkflowGraph graph,
            boolean relaxedMode
    ) {

        this.graph =
                graph;

        this.relaxedMode =
                relaxedMode;

        this.pathValidator =
                new WorkflowPathValidator(
                        relaxedMode
                );

        this.apiTimingValidator =
                new ApiTimingValidator();

        this.gapTimingValidator =
                new GapTimingValidator();

        this.integrityValidator =
                new TraceIntegrityValidator();

        this.blockchainConnector =
                new BlockchainConnector(

                        "http://127.0.0.1:8545",

                        "0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80",

                        "0x5FbDB2315678afecb367f032d93F642f64180aa3"
                );
    }

    public VerificationResult validateTrace(
            ExecutionTrace trace,
            String workflowMode,
            String workflowFile,
            String traceFile
    ) {

        boolean workflowValid;

        boolean apiTimingValid;

        boolean integrityValid;

        boolean finalValid;

        /*
         * Workflow validation
         */
        workflowValid =
                pathValidator.validate(
                        trace,
                        graph
                );

        /*
         * Timing validation
         */
        apiTimingValid =
                apiTimingValidator.validate(
                        trace
                );

        /*
         * Gap analysis
         */
        gapTimingValidator.analyze(
                trace
        );

        /*
         * Integrity validation
         */
        integrityValid =
                integrityValidator.validate(
                        trace
                );

        /*
         * Final result
         */
        finalValid =
                workflowValid
                        &&
                        apiTimingValid
                        &&
                        integrityValid;

        /*
         * Hash generation
         */
        String workflowHash =
                HashUtils.sha256File(
                        workflowFile
                );

        String traceHash =
                HashUtils.sha256File(
                        traceFile
                );

        /*
         * Blockchain attestation
         */
        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " BLOCKCHAIN ATTESTATION"
        );

        System.out.println(
                "========================================"
        );

        blockchainConnector.registerVerification(
                workflowHash,
                traceHash,
                finalValid
        );

        /*
         * Result object
         */
        VerificationResult result =
                new VerificationResult(

                        null,

                        workflowMode,

                        finalValid,

                        traceHash,

                        workflowHash,

                        System.currentTimeMillis()
                );

        /*
         * Print result
         */
        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " VERIFICATION RESULT"
        );

        System.out.println(
                "========================================"
        );

        System.out.println();

        System.out.println(
                result
        );

        return result;
    }
}