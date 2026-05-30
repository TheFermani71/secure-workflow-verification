import java.math.BigInteger;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.web3j.abi.FunctionEncoder;
import org.web3j.abi.FunctionReturnDecoder;

import org.web3j.abi.TypeReference;

import org.web3j.abi.datatypes.Bool;
import org.web3j.abi.datatypes.Function;
import org.web3j.abi.datatypes.Type;

import org.web3j.abi.datatypes.Utf8String;

import org.web3j.abi.datatypes.generated.Uint256;

import org.web3j.protocol.Web3j;

import org.web3j.protocol.core.DefaultBlockParameterName;

import org.web3j.protocol.core.methods.request.Transaction;

public class WorkflowVerifierReader {

    private Web3j web3;

    private String contractAddress;

    public WorkflowVerifierReader(

            Web3j web3,

            String contractAddress
    ) {

        this.web3 =
                web3;

        this.contractAddress =
                contractAddress;
    }

    /*
     * getRecordCount()
     */
    public BigInteger getRecordCount()
            throws Exception {

        Function function =
                new Function(

                        "getRecordCount",

                        Collections.emptyList(),

                        Arrays.asList(

                                new TypeReference<Uint256>() {}
                        )
                );

        String encoded =
                FunctionEncoder.encode(
                        function
                );

        String value =
                web3.ethCall(

                                Transaction.createEthCallTransaction(

                                        null,

                                        contractAddress,

                                        encoded
                                ),

                                DefaultBlockParameterName.LATEST
                        )
                        .send()
                        .getValue();

        List<Type> result =
                FunctionReturnDecoder.decode(

                        value,

                        function.getOutputParameters()
                );

        return
                (BigInteger)
                        result.get(0).getValue();
    }

    /*
     * getRecord(index)
     */
    public VerificationRecord getRecord(
            int index
    )
            throws Exception {

        Function function =
                new Function(

                        "getRecord",

                        Arrays.asList(

                                new Uint256(
                                        BigInteger.valueOf(
                                                index
                                        )
                                )
                        ),

                        Arrays.asList(

                                new TypeReference<Utf8String>() {},

                                new TypeReference<Utf8String>() {},

                                new TypeReference<Utf8String>() {},

                                new TypeReference<Utf8String>() {},

                                new TypeReference<Utf8String>() {},

                                new TypeReference<Utf8String>() {},

                                new TypeReference<Uint256>() {},

                                new TypeReference<Bool>() {},

                                new TypeReference<Uint256>() {}
                        )
                );

        String encoded =
                FunctionEncoder.encode(
                        function
                );

        String value =
                web3.ethCall(

                                Transaction.createEthCallTransaction(

                                        null,

                                        contractAddress,

                                        encoded
                                ),

                                DefaultBlockParameterName.LATEST
                        )
                        .send()
                        .getValue();

        List<Type> result =
                FunctionReturnDecoder.decode(

                        value,

                        function.getOutputParameters()
                );

        return new VerificationRecord(

                result.get(0).getValue().toString(),

                result.get(1).getValue().toString(),

                result.get(2).getValue().toString(),

                result.get(3).getValue().toString(),

                result.get(4).getValue().toString(),

                result.get(5).getValue().toString(),

                ((BigInteger)
                        result.get(6).getValue())
                        .longValue(),

                (Boolean)
                        result.get(7).getValue(),

                ((BigInteger)
                        result.get(8).getValue())
                        .longValue()
        );
    }
}