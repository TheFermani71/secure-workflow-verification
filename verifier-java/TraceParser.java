import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;

import org.json.simple.*;
import org.json.simple.parser.JSONParser;

public class TraceParser {

    public static ExecutionTrace parse(String filename) {

        List<TraceEntry> entries = new ArrayList<>();

        JSONParser parser = new JSONParser();

        try {

            Object obj = parser.parse(new FileReader(filename));
            JSONObject json = (JSONObject) obj;

            JSONArray array = (JSONArray) json.get("entries");

            for (Object o : array) {

                JSONObject e = (JSONObject) o;

                int varId = ((Long) e.get("var_id")).intValue();
                int op = ((Long) e.get("op")).intValue();

                String in1 = (String) e.get("in1_id");
                String in2 = (String) e.get("in2_id");
                String out = (String) e.get("out_id");

                long delta = ((Long) e.get("delta_us"));

                TraceEntry entry = new TraceEntry(varId, op, in1, in2, out, delta);

                entries.add(entry);
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

        return new ExecutionTrace(entries);
    }

}