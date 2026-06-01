import java.security.MessageDigest;

public class MerkleProofVerifier {

    public static boolean verify(
            MerkleProof proof
    ) {

        String currentHash =
                proof.getLeafHash();

        for (
                MerkleProofNode node :
                proof.getProofNodes()
        ) {

            if (
                    node.isLeftSibling()
            ) {

                currentHash =
                        hashPair(
                                node.getHash(),
                                currentHash
                        );

            } else {

                currentHash =
                        hashPair(
                                currentHash,
                                node.getHash()
                        );
            }
        }

        return
                currentHash.equalsIgnoreCase(
                        proof.getRootHash()
                );
    }

    public static String hashPair(
            String leftHex,
            String rightHex
    ) {

        try {

            byte[] left =
                    hexToBytes(
                            leftHex
                    );

            byte[] right =
                    hexToBytes(
                            rightHex
                    );

            byte[] combined =
                    new byte[
                            left.length
                                    +
                                    right.length
                    ];

            System.arraycopy(
                    left,
                    0,
                    combined,
                    0,
                    left.length
            );

            System.arraycopy(
                    right,
                    0,
                    combined,
                    left.length,
                    right.length
            );

            MessageDigest digest =
                    MessageDigest.getInstance(
                            "SHA-256"
                    );

            byte[] result =
                    digest.digest(
                            combined
                    );

            return bytesToHex(
                    result
            );

        } catch (Exception e) {

            throw new RuntimeException(
                    e
            );
        }
    }

    private static byte[] hexToBytes(
            String hex
    ) {

        int len =
                hex.length();

        byte[] data =
                new byte[len / 2];

        for (
                int i = 0;
                i < len;
                i += 2
        ) {

            int value =

                    (Character.digit(
                            hex.charAt(i),
                            16
                    ) << 4)

                            +

                            Character.digit(
                                    hex.charAt(i + 1),
                                    16
                            );

            data[i / 2] =
                    (byte) value;
        }

        return data;
    }

    private static String bytesToHex(
            byte[] bytes
    ) {

        StringBuilder sb =
                new StringBuilder();

        for (
                byte b :
                bytes
        ) {

            sb.append(

                    String.format(
                            "%02x",
                            b
                    )
            );
        }

        return sb.toString();
    }
}