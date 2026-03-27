import java.util.List;

public class ExecutionTrace {

    private List<TraceEntry> entries;

    public ExecutionTrace(List<TraceEntry> entries) {
        this.entries = entries;
    }

    public List<TraceEntry> getEntries() {
        return entries;
    }

    public int size() {
        return entries.size();
    }

}