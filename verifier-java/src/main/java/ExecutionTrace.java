import java.util.List;

public class ExecutionTrace {

    public int apiVersion;

    public String deviceId;

    public long calTickUs;

    /*
     * Merkle Root
     */
    public String traceMerkleRoot;

    public List<TraceEntry> entries;

    public ExecutionTrace() {
    }

    public ExecutionTrace(

            int apiVersion,

            String deviceId,

            long calTickUs,

            String traceMerkleRoot,

            List<TraceEntry> entries
    ) {

        this.apiVersion =
                apiVersion;

        this.deviceId =
                deviceId;

        this.calTickUs =
                calTickUs;

        this.traceMerkleRoot =
                traceMerkleRoot;

        this.entries =
                entries;
    }

    /*
     * Get entries
     */
    public List<TraceEntry> getEntries() {

        return entries;
    }

    /*
     * Get calibration tick
     */
    public long getCalTickUs() {

        return calTickUs;
    }

    /*
     * Get Merkle Root
     */
    public String getTraceMerkleRoot() {

        return traceMerkleRoot;
    }

    /*
     * Debug
     */
    @Override
    public String toString() {

        return "[ExecutionTrace entries="
                + entries.size()
                + ", merkleRoot="
                + traceMerkleRoot
                + "]";
    }
}