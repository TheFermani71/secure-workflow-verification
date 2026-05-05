import java.util.List;

public class TemporalValidator {

    public boolean validate(List<TraceEntry> entries, long calTick) {

        double real = 0;
        double formal = 0;

        for (TraceEntry e : entries) {
            real += e.delta;
            formal += getWeight(e.opName) * calTick;
        }

        double ratio = real / formal;

        System.out.println("T_real: " + real);
        System.out.println("T_formal: " + formal);
        System.out.println("Ratio: " + ratio);

        return ratio >= 0.2 && ratio <= 5.0;
    }

    private double getWeight(String op) {

        switch (op) {
            case "GT":
                return 1.0;

            case "INIT":
            case "CONST":
            case "AVG_N":
                return 0.5;

            case "WRITE":
                return 0.1;

            default:
                return 1.0;
        }
    }
}