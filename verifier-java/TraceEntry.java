public class TraceEntry {

    public String opName;
    public String in1;
    public String in2;
    public String out;
    public long delta;

    public TraceEntry(String opName, String in1, String in2, String out, long delta) {
        this.opName = opName;
        this.in1 = in1;
        this.in2 = in2;
        this.out = out;
        this.delta = delta;
    }
}