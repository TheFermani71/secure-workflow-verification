import java.io.FileReader;
import org.json.simple.*;
import org.json.simple.parser.JSONParser;

public class WorkflowLoader {

    public static WorkflowGraph load(String filename) {

        WorkflowGraph graph = new WorkflowGraph();

        try {
            JSONParser parser = new JSONParser();
            JSONObject obj = (JSONObject) parser.parse(new FileReader(filename));

            JSONArray edges = (JSONArray) obj.get("edges");

            for (Object o : edges) {
                JSONArray e = (JSONArray) o;

                String from = (String) e.get(0);
                String to = (String) e.get(1);

                graph.addEdge(from, to);
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

        return graph;
    }
}