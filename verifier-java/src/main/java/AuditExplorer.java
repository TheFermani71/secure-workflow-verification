import java.math.BigInteger;

import org.web3j.crypto.Credentials;

import org.web3j.protocol.Web3j;
import org.web3j.protocol.http.HttpService;

public class AuditExplorer {

    private Web3j web3;

    private Credentials credentials;

    private String contractAddress;

    public AuditExplorer() {

        this.web3 =
                Web3j.build(
                        new HttpService(
                                "http://127.0.0.1:8545"
                        )
                );

        this.credentials =
                Credentials.create(
                        "0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80"
                );

        this.contractAddress =
                "0x5FbDB2315678afecb367f032d93F642f64180aa3";
    }

    public void printAuditReport() {

        try {

            System.out.println();

            System.out.println(
                    "========================================"
            );

            System.out.println(
                    " BLOCKCHAIN AUDIT REPORT"
            );

            System.out.println(
                    "========================================"
            );

            WorkflowVerifierReader reader =
                    new WorkflowVerifierReader(
                            web3,
                            contractAddress
                    );

            BigInteger count =
                    reader.getRecordCount();

            System.out.println();

            System.out.println(
                    "Records stored : "
                            + count
            );

            System.out.println();

            for (int i = 0; i < count.intValue(); i++) {

                VerificationRecord record =
                        reader.getRecord(i);

                System.out.println(
                        "----------------------------------------"
                );

                System.out.println(
                        "Record #" + i
                );

                System.out.println(
                        record
                );

                System.out.println();
            }

        } catch (Exception e) {

            e.printStackTrace();
        }
    }

    public static void main(
            String[] args
    ) {

        AuditExplorer explorer =
                new AuditExplorer();

        explorer.printAuditReport();
    }
}