/*
TestVerifier

Simple test program for the workflow verifier.
*/

public class TestVerifier {

    public static void main(String[] args) {

        WorkflowGraph graph = new WorkflowGraph(16);

        // define valid transitions

        graph.addEdge(1,2);
        graph.addEdge(2,3);
        graph.addEdge(3,4);

        int[] sequence = {1,2,3,4};

        ExecutionTrace trace = new ExecutionTrace(sequence);

        Verifier verifier = new Verifier(graph);

        boolean result = verifier.verify(trace);

        if(result) {

            System.out.println("TRACE VALID");

        }
        else {

            System.out.println("TRACE INVALID");

        }

    }

}