import java.util.List;

public class ExecutionTrace {

    private List<TraceEntry> entries;
    private long calTickUs;

    public ExecutionTrace(List<TraceEntry> entries, long calTickUs) {
        this.entries = entries;
        this.calTickUs = calTickUs;
    }

    public List<TraceEntry> getEntries() {
        return entries;
    }

    public long getCalTickUs() {
        return calTickUs;
    }

    public int size() {
        return entries.size();
    }
}