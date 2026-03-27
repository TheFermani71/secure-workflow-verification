import java.util.HashMap;
import java.util.Map;

public class WorkflowGraph {

    private Map<String, Integer> transitions;

    public WorkflowGraph() {
        transitions = new HashMap<>();
    }

    private String key(int from, int to) {
        return from + "-" + to;
    }

    public void addEdge(int from, int to, int time) {
        transitions.put(key(from, to), time);
    }

    public boolean isValidTransition(int from, int to) {
        return transitions.containsKey(key(from, to));
    }

    public int getTime(int from, int to) {
        return transitions.getOrDefault(key(from, to), -1);
    }
}