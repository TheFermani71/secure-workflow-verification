import java.math.BigInteger;

import org.web3j.protocol.Web3j;

import org.web3j.protocol.http.HttpService;

import org.web3j.crypto.Credentials;

import org.web3j.tx.RawTransactionManager;

import org.web3j.tx.gas.DefaultGasProvider;

import org.web3j.protocol.core.methods.response.TransactionReceipt;

public class BlockchainConnector {

    private Web3j web3;

    private Credentials credentials;

    private String contractAddress;

    public BlockchainConnector(
        String rpcUrl,
        String privateKey,
        String contractAddress
    ) {

        this.web3 =
            Web3j.build(
                new HttpService(rpcUrl)
            );

        this.credentials =
            Credentials.create(privateKey);

        this.contractAddress =
            contractAddress;
    }

    public void registerVerification(

        String workflowHash,
        String traceHash,
        boolean valid

    ) {

        try {

            System.out.println(
                "\n========================================"
            );

            System.out.println(
                " BLOCKCHAIN ATTESTATION"
            );

            System.out.println(
                "========================================\n"
            );

            System.out.println(
                "Connecting to Ethereum node..."
            );

            BigInteger blockNumber =
                web3.ethBlockNumber()
                    .send()
                    .getBlockNumber();

            System.out.println(
                "Current block: " + blockNumber
            );

            System.out.println(
                "\nVerification data:"
            );

            System.out.println(
                "workflowHash = " + workflowHash
            );

            System.out.println(
                "traceHash    = " + traceHash
            );

            System.out.println(
                "valid        = " + valid
            );

            System.out.println(
                "\n[SIMULATION]"
            );

            System.out.println(
                "Transaction prepared for contract:"
            );

            System.out.println(
                contractAddress
            );

            System.out.println(
                "\nAttestation completed."
            );

        } catch (Exception e) {

            e.printStackTrace();
        }
    }
}