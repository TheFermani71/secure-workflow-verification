public class MerkleProofNode {

    private final String hash;

    private final boolean leftSibling;

    public MerkleProofNode(
            String hash,
            boolean leftSibling
    ) {

        this.hash =
                hash;

        this.leftSibling =
                leftSibling;
    }

    public String getHash() {

        return hash;
    }

    public boolean isLeftSibling() {

        return leftSibling;
    }

    @Override
    public String toString() {

        return "{hash="

                + hash

                + ", leftSibling="

                + leftSibling

                + "}";
    }
}