import java.util.HashMap;
import java.util.Map;

public class ApiTimingValidator {

    /*
     * Symbolic cost table
     *
     * Derived from firmware documentation
     * (SV-API v6).
     *
     * Reference:
     * ADD = cost 4
     */
    private final Map<String, Integer> symbolicCost;

    /*
     * Allowed tolerance
     *
     * ±50%
     */
    private static final double TOLERANCE = 0.50;

    public ApiTimingValidator() {

        symbolicCost = new HashMap<>();

        /*
         * Cost = 2
         */
        symbolicCost.put("INIT", 2);
        symbolicCost.put("ASSIGN", 2);
        symbolicCost.put("CONST", 2);
        symbolicCost.put("READ", 2);
        symbolicCost.put("WRITE", 2);
        symbolicCost.put("EVENT", 2);

        /*
         * Cost = 4
         */
        symbolicCost.put("ADD", 4);
        symbolicCost.put("SUB", 4);
        symbolicCost.put("MUL", 4);
        symbolicCost.put("DIV", 4);
        symbolicCost.put("GT", 4);
        symbolicCost.put("LT", 4);
        symbolicCost.put("EQ", 4);

        /*
         * Cost = 3
         */
        symbolicCost.put("CNT", 3);

        /*
         * AVG_N:
         * treated as heavy API
         */
        symbolicCost.put("AVG_N", 4);
    }

    /*
     * Validate all entries
     */
    public boolean validate(ExecutionTrace trace) {

        boolean allValid = true;

        System.out.println();
        System.out.println(
                "========================================"
        );
        System.out.println(
                " API TIMING VALIDATION (SV-API v6)"
        );
        System.out.println(
                "========================================"
        );

        for (TraceEntry entry : trace.entries) {

            /*
             * Skip first entry:
             * no previous timing reference
             */
            if (entry.seq == 0) {

                System.out.println(
                        "[SKIP] seq=0"
                );

                continue;
            }

            /*
             * Ignore export API
             */
            if (entry.opName.equals("TRACE_EXPORT_JSON")) {

                System.out.println(
                        "[SKIP] TRACE_EXPORT_JSON"
                );

                continue;
            }

            boolean valid =
                    validateEntry(entry, trace.calTickUs);

            if (!valid) {
                allValid = false;
            }
        }

        System.out.println();

        if (allValid) {

            System.out.println(
                    "[API TIMING] VALID"
            );

        } else {

            System.out.println(
                    "[API TIMING] INVALID"
            );
        }

        return allValid;
    }

    /*
     * Validate single API timing
     */
    private boolean validateEntry(
            TraceEntry entry,
            long calTickUs
    ) {

        Integer cost =
                symbolicCost.get(entry.opName);

        if (cost == null) {

            System.out.println(
                    "[WARN] Unknown API: "
                            + entry.opName
            );

            return true;
        }

        /*
         * ADD = reference cost 4
         */
        double expected =
                ((double) cost / 4.0)
                        * calTickUs;

        double min =
                expected * (1.0 - TOLERANCE);

        double max =
                expected * (1.0 + TOLERANCE);

        long observed =
                entry.deltaApiUs;

        boolean valid =
                observed >= min
                        && observed <= max;

        System.out.printf(
                "[%s] expected=%.1f us | observed=%d us | range=[%.1f - %.1f] -> %s%n",
                entry.opName,
                expected,
                observed,
                min,
                max,
                valid ? "OK" : "FAIL"
        );

        return valid;
    }
}