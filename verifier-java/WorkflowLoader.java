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
                    (JSONArray) root.get("nodes");

            for (Object obj : nodes) {

                JSONObject n =
                        (JSONObject) obj;

                int id =
                        ((Long) n.get("id")).intValue();

                String type =
                        (String) n.get("type");

                String name =
                        (String) n.get("name");

                graph.addNode(
                        id,
                        type,
                        name
                );
            }

            /*
             * Edges
             */
            JSONArray edges =
                    (JSONArray) root.get("edges");

            for (Object obj : edges) {

                JSONObject e =
                        (JSONObject) obj;

                int from =
                        ((Long) e.get("from")).intValue();

                int to =
                        ((Long) e.get("to")).intValue();

                graph.addEdge(
                        from,
                        to
                );
            }

            System.out.println(
                    "[WorkflowLoader] Graph loaded"
            );

            System.out.println(
                    " Nodes : "
                            + graph.nodes.size()
            );

            System.out.println(
                    " Edges : "
                            + graph.edges.size()
            );

        } catch (Exception ex) {

            ex.printStackTrace();
        }

        return graph;
    }
}