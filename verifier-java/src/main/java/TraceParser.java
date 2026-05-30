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

        int apiVersion = 0;

        String deviceId = null;

        String traceMerkleRoot = null;

        try {

            JSONParser parser =
                    new JSONParser();

            JSONObject root =
                    (JSONObject) parser.parse(
                            new FileReader(path)
                    );

            /*
             * Metadata
             */
            apiVersion =
                    ((Long) root.get(
                            "sv_api_version"
                    )).intValue();

            deviceId =
                    (String) root.get(
                            "device_id"
                    );

            calTickUs =
                    (Long) root.get(
                            "cal_tick_us"
                    );

            traceMerkleRoot =
                    (String) root.get(
                            "trace_merkle_root"
                    );

            /*
             * Entries
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

                /*
                 * HASH CHAIN
                 */
                entry.prevHash =
                        (String) e.get(
                                "prev_hash"
                        );

                entry.entryHash =
                        (String) e.get(
                                "entry_hash"
                        );

                entries.add(entry);
            }

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

            System.out.println(
                    " Merkle Root : "
                            + traceMerkleRoot
            );

        } catch (Exception ex) {

            ex.printStackTrace();
        }

        return new ExecutionTrace(

                apiVersion,

                deviceId,

                calTickUs,

                traceMerkleRoot,

                entries
        );
    }
}