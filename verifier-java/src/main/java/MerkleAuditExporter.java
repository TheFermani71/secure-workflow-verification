public class MerkleAuditExporter {

    public static void main(
            String[] args
    ) {

        ExecutionTrace trace =

                TraceParser.parseTrace(
                        "../extractor/trace.json"
                );

        String root =

                MerkleTreeBuilder.computeRoot(
                        trace
                );

        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " MERKLE AUDIT EXPORT"
        );

        System.out.println(
                "========================================"
        );

        System.out.println();

        System.out.println(
                "Root (hex):"
        );

        System.out.println(
                root
        );

        System.out.println();

        System.out.println(
                "Root (bytes32):"
        );

        System.out.println(
                "0x" + root
        );
    }
}