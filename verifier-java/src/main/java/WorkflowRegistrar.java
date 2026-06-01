import java.io.File;
import java.nio.file.Files;
import java.security.MessageDigest;

public class WorkflowRegistrar {

    public static void main(String[] args) {

        try {

            if (args.length != 3) {

                System.out.println();

                System.out.println(
                        "Usage:"
                );

                System.out.println(
                        "WorkflowRegistrar <workflow-file> <name> <version>"
                );

                return;
            }

            String workflowFile =
                    args[0];

            String workflowName =
                    args[1];

            String workflowVersion =
                    args[2];

            String workflowHash =
                    sha256File(
                            workflowFile
                    );

            System.out.println();

            System.out.println(
                    "========================================"
            );

            System.out.println(
                    " WORKFLOW REGISTRATION"
            );

            System.out.println(
                    "========================================"
            );

            System.out.println();

            System.out.println(
                    "File     : "
                            + workflowFile
            );

            System.out.println(
                    "Name     : "
                            + workflowName
            );

            System.out.println(
                    "Version  : "
                            + workflowVersion
            );

            System.out.println(
                    "Hash     : "
                            + workflowHash
            );

            BlockchainConnector connector =

                    new BlockchainConnector(

                            "http://127.0.0.1:8545",

                            "0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80",

                            "0x5FbDB2315678afecb367f032d93F642f64180aa3"
                    );

            connector.registerWorkflow(

                    workflowHash,

                    workflowName,

                    workflowVersion
            );

            connector.close();

        } catch (Exception e) {

            e.printStackTrace();
        }
    }

    private static String sha256File(
            String filename
    )
            throws Exception {

        byte[] data =

                Files.readAllBytes(
                        new File(
                                filename
                        ).toPath()
                );

        MessageDigest digest =

                MessageDigest.getInstance(
                        "SHA-256"
                );

        byte[] hash =
                digest.digest(
                        data
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
    }
}