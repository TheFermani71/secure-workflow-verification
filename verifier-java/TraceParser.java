/*
TraceParser

Reads a trace file produced by the device and converts it
into an ExecutionTrace object.
*/

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

public class TraceParser {

    public static ExecutionTrace parse(String filename) {

        ArrayList<Integer> sequence = new ArrayList<>();

        try {

            BufferedReader reader = new BufferedReader(new FileReader(filename));

            String line;

            while((line = reader.readLine()) != null) {

                line = line.trim();

                if(line.equals("TRACE") || line.equals("END") || line.isEmpty()) {

                    continue;

                }

                int apiId = mapApiToId(line);

                sequence.add(apiId);

            }

            reader.close();

        } catch(IOException e) {

            System.out.println("Error reading trace file");
            e.printStackTrace();

        }

        int[] array = new int[sequence.size()];

        for(int i = 0; i < sequence.size(); i++) {

            array[i] = sequence.get(i);

        }

        return new ExecutionTrace(array);

    }

    private static int mapApiToId(String api) {

        switch(api) {

            case "INIT":
                return 1;

            case "ASSIGN":
                return 2;

            case "ADD":
                return 3;

            case "DIV":
                return 4;

            default:
                throw new RuntimeException("Unknown API in trace: " + api);

        }

    }

}