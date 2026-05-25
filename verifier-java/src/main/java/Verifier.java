public class Verifier {

    private WorkflowPathValidator workflowValidator;

    private ApiTimingValidator apiTimingValidator;

    private GapTimingValidator gapTimingValidator;

    private WorkflowGraph graph;

    public Verifier(
            WorkflowGraph graph
    ) {

        this.graph = graph;

        workflowValidator =
                new WorkflowPathValidator();

        apiTimingValidator =
                new ApiTimingValidator();

        gapTimingValidator =
                new GapTimingValidator();
    }

    public VerificationResult validateTrace(

            ExecutionTrace trace,

            String workflowFile,

            String traceFile,

            String workflowMode

    ) {

        /*
         * ========================================
         * WORKFLOW VALIDATION
         * ========================================
         */

        boolean workflowValid =
                workflowValidator.validate(
                        trace,
                        graph
                );

        /*
         * ========================================
         * API TIMING VALIDATION
         * ========================================
         */

        boolean timingValid =
                apiTimingValidator.validate(
                        trace
                );

        /*
         * ========================================
         * GAP ANALYSIS
         * ========================================
         */

        gapTimingValidator.analyze(
                trace
        );

        /*
         * ========================================
         * FINAL RESULT
         * ========================================
         */

        boolean finalValid =
                workflowValid &&
                timingValid;

        /*
         * ========================================
         * SHA256 HASHES
         * ========================================
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
         * ========================================
         * BLOCKCHAIN ATTESTATION
         * ========================================
         */

        System.out.println();
        System.out.println("========================================");
        System.out.println(" BLOCKCHAIN ATTESTATION");
        System.out.println("========================================");
        System.out.println();

        BlockchainConnector connector =
                new BlockchainConnector(

                        "http://127.0.0.1:8545",

                        /*
                         * Hardhat Account #0 private key
                         */
                        "0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80",

                        /*
                         * Smart contract address
                         */
                        "0x5FbDB2315678afecb367f032d93F642f64180aa3"
                );

        connector.registerVerification(

                workflowHash,

                traceHash,

                finalValid
        );

        /*
         * ========================================
         * RESULT OBJECT
         * ========================================
         */

        VerificationResult result =
                new VerificationResult(

                        trace.deviceId,

                        workflowMode,

                        finalValid,

                        traceHash,

                        workflowHash,

                        System.currentTimeMillis()
                );

        /*
         * ========================================
         * PRINT RESULT
         * ========================================
         */

        System.out.println();
        System.out.println("========================================");
        System.out.println(" VERIFICATION RESULT");
        System.out.println("========================================");
        System.out.println();

        System.out.println(result);

        return result;
    }
}