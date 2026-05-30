// SPDX-License-Identifier: MIT

pragma solidity ^0.8.19;

contract WorkflowVerifier {

    struct VerificationRecord {

        string deviceId;

        string workflowMode;

        string verifierMode;

        string workflowHash;

        string traceHash;

        string traceMerkleRoot;

        uint256 traceLength;

        bool valid;

        uint256 timestamp;
    }

    /*
     * Verification storage
     */
    VerificationRecord[] public records;

    /*
     * Replay protection
     */
    mapping(string => bool) public registeredTraceHashes;

    /*
     * Consistency protection
     */
    mapping(string => string) public traceHashToMerkleRoot;

    /*
     * Event
     */
    event VerificationStored(

        string deviceId,

        string workflowMode,

        string verifierMode,

        string workflowHash,

        string traceHash,

        string traceMerkleRoot,

        uint256 traceLength,

        bool valid,

        uint256 timestamp
    );

    /*
     * Store verification
     */
    function storeVerification(

        string memory deviceId,

        string memory workflowMode,

        string memory verifierMode,

        string memory workflowHash,

        string memory traceHash,

        string memory traceMerkleRoot,

        uint256 traceLength,

        bool valid
    )

        public
    {

        require(

            bytes(deviceId).length > 0,

            "Invalid deviceId"
        );

        require(

            bytes(workflowMode).length > 0,

            "Invalid workflowMode"
        );

        require(

            bytes(verifierMode).length > 0,

            "Invalid verifierMode"
        );

        require(

            bytes(traceMerkleRoot).length > 0,

            "Invalid Merkle Root"
        );

        require(

            traceLength > 0,

            "Invalid trace length"
        );

        /*
         * Replay protection
         */
        require(

            !registeredTraceHashes[traceHash],

            "Trace already registered"
        );

        /*
         * Consistency protection
         */
        traceHashToMerkleRoot[traceHash] =

            traceMerkleRoot;

        registeredTraceHashes[traceHash] = true;

        VerificationRecord memory record =

            VerificationRecord(

                deviceId,

                workflowMode,

                verifierMode,

                workflowHash,

                traceHash,

                traceMerkleRoot,

                traceLength,

                valid,

                block.timestamp
            );

        records.push(record);

        emit VerificationStored(

            deviceId,

            workflowMode,

            verifierMode,

            workflowHash,

            traceHash,

            traceMerkleRoot,

            traceLength,

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

            string memory,

            string memory,

            uint256,

            bool,

            uint256
        )
    {

        VerificationRecord memory r =

            records[index];

        return (

            r.deviceId,

            r.workflowMode,

            r.verifierMode,

            r.workflowHash,

            r.traceHash,

            r.traceMerkleRoot,

            r.traceLength,

            r.valid,

            r.timestamp
        );
    }

    /*
     * Check if trace hash exists
     */
    function isTraceRegistered(

        string memory traceHash
    )

        public

        view

        returns (bool)
    {

        return

            registeredTraceHashes[traceHash];
    }

    /*
     * Get Merkle Root associated
     */
    function getMerkleRoot(

        string memory traceHash
    )

        public

        view

        returns (string memory)
    {

        return

            traceHashToMerkleRoot[traceHash];
    }
}