public class VerificationRecord {

    /*
     * Device
     */
    public String deviceId;

    /*
     * Workflow mode
     */
    public String workflowMode;

    /*
     * Verifier mode
     */
    public String verifierMode;

    /*
     * Workflow hash
     */
    public String workflowHash;

    /*
     * Trace hash
     */
    public String traceHash;

    /*
     * Merkle root
     */
    public String traceMerkleRoot;

    /*
     * Trace length
     */
    public long traceLength;

    /*
     * Verification result
     */
    public boolean valid;

    /*
     * Blockchain timestamp
     */
    public long timestamp;

    public VerificationRecord(

            String deviceId,

            String workflowMode,

            String verifierMode,

            String workflowHash,

            String traceHash,

            String traceMerkleRoot,

            long traceLength,

            boolean valid,

            long timestamp
    ) {

        this.deviceId =
                deviceId;

        this.workflowMode =
                workflowMode;

        this.verifierMode =
                verifierMode;

        this.workflowHash =
                workflowHash;

        this.traceHash =
                traceHash;

        this.traceMerkleRoot =
                traceMerkleRoot;

        this.traceLength =
                traceLength;

        this.valid =
                valid;

        this.timestamp =
                timestamp;
    }

    @Override
    public String toString() {

        return "\nVerificationRecord {\n"

                + " deviceId        = "
                + deviceId
                + "\n"

                + " workflowMode   = "
                + workflowMode
                + "\n"

                + " verifierMode   = "
                + verifierMode
                + "\n"

                + " workflowHash   = "
                + workflowHash
                + "\n"

                + " traceHash      = "
                + traceHash
                + "\n"

                + " merkleRoot     = "
                + traceMerkleRoot
                + "\n"

                + " traceLength    = "
                + traceLength
                + "\n"

                + " valid          = "
                + valid
                + "\n"

                + " timestamp      = "
                + timestamp
                + "\n"

                + "}";
    }
}