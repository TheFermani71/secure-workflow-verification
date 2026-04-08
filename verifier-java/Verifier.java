import java.util.*;

public class Verifier {

    private WorkflowGraph graph;

    // Lookup pesi operazioni
    private Map<String, Double> opWeights;

    public Verifier(WorkflowGraph graph) {

        this.graph = graph;

        opWeights = new HashMap<>();

        // Doppio seal
        opWeights.put("ADD", 1.0);
        opWeights.put("SUB", 1.0);
        opWeights.put("MUL", 1.0);
        opWeights.put("DIV", 1.0);
        opWeights.put("GT", 1.0);
        opWeights.put("LT", 1.0);
        opWeights.put("EQ", 1.0);

        // Singolo seal
        opWeights.put("READ", 0.5);
        opWeights.put("CONST", 0.5);
        opWeights.put("ASSIGN", 0.5);
        opWeights.put("INIT", 0.5);
        opWeights.put("AVG_N", 0.5);

        // Trace only
        opWeights.put("BIND", 0.1);
        opWeights.put("WRITE", 0.1);
        opWeights.put("EVENT", 0.1);
    }

    public boolean verify(ExecutionTrace trace) {

        List<TraceEntry> entries = trace.getEntries();
        long calTick = trace.getCalTickUs();

        Set<String> produced = new HashSet<>();

        double totalFormal = 0;
        double totalReal = 0;

        for (int i = 0; i < entries.size(); i++) {

            TraceEntry e = entries.get(i);

            // ---------------- DATA FLOW ----------------

            if (e.in1 != null && !e.in1.equals("") && !e.in1.equals("00000000") && !produced.contains(e.in1)) {
                System.out.println("Missing input: " + e.in1);
                return false;
            }

            if (e.in2 != null && !e.in2.equals("") && !e.in2.equals("00000000") && !produced.contains(e.in2)) {
                System.out.println("Missing input: " + e.in2);
                return false;
            }

            if (e.out != null) {
                produced.add(e.out);
            }

            // ---------------- WORKFLOW ----------------

            if (i < entries.size() - 1) {

                int curr = e.op;
                int next = entries.get(i + 1).op;

                if (!graph.isValidTransition(curr, next)) {
                    System.out.println("Invalid transition");
                    return false;
                }
            }

            // ---------------- TIME ----------------

            totalReal += e.delta;

            double weight = opWeights.getOrDefault(e.opName, 1.0);
            totalFormal += weight * calTick;
        }

        // ---------------- PROPORTIONAL CHECK ----------------

        double ratio = totalReal / totalFormal;

        System.out.println("Real time: " + totalReal);
        System.out.println("Formal time: " + totalFormal);
        System.out.println("Ratio: " + ratio);

        if (ratio < 0.2 || ratio > 5.0) {
            System.out.println("Timing anomaly (proportional check)");
            return false;
        }

        return true;
    }
}