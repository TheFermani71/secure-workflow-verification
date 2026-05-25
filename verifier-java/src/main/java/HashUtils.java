import java.io.FileInputStream;
import java.security.MessageDigest;

public class HashUtils {

    /*
     * SHA-256 from string
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
                            input.getBytes("UTF-8")
                    );

            return bytesToHex(hash);

        } catch (Exception ex) {

            ex.printStackTrace();
        }

        return "";
    }

    /*
     * SHA-256 from file
     */
    public static String sha256File(
            String path
    ) {

        try {

            MessageDigest digest =
                    MessageDigest.getInstance(
                            "SHA-256"
                    );

            FileInputStream fis =
                    new FileInputStream(path);

            byte[] buffer =
                    new byte[1024];

            int read;

            while ((read = fis.read(buffer)) != -1) {

                digest.update(
                        buffer,
                        0,
                        read
                );
            }

            fis.close();

            byte[] hash =
                    digest.digest();

            return bytesToHex(hash);

        } catch (Exception ex) {

            ex.printStackTrace();
        }

        return "";
    }

    /*
     * Bytes -> hex
     */
    private static String bytesToHex(
            byte[] bytes
    ) {

        StringBuilder sb =
                new StringBuilder();

        for (byte b : bytes) {

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