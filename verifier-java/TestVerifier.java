public class TestVerifier {

    public static void main(String[] args) {

        WorkflowGraph graph = WorkflowLoader.loadGraph("../extractor/workflow_graph.json");

        ExecutionTrace trace = TraceParser.parse("trace.txt");

        Verifier verifier = new Verifier(graph);

        boolean result = verifier.verify(trace);

        if(result) {

            System.out.println("TRACE VALID");

        } else {

            System.out.println("TRACE INVALID");

        }

    }

}