import java.util.List;

public class MerkleProof {

    private final String leafHash;

    private final String rootHash;

    private final List<MerkleProofNode> proofNodes;

    public MerkleProof(

            String leafHash,

            String rootHash,

            List<MerkleProofNode> proofNodes
    ) {

        this.leafHash =
                leafHash;

        this.rootHash =
                rootHash;

        this.proofNodes =
                proofNodes;
    }

    public String getLeafHash() {

        return leafHash;
    }

    public String getRootHash() {

        return rootHash;
    }

    public List<MerkleProofNode> getProofNodes() {

        return proofNodes;
    }

    @Override
    public String toString() {

        return "\nMerkleProof {\n"

                + " leafHash   = "

                + leafHash

                + "\n"

                + " rootHash   = "

                + rootHash

                + "\n"

                + " proofSize  = "

                + proofNodes.size()

                + "\n"

                + "}";
    }
}