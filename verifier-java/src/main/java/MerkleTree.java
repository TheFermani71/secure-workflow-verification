import java.security.MessageDigest;
import java.util.ArrayList;
import java.util.List;

public class MerkleTree {

    private final List<String> leaves;

    private String root;

    public MerkleTree(
            List<String> entryHashes
    ) {

        this.leaves =
                new ArrayList<>();

        for (String hash : entryHashes) {

            this.leaves.add(
                    sha256(hash)
            );
        }

        this.root =
                buildRoot(
                        this.leaves
                );
    }

    public String getRoot() {

        return root;
    }

    public MerkleProof getProof(
            int index
    ) {

        if (index < 0 ||
                index >= leaves.size()) {

            throw new IllegalArgumentException(
                    "Invalid leaf index"
            );
        }

        List<MerkleProofNode> proof =
                new ArrayList<>();

        List<String> current =
                new ArrayList<>(leaves);

        int currentIndex =
                index;

        while (current.size() > 1) {

            if (current.size() % 2 != 0) {

                current.add(
                        current.get(
                                current.size() - 1
                        )
                );
            }

            List<String> next =
                    new ArrayList<>();

            for (int i = 0;
                 i < current.size();
                 i += 2) {

                String left =
                        current.get(i);

                String right =
                        current.get(i + 1);

                String parent =
                        MerkleProofVerifier.hashPair(
                                left,
                                right
                        );

                next.add(parent);

                if (i == currentIndex ||
                        i + 1 == currentIndex) {

                    if (i == currentIndex) {

                        proof.add(

                                new MerkleProofNode(

                                        right,

                                        false
                                )
                        );

                    } else {

                        proof.add(

                                new MerkleProofNode(

                                        left,

                                        true
                                )
                        );
                    }

                    currentIndex =
                            next.size() - 1;
                }
            }

            current = next;
        }

        return new MerkleProof(

                leaves.get(index),

                root,

                proof
        );
    }

    private String buildRoot(
            List<String> hashes
    ) {

        List<String> current =
                new ArrayList<>(hashes);

        while (current.size() > 1) {

            if (current.size() % 2 != 0) {

                current.add(
                        current.get(
                                current.size() - 1
                        )
                );
            }

            List<String> next =
                    new ArrayList<>();

            for (int i = 0;
                 i < current.size();
                 i += 2) {

                String left =
                        current.get(i);

                String right =
                        current.get(i + 1);

                String parent =
                        MerkleProofVerifier.hashPair(
                                left,
                                right
                        );

                next.add(parent);
            }

            current = next;
        }

        return current.get(0);
    }

    private static String sha256(
            String input
    ) {

        try {

            MessageDigest digest =

                    MessageDigest.getInstance(
                            "SHA-256"
                    );

            byte[] hash =

                    digest.digest(
                            input.getBytes(
                                    "UTF-8"
                            )
                    );

            StringBuilder sb =
                    new StringBuilder();

            for (byte b : hash) {

                sb.append(

                        String.format(
                                "%02x",
                                b
                        )
                );
            }

            return sb.toString();

        } catch (Exception e) {

            throw new RuntimeException(
                    e
            );
        }
    }
}