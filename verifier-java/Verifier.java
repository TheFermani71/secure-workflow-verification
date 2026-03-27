public class Verifier {

    private WorkflowGraph graph;

    public Verifier(WorkflowGraph graph) {
        this.graph = graph;
    }

    public boolean verify(ExecutionTrace trace) {

        int[] seq = trace.getSequence();
        int[] deltas = trace.getTimeDeltas();

        for(int i = 0; i < seq.length - 1; i++) {

            int current = seq[i];
            int next = seq[i + 1];
            int observedTime = deltas[i + 1];

            if(!graph.isValidTransition(current, next)) {
                return false;
            }

            int expectedTime = graph.getTime(current, next);

            if(Math.abs(observedTime - expectedTime) > 1) {
                return false;
            }
        }

        return true;
    }
}