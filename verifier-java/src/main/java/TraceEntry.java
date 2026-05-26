public class TraceEntry {

    public int seq;

    public int op;

    public String opName;

    public String outId;

    public String in1Id;

    public String in2Id;

    public int varId;

    public String varName;

    /*
     * deltaUs:
     *
     * Total elapsed time since previous entry.
     */
    public long deltaUs;

    /*
     * Internal API timing.
     */
    public long deltaApiUs;

    /*
     * External gap timing.
     */
    public long deltaGapUs;

    public String chainTag;

    /*
     * Hash chain fields
     */
    public String prevHash;

    public String entryHash;

    @Override
    public String toString() {

        return "["
                + seq
                + "] "
                + opName
                + " | delta="
                + deltaUs
                + " us";
    }
}