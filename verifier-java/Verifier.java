import java.util.*;

public class Verifier {

    private WorkflowGraph graph;
    private TemporalValidator temporal;

    public Verifier(WorkflowGraph graph) {
        this.graph = graph;
        this.temporal = new TemporalValidator();
    }

    public boolean verify(ExecutionTrace trace) {

        List<TraceEntry> entries = trace.getEntries();

        // DATA FLOW
        Set<String> produced = new HashSet<>();

        for (TraceEntry e : entries) {

            if (!e.in1.equals("00000000") && !produced.contains(e.in1)) {
                System.out.println("DataFlow error");
                return false;
            }

            if (!e.in2.equals("00000000") && !produced.contains(e.in2)) {
                System.out.println("DataFlow error");
                return false;
            }

            produced.add(e.out);
        }

        // WORKFLOW
        for (int i = 0; i < entries.size() - 1; i++) {

            String curr = entries.get(i).opName;
            String next = entries.get(i + 1).opName;

            System.out.println("Transition: " + curr + " -> " + next);

            if (!graph.isValidTransition(curr, next)) {
                System.out.println("Workflow error");
                return false;
            }
        }

        // TEMPORAL
        if (!temporal.validate(entries, trace.getCalTickUs())) {
            return false;
        }

        return true;
    }
}