import java.io.FileReader;
import java.util.*;
import org.json.simple.*;
import org.json.simple.parser.JSONParser;

public class TraceParser {

    public static ExecutionTrace parse(String filename) {

        List<TraceEntry> list = new ArrayList<>();
        long calTick = 0;

        try {
            JSONParser parser = new JSONParser();
            JSONObject obj = (JSONObject) parser.parse(new FileReader(filename));

            calTick = (Long) obj.get("cal_tick_us");

            JSONArray entries = (JSONArray) obj.get("entries");

            for (Object o : entries) {
                JSONObject e = (JSONObject) o;

                String opName = (String) e.get("op_name");
                String in1 = (String) e.get("in1_id");
                String in2 = (String) e.get("in2_id");
                String out = (String) e.get("out_id");
                long delta = (Long) e.get("delta_us");

                list.add(new TraceEntry(opName, in1, in2, out, delta));
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

        return new ExecutionTrace(list, calTick);
    }
}