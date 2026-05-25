import java.util.List;

public class ExecutionTrace {

    public int apiVersion;

    public String deviceId;

    public long calTickUs;

    public List<TraceEntry> entries;

    public ExecutionTrace() {
    }

    public ExecutionTrace(
            List<TraceEntry> entries,
            long calTickUs
    ) {

        this.entries =
                entries;

        this.calTickUs =
                calTickUs;
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
     * Debug
     */
    @Override
    public String toString() {

        return "[ExecutionTrace entries="
                + entries.size()
                + "]";
    }
}