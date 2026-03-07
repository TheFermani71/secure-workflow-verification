/*
WorkflowGraph

Represents the workflow graph extracted from the program.
The graph is implemented using an adjacency matrix.
*/

public class WorkflowGraph {

    private boolean[][] graph;

    public WorkflowGraph(int maxApi) {

        graph = new boolean[maxApi][maxApi];

    }

    /*
    Adds a valid transition between two APIs
    */

    public void addEdge(int from, int to) {

        graph[from][to] = true;

    }

    /*
    Checks whether a transition is valid
    */

    public boolean isValidTransition(int from, int to) {

        return graph[from][to];

    }

}