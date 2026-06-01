import java.util.ArrayList;
import java.util.List;

public class MerkleTreeBuilder {

    public static String computeRoot(
            ExecutionTrace trace
    ) {

        if (trace == null) {

            throw new IllegalArgumentException(
                    "Trace is null"
            );
        }

        if (trace.entries == null ||
                trace.entries.isEmpty()) {

            throw new IllegalArgumentException(
                    "Trace has no entries"
            );
        }

        List<String> entryHashes =
                new ArrayList<>();

        for (TraceEntry entry : trace.entries) {

            if (entry.entryHash == null ||
                    entry.entryHash.isBlank()) {

                throw new IllegalArgumentException(
                        "Missing entryHash at seq="
                                + entry.seq
                );
            }

            entryHashes.add(
                    entry.entryHash
            );
        }

        MerkleTree tree =
                new MerkleTree(
                        entryHashes
                );

        return tree.getRoot();
    }

    public static MerkleProof buildProof(
            ExecutionTrace trace,
            int index
    ) {

        List<String> entryHashes =
                new ArrayList<>();

        for (TraceEntry entry : trace.entries) {

            entryHashes.add(
                    entry.entryHash
            );
        }

        MerkleTree tree =
                new MerkleTree(
                        entryHashes
                );

        return tree.getProof(
                index
        );
    }
}