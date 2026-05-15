import java.util.ArrayList;
import java.util.List;

public class ExecutionTrace {

    /*
     * SV-API protocol version
     */
    public int apiVersion;

    /*
     * Device identifier
     */
    public String deviceId;

    /*
     * Calibration reference
     *
     * Based on sv_add()
     */
    public long calTickUs;

    /*
     * Runtime trace entries
     */
    public List<TraceEntry> entries;

    public ExecutionTrace() {

        this.entries = new ArrayList<>();
    }
}