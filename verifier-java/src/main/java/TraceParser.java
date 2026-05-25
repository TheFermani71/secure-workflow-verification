import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;

public class TraceParser {

    public static ExecutionTrace parseTrace(
            String path
    ) {

        List<TraceEntry> entries =
                new ArrayList<>();

        long calTickUs = 0;

        try {

            JSONParser parser =
                    new JSONParser();

            JSONObject root =
                    (JSONObject) parser.parse(
                            new FileReader(path)
                    );

            /*
             * Global metadata
             */
            long apiVersion =
                    (Long) root.get(
                            "sv_api_version"
                    );

            String deviceId =
                    (String) root.get(
                            "device_id"
                    );

            calTickUs =
                    (Long) root.get(
                            "cal_tick_us"
                    );

            /*
             * Trace entries
             */
            JSONArray jsonEntries =
                    (JSONArray) root.get(
                            "entries"
                    );

            for (Object obj : jsonEntries) {

                JSONObject e =
                        (JSONObject) obj;

                TraceEntry entry =
                        new TraceEntry();

                entry.seq =
                        ((Long) e.get(
                                "seq"
                        )).intValue();

                entry.op =
                        ((Long) e.get(
                                "op"
                        )).intValue();

                entry.opName =
                        (String) e.get(
                                "op_name"
                        );

                entry.outId =
                        (String) e.get(
                                "out_id"
                        );

                entry.in1Id =
                        (String) e.get(
                                "in1_id"
                        );

                entry.in2Id =
                        (String) e.get(
                                "in2_id"
                        );

                entry.varId =
                        ((Long) e.get(
                                "var_id"
                        )).intValue();

                entry.varName =
                        (String) e.get(
                                "var_name"
                        );

                entry.deltaUs =
                        (Long) e.get(
                                "delta_us"
                        );

                entry.deltaApiUs =
                        (Long) e.get(
                                "delta_api_us"
                        );

                entry.chainTag =
                        (String) e.get(
                                "chain_tag"
                        );

                entries.add(entry);
            }

            /*
             * Debug output
             */
            System.out.println();

            System.out.println(
                    "[TraceParser] Loaded trace:"
            );

            System.out.println(
                    " API version : "
                            + apiVersion
            );

            System.out.println(
                    " Device      : "
                            + deviceId
            );

            System.out.println(
                    " Entries     : "
                            + entries.size()
            );

            System.out.println(
                    " cal_tick_us : "
                            + calTickUs
            );

        } catch (Exception ex) {

            ex.printStackTrace();
        }

        return new ExecutionTrace(
                entries,
                calTickUs
        );
    }
}