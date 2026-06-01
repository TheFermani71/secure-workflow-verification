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

System.out.println("Leaf:");
System.out.println("0x" + proof.getLeafHash());

System.out.println();

System.out.println("Root:");
System.out.println("0x" + proof.getRootHash());

System.out.println();

System.out.println("Proof:");

for (MerkleProofNode n : proof.getProofNodes()) {

    System.out.println(
            "0x" + n.getHash()
    );
}

System.out.println();

System.out.println("Directions:");

for (MerkleProofNode n : proof.getProofNodes()) {

    System.out.println(
            n.isLeftSibling()
    );
}

        System.out.println();

        System.out.println(
                "Proof valid : "
                        + valid
        );
    }
}