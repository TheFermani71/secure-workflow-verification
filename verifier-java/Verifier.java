/*
Verifier

Validates execution traces against the workflow graph.
*/

public class Verifier {

    private WorkflowGraph graph;

    public Verifier(WorkflowGraph graph) {

        this.graph = graph;

    }

    /*
    Verifies that the execution trace follows the workflow graph
    */

    public boolean verify(ExecutionTrace trace) {

        int[] sequence = trace.getSequence();

        for(int i = 0; i < sequence.length - 1; i++) {

            int current = sequence[i];
            int next = sequence[i + 1];

            if(!graph.isValidTransition(current, next)) {

                return false;

            }

        }

        return true;

    }

}