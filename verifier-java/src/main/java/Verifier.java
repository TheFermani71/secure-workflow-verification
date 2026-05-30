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

        workflowValid =
                pathValidator.validate(
                        trace,
                        graph
                );

        apiTimingValid =
                apiTimingValidator.validate(
                        trace
                );

        gapTimingValidator.analyze(
                trace
        );

        integrityValid =
                integrityValidator.validate(
                        trace
                );

        finalValid =
                workflowValid
                        &&
                        apiTimingValid
                        &&
                        integrityValid;

        String workflowHash =
                HashUtils.sha256File(
                        workflowFile
                );

        String traceHash =
                HashUtils.sha256File(
                        traceFile
                );

        /*
         * Verifier mode
         */
        String verifierMode;

        if (relaxedMode) {

            verifierMode =
                    "RELAXED";

        } else {

            verifierMode =
                    "STRICT";
        }

        /*
         * Trace metadata
         */
        long traceLength =
                trace.entries.size();

        String traceMerkleRoot =
                trace.traceMerkleRoot;

        /*
         * Build result object
         */
        VerificationResult result =
                new VerificationResult(

                        trace.deviceId,

                        workflowMode,

                        verifierMode,

                        finalValid,

                        traceHash,

                        workflowHash,

                        traceMerkleRoot,

                        traceLength,

                        System.currentTimeMillis()
                );

        /*
         * Blockchain attestation
         */
        blockchainConnector.registerVerification(
                result
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