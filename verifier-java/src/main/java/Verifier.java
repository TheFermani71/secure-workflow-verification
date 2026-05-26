public class Verifier {

    /*
     * Workflow graph
     */
    private WorkflowGraph graph;

    /*
     * Validators
     */
    private WorkflowPathValidator pathValidator;

    private ApiTimingValidator apiTimingValidator;

    private GapTimingValidator gapTimingValidator;

    /*
     * Blockchain connector
     */
    private BlockchainConnector blockchainConnector;

    /*
     * Validation mode
     */
    private boolean relaxedMode;

    /*
     * Constructor
     */
    public Verifier(
            WorkflowGraph graph,
            boolean relaxedMode
    ) {

        this.graph =
                graph;

        this.relaxedMode =
                relaxedMode;

        /*
         * Validators
         */
        this.pathValidator =
                new WorkflowPathValidator(
                        relaxedMode
                );

        this.apiTimingValidator =
                new ApiTimingValidator();

        this.gapTimingValidator =
                new GapTimingValidator();

        /*
         * Blockchain connector
         */
        this.blockchainConnector =
                new BlockchainConnector(

                        "http://127.0.0.1:8545",

                        /*
                         * Hardhat test private key
                         */
                        "0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80",

                        /*
                         * Contract address
                         */
                        "0x5FbDB2315678afecb367f032d93F642f64180aa3"
                );
    }

    /*
     * Main validation pipeline
     */
    public VerificationResult validateTrace(
            ExecutionTrace trace,
            String workflowMode,
            String workflowFile,
            String traceFile
    ) {

        boolean workflowValid;
        boolean apiTimingValid;
        boolean finalValid;

        /*
         * ========================================
         * WORKFLOW VALIDATION
         * ========================================
         */

        workflowValid =
                pathValidator.validate(
                        trace,
                        graph
                );

        /*
         * ========================================
         * API TIMING VALIDATION
         * ========================================
         */

        apiTimingValid =
                apiTimingValidator.validate(
                        trace
                );

        /*
         * ========================================
         * GAP TIMING ANALYSIS
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

        finalValid =
                workflowValid
                        &&
                        apiTimingValid;

        /*
         * ========================================
         * HASH GENERATION
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
         * ========================================
         * RESULT OBJECT
         * ========================================
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
         * ========================================
         * PRINT RESULT
         * ========================================
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