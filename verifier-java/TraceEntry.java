public class TraceEntry {

    public int varId;
    public int op;
    public String in1;
    public String in2;
    public String out;
    public long delta;

    public TraceEntry(int varId, int op, String in1, String in2, String out, long delta) {
        this.varId = varId;
        this.op = op;
        this.in1 = in1;
        this.in2 = in2;
        this.out = out;
        this.delta = delta;
    }

}