import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;

public class HashChainUtils {

    /*
     * SHA-256 helper
     */
    public static String sha256(
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
                                    StandardCharsets.UTF_8
                            )
                    );

            StringBuilder hex =
                    new StringBuilder();

            for (byte b : hash) {

                hex.append(
                        String.format(
                                "%02x",
                                b
                        )
                );
            }

            return hex.toString();

        } catch (Exception ex) {

            ex.printStackTrace();

            return "";
        }
    }

    /*
     * Build canonical entry payload
     */
    public static String buildEntryPayload(
            TraceEntry entry
    ) {

        return

                entry.seq
                        + "|"

                        + entry.op
                        + "|"

                        + entry.opName
                        + "|"

                        + entry.deltaUs
                        + "|"

                        + entry.deltaApiUs
                        + "|"

                        + entry.chainTag;
    }
}