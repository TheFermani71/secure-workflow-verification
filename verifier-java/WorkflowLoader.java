import java.io.FileReader;
import org.json.simple.*;
import org.json.simple.parser.JSONParser;

public class WorkflowLoader {

    public static WorkflowGraph loadGraph(String filename) {

        WorkflowGraph graph = new WorkflowGraph();

        JSONParser parser = new JSONParser();

        try {

            Object obj = parser.parse(new FileReader(filename));

            JSONObject jsonObject = (JSONObject) obj;

            JSONArray edges = (JSONArray) jsonObject.get("edges");

            for(Object edge : edges) {

                JSONObject e = (JSONObject) edge;

                int from = ((Long) e.get("from")).intValue();
                int to = ((Long) e.get("to")).intValue();
                int time = ((Long) e.get("time")).intValue();

                graph.addEdge(from, to, time);
            }

        } catch(Exception e) {
            e.printStackTrace();
        }

        return graph;
    }
}