public class TestMerkle {

    public static void main(
            String[] args
    ) {

        ExecutionTrace trace =

                TraceParser.parseTrace(

                        "../extractor/trace.json"
                );

        MerkleProof proof =

                MerkleTreeBuilder.buildProof(

                        trace,

                        2
                );

        boolean valid =

                MerkleProofVerifier.verify(
                        proof
                );

        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " MERKLE TEST"
        );

        System.out.println(
                "========================================"
        );

        System.out.println(
                proof
        );

        System.out.println();

        System.out.println(
                "Proof valid : "
                        + valid
        );
    }
}