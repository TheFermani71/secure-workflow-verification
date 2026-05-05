import java.util.*;

public class WorkflowGraph {

    private Map<String, Set<String>> graph = new HashMap<>();

    public void addEdge(String from, String to) {
        graph.putIfAbsent(from, new HashSet<>());
        graph.get(from).add(to);
    }

    public boolean isValidTransition(String from, String to) {
        return graph.containsKey(from) && graph.get(from).contains(to);
    }
}