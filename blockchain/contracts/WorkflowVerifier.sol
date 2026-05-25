// SPDX-License-Identifier: MIT

pragma solidity ^0.8.19;

contract WorkflowVerifier {

    struct VerificationRecord {

        string deviceId;

        string workflowMode;

        string workflowHash;

        string traceHash;

        bool valid;

        uint256 timestamp;
    }

    /*
     * Verification storage
     */
    VerificationRecord[] public records;

    /*
     * Event
     */
    event VerificationStored(

        string deviceId,

        string workflowMode,

        string workflowHash,

        string traceHash,

        bool valid,

        uint256 timestamp
    );

    /*
     * Store verification
     */
    function storeVerification(

        string memory deviceId,

        string memory workflowMode,

        string memory workflowHash,

        string memory traceHash,

        bool valid
    )

        public
    {

        VerificationRecord memory record =

            VerificationRecord(

                deviceId,

                workflowMode,

                workflowHash,

                traceHash,

                valid,

                block.timestamp
            );

        records.push(record);

        emit VerificationStored(

            deviceId,

            workflowMode,

            workflowHash,

            traceHash,

            valid,

            block.timestamp
        );
    }

    /*
     * Number of records
     */
    function getRecordCount()

        public

        view

        returns (uint256)
    {

        return records.length;
    }

    /*
     * Read verification
     */
    function getRecord(

        uint256 index
    )

        public

        view

        returns (

            string memory,

            string memory,

            string memory,

            string memory,

            bool,

            uint256
        )
    {

        VerificationRecord memory r =

            records[index];

        return (

            r.deviceId,

            r.workflowMode,

            r.workflowHash,

            r.traceHash,

            r.valid,

            r.timestamp
        );
    }
}