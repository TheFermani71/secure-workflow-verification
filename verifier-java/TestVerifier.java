public class TestVerifier {

    public static void main(String[] args) {

        ExecutionTrace trace = TraceParser.parse("trace.json");
        WorkflowGraph graph = WorkflowLoader.load("../extractor/workflow_graph.json");

        Verifier verifier = new Verifier(graph);

        if (verifier.verify(trace)) {
            System.out.println("TRACE VALID");
        } else {
            System.out.println("TRACE INVALID");
        }
    }
}