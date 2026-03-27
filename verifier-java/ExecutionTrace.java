public class ExecutionTrace {

    private int[] apiSequence;
    private int[] timeDeltas;

    public ExecutionTrace(int[] apiSequence, int[] timeDeltas) {

        this.apiSequence = apiSequence;
        this.timeDeltas = timeDeltas;
    }

    public int[] getSequence() {
        return apiSequence;
    }

    public int[] getTimeDeltas() {
        return timeDeltas;
    }

    public int length() {
        return apiSequence.length;
    }
}