import java.math.BigInteger;
import java.util.Arrays;
import java.util.Collections;

import org.web3j.abi.FunctionEncoder;
import org.web3j.abi.FunctionReturnDecoder;

import org.web3j.abi.TypeReference;

import org.web3j.abi.datatypes.Bool;
import org.web3j.abi.datatypes.Function;
import org.web3j.abi.datatypes.Type;
import org.web3j.abi.datatypes.Utf8String;
import org.web3j.abi.datatypes.generated.Uint256;

import org.web3j.crypto.Credentials;

import org.web3j.protocol.Web3j;
import org.web3j.protocol.core.DefaultBlockParameterName;
import org.web3j.protocol.core.methods.request.Transaction;

import org.web3j.protocol.http.HttpService;

import org.web3j.protocol.core.methods.response.EthCall;
import org.web3j.protocol.core.methods.response.EthGetTransactionReceipt;
import org.web3j.protocol.core.methods.response.EthSendTransaction;
import org.web3j.protocol.core.methods.response.TransactionReceipt;

import org.web3j.tx.RawTransactionManager;
import org.web3j.tx.gas.DefaultGasProvider;

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
                        new HttpService(
                                rpcUrl
                        )
                );

        this.credentials =
                Credentials.create(
                        privateKey
                );

        this.contractAddress =
                contractAddress;
    }

    // ============================================================
    // WORKFLOW REGISTRY
    // ============================================================

    public void registerWorkflow(

            String workflowHash,

            String workflowName,

            String workflowVersion
    ) {

        try {

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

            Function function =
                    new Function(

                            "registerWorkflow",

                            Arrays.asList(

                                    new Utf8String(
                                            workflowHash
                                    ),

                                    new Utf8String(
                                            workflowName
                                    ),

                                    new Utf8String(
                                            workflowVersion
                                    )
                            ),

                            Collections.emptyList()
                    );

            String encodedFunction =
                    FunctionEncoder.encode(
                            function
                    );

            RawTransactionManager txManager =
                    new RawTransactionManager(
                            web3,
                            credentials
                    );

            EthSendTransaction txResponse =
                    txManager.sendTransaction(

                            DefaultGasProvider.GAS_PRICE,

                            BigInteger.valueOf(
                                    1000000
                            ),

                            contractAddress,

                            encodedFunction,

                            BigInteger.ZERO
                    );

            if (txResponse.hasError()) {

                System.out.println();

                System.out.println(
                        "[BLOCKCHAIN ERROR]"
                );

                System.out.println(
                        txResponse.getError()
                                .getMessage()
                );

                return;
            }

            System.out.println();

            System.out.println(
                    "Workflow registered"
            );

            System.out.println(
                    "TX HASH: "
                            + txResponse.getTransactionHash()
            );

        } catch (Exception e) {

            e.printStackTrace();
        }
    }

    public boolean isWorkflowRegistered(

            String workflowHash
    ) {

        try {

            Function function =
                    new Function(

                            "isWorkflowRegistered",

                            Arrays.asList(

                                    new Utf8String(
                                            workflowHash
                                    )
                            ),

                            Arrays.asList(

                                    new TypeReference<Bool>() {
                                    }
                            )
                    );

            String encodedFunction =
                    FunctionEncoder.encode(
                            function
                    );

            EthCall response =
                    web3.ethCall(

                                    Transaction.createEthCallTransaction(

                                            credentials.getAddress(),

                                            contractAddress,

                                            encodedFunction
                                    ),

                                    DefaultBlockParameterName.LATEST
                            )
                            .send();

            java.util.List<Type> result =

                    FunctionReturnDecoder.decode(

                            response.getValue(),

                            function.getOutputParameters()
                    );

            if (result.isEmpty()) {

                return false;
            }

            return
                    ((Bool) result.get(0))
                            .getValue();

        } catch (Exception e) {

            e.printStackTrace();

            return false;
        }
    }

    // ============================================================
    // VERIFICATION REGISTRY
    // ============================================================

    public void registerVerification(
            VerificationResult result
    ) {

        try {

            System.out.println();

            System.out.println(
                    "========================================"
            );

            System.out.println(
                    " BLOCKCHAIN ATTESTATION"
            );

            System.out.println(
                    "========================================"
            );

            BigInteger blockNumber =
                    web3.ethBlockNumber()
                            .send()
                            .getBlockNumber();

            System.out.println();

            System.out.println(
                    "Current block : "
                            + blockNumber
            );

            Function function =
                    new Function(

                            "storeVerification",

                            Arrays.asList(

                                    new Utf8String(
                                            result.deviceId
                                    ),

                                    new Utf8String(
                                            result.workflowMode
                                    ),

                                    new Utf8String(
                                            result.verifierMode
                                    ),

                                    new Utf8String(
                                            result.workflowHash
                                    ),

                                    new Utf8String(
                                            result.traceHash
                                    ),

                                    new Utf8String(
                                            result.traceMerkleRoot
                                    ),

                                    new Uint256(
                                            BigInteger.valueOf(
                                                    result.traceLength
                                            )
                                    ),

                                    new Bool(
                                            result.valid
                                    )
                            ),

                            Collections.emptyList()
                    );

            String encodedFunction =
                    FunctionEncoder.encode(
                            function
                    );

            RawTransactionManager txManager =
                    new RawTransactionManager(
                            web3,
                            credentials
                    );

            System.out.println();

            System.out.println(
                    "Sending transaction..."
            );

            EthSendTransaction txResponse =
                    txManager.sendTransaction(

                            DefaultGasProvider.GAS_PRICE,

                            BigInteger.valueOf(
                                    1000000
                            ),

                            contractAddress,

                            encodedFunction,

                            BigInteger.ZERO
                    );

            if (txResponse.hasError()) {

                System.out.println();

                System.out.println(
                        "[BLOCKCHAIN ERROR]"
                );

                System.out.println(
                        txResponse.getError()
                                .getMessage()
                );

                return;
            }

            String txHash =
                    txResponse.getTransactionHash();

            System.out.println();

            System.out.println(
                    "TX HASH:"
            );

            System.out.println(
                    txHash
            );

            TransactionReceipt receipt =
                    null;

            for (int i = 0; i < 20; i++) {

                Thread.sleep(
                        1000
                );

                EthGetTransactionReceipt receiptResponse =
                        web3.ethGetTransactionReceipt(
                                        txHash
                                )
                                .send();

                if (
                        receiptResponse
                                .getTransactionReceipt()
                                .isPresent()
                ) {

                    receipt =
                            receiptResponse
                                    .getTransactionReceipt()
                                    .get();

                    break;
                }
            }

            if (receipt != null) {

                System.out.println();

                System.out.println(
                        "Transaction mined"
                );

                System.out.println(
                        "Block      : "
                                + receipt.getBlockNumber()
                );

                System.out.println(
                        "Gas Used   : "
                                + receipt.getGasUsed()
                );

                System.out.println(
                        "Status     : "
                                + receipt.getStatus()
                );

            } else {

                System.out.println();

                System.out.println(
                        "Receipt timeout"
                );
            }

        } catch (Exception e) {

            e.printStackTrace();
        }
    }

    // ============================================================
    // CLEANUP
    // ============================================================

    public void close() {

        try {

            if (web3 != null) {

                web3.shutdown();
            }

        } catch (Exception e) {

            e.printStackTrace();
        }
    }
}