import java.io.FileReader;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;

public class TraceParser {

    public static ExecutionTrace parse(String path) {

        ExecutionTrace trace = new ExecutionTrace();

        try {

            JSONParser parser = new JSONParser();

            JSONObject root =
                    (JSONObject) parser.parse(new FileReader(path));

            /*
             * Header JSON
             */
            trace.apiVersion =
                    ((Long) root.get("sv_api_version")).intValue();

            trace.deviceId =
                    (String) root.get("device_id");

            trace.calTickUs =
                    ((Long) root.get("cal_tick_us")).longValue();

            /*
             * Entries
             */
            JSONArray entries =
                    (JSONArray) root.get("entries");

            for (Object obj : entries) {

                JSONObject e = (JSONObject) obj;

                TraceEntry entry = new TraceEntry();

                entry.seq =
                        ((Long) e.get("seq")).intValue();

                entry.op =
                        ((Long) e.get("op")).intValue();

                entry.opName =
                        (String) e.get("op_name");

                entry.outId =
                        (String) e.get("out_id");

                entry.in1Id =
                        (String) e.get("in1_id");

                entry.in2Id =
                        (String) e.get("in2_id");

                entry.varId =
                        ((Long) e.get("var_id")).intValue();

                entry.varName =
                        (String) e.get("var_name");

                /*
                 * v6:
                 * total elapsed time
                 */
                entry.deltaUs =
                        ((Long) e.get("delta_us")).longValue();

                /*
                 * v6:
                 * trusted kernel timing
                 */
                entry.deltaApiUs =
                        ((Long) e.get("delta_api_us")).longValue();

                /*
                 * Derived application timing
                 */
                entry.deltaGapUs =
                        entry.deltaUs - entry.deltaApiUs;

                entry.chainTag =
                        (String) e.get("chain_tag");

                trace.entries.add(entry);
            }

            System.out.println(
                    "[TraceParser] Loaded trace:"
            );

            System.out.println(
                    "  API version : "
                            + trace.apiVersion
            );

            System.out.println(
                    "  Device      : "
                            + trace.deviceId
            );

            System.out.println(
                    "  Entries     : "
                            + trace.entries.size()
            );

            System.out.println(
                    "  cal_tick_us : "
                            + trace.calTickUs
            );

        } catch (Exception ex) {

            ex.printStackTrace();
        }

        return trace;
    }
}