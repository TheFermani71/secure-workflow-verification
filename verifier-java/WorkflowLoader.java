import java.io.FileReader;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;

public class WorkflowLoader {

    public static WorkflowGraph loadGraph(
            String path
    ) {

        WorkflowGraph graph =
                new WorkflowGraph();

        try {

            JSONParser parser =
                    new JSONParser();

            JSONObject root =
                    (JSONObject) parser.parse(
                            new FileReader(path)
                    );

            /*
             * Nodes
             */
            JSONArray nodes =
                    (JSONArray) root.get(
                            "nodes"
                    );

            for (Object obj : nodes) {

                JSONObject node =
                        (JSONObject) obj;

                String label =
                        (String) node.get(
                                "label"
                        );

                graph.addNode(label);
            }

            /*
             * Edges
             */
            JSONArray edges =
                    (JSONArray) root.get(
                            "edges"
                    );

            for (Object obj : edges) {

                JSONArray edge =
                        (JSONArray) obj;

                String from =
                        (String) edge.get(0);

                String to =
                        (String) edge.get(1);

                graph.addEdge(
                        from,
                        to
                );
            }

            /*
             * Debug
             */
            System.out.println();

            System.out.println(
                    "[WorkflowLoader] Graph loaded"
            );

            System.out.println(
                    " Nodes : "
                            + graph.nodes.size()
            );

            int edgeCount = 0;

            for (String node : graph.nodes) {

                edgeCount +=
                        graph.adjacency
                                .get(node)
                                .size();
            }

            System.out.println(
                    " Edges : "
                            + edgeCount
            );

        } catch (Exception ex) {

            ex.printStackTrace();
        }

        return graph;
    }
}