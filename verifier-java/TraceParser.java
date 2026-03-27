import java.io.*;
import java.util.*;

public class TraceParser {

    public static ExecutionTrace parse(String filename) {

        ArrayList<Integer> sequence = new ArrayList<>();
        ArrayList<Integer> deltas = new ArrayList<>();

        try {

            BufferedReader reader = new BufferedReader(new FileReader(filename));

            String line;

            while((line = reader.readLine()) != null) {

                line = line.trim();

                if(line.equals("TRACE") || line.equals("END") || line.isEmpty()) {
                    continue;
                }

                String[] parts = line.split(" ");

                String apiName = parts[0];
                int delta = Integer.parseInt(parts[1]);

                sequence.add(mapApiToId(apiName));
                deltas.add(delta);
            }

            reader.close();

        } catch(IOException e) {
            e.printStackTrace();
        }

        int[] seqArray = sequence.stream().mapToInt(i -> i).toArray();
        int[] deltaArray = deltas.stream().mapToInt(i -> i).toArray();

        return new ExecutionTrace(seqArray, deltaArray);
    }

    private static int mapApiToId(String api) {

        switch(api) {
            case "INIT": return 1;
            case "ASSIGN": return 2;
            case "ADD": return 3;
            case "DIV": return 4;
            default:
                throw new RuntimeException("Unknown API: " + api);
        }
    }
}