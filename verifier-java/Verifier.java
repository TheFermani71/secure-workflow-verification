import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class Verifier {

    private WorkflowGraph graph;

    public Verifier(WorkflowGraph graph) {
        this.graph = graph;
    }

    public boolean verify(ExecutionTrace trace) {

        List<TraceEntry> entries = trace.getEntries();

        Set<String> producedValues = new HashSet<>();

        for (int i = 0; i < entries.size(); i++) {

            TraceEntry current = entries.get(i);

            // ---------------------------
            // DATA FLOW CHECK
            // ---------------------------

            if (current.in1 != null && !current.in1.equals("") && !producedValues.contains(current.in1)) {
                System.out.println("Invalid data flow: missing input " + current.in1);
                return false;
            }

            if (current.in2 != null && !current.in2.equals("") && !producedValues.contains(current.in2)) {
                System.out.println("Invalid data flow: missing input " + current.in2);
                return false;
            }

            // Add output to produced set
            if (current.out != null) {
                producedValues.add(current.out);
            }

            // ---------------------------
            // WORKFLOW CHECK
            // ---------------------------

            if (i < entries.size() - 1) {

                int currOp = current.op;
                int nextOp = entries.get(i + 1).op;

                if (!graph.isValidTransition(currOp, nextOp)) {
                    System.out.println("Invalid transition: " + currOp + " → " + nextOp);
                    return false;
                }
            }

            // ---------------------------
            // TIME CHECK (semplice)
            // ---------------------------

            if (current.delta > 1000000) {
                System.out.println("Timing anomaly detected");
                return false;
            }
        }

        return true;
    }
}