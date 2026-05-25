public class VerificationResult {

    /*
     * Device info
     */
    public String deviceId;

    /*
     * Workflow mode
     */
    public String workflowMode;

    /*
     * Verification status
     */
    public boolean valid;

    /*
     * Trace hash
     */
    public String traceHash;

    /*
     * Workflow hash
     */
    public String workflowHash;

    /*
     * Timestamp
     */
    public long timestamp;

    public VerificationResult(
            String deviceId,
            String workflowMode,
            boolean valid,
            String traceHash,
            String workflowHash,
            long timestamp
    ) {

        this.deviceId =
                deviceId;

        this.workflowMode =
                workflowMode;

        this.valid =
                valid;

        this.traceHash =
                traceHash;

        this.workflowHash =
                workflowHash;

        this.timestamp =
                timestamp;
    }

    @Override
    public String toString() {

        return "\nVerificationResult {\n"
                + " deviceId      = " + deviceId + "\n"
                + " workflowMode  = " + workflowMode + "\n"
                + " valid         = " + valid + "\n"
                + " traceHash     = " + traceHash + "\n"
                + " workflowHash  = " + workflowHash + "\n"
                + " timestamp     = " + timestamp + "\n"
                + "}";
    }
}