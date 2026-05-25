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
     * Tempo totale trascorso rispetto
     * all'entry precedente.
     *
     * Include:
     * - tempo API
     * - tempo applicativo
     * - eventuali gap/interruzioni
     */
    public long deltaUs;

    /*
     * deltaApiUs:
     *
     * Tempo interno della singola API.
     *
     * Include:
     * - AES-GCM
     * - seal/unseal
     * - HMAC
     * - logica kernel
     *
     * Valore trusted:
     * misurato internamente dal firmware.
     */
    public long deltaApiUs;

    /*
     * deltaGapUs:
     *
     * Tempo intermedio tra:
     * - fine API precedente
     * - inizio API corrente
     *
     * NON arriva dal firmware.
     *
     * Viene derivato dal verifier:
     *
     * deltaGapUs =
     *     deltaUs - deltaApiUs
     *
     * Questo rappresenta:
     * - if
     * - for
     * - coroutine scheduling
     * - codice utente
     * - ritardi artificiali
     */
    public long deltaGapUs;

    public String chainTag;

    @Override
    public String toString() {

        return "["
                + seq
                + "] "
                + opName
                + " | delta="
                + deltaUs
                + " us"
                + " | api="
                + deltaApiUs
                + " us"
                + " | gap="
                + deltaGapUs
                + " us";
    }
}