// SPDX-License-Identifier: MIT

pragma solidity ^0.8.19;

contract WorkflowVerifier {

    // ============================================================
    // WORKFLOW REGISTRY
    // ============================================================

    struct Workflow {

        string workflowHash;

        string workflowName;

        string workflowVersion;

        uint256 registeredAt;

        bool active;
    }

    Workflow[] public workflows;

    mapping(string => bool)
        public registeredWorkflows;

    mapping(string => Workflow)
        private workflowRegistry;

    event WorkflowRegistered(

        string workflowHash,

        string workflowName,

        string workflowVersion,

        uint256 timestamp
    );

    // ============================================================
    // VERIFICATION REGISTRY
    // ============================================================

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
    mapping(string => bool)
        public registeredTraceHashes;

    /*
     * Consistency protection
     */
    mapping(string => string)
        public traceHashToMerkleRoot;

    /*
     * Verification event
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

    // ============================================================
    // WORKFLOW REGISTRATION
    // ============================================================

    function registerWorkflow(

        string memory workflowHash,

        string memory workflowName,

        string memory workflowVersion
    )

        public
    {

        require(

            bytes(workflowHash).length > 0,

            "Invalid workflow hash"
        );

        require(

            bytes(workflowName).length > 0,

            "Invalid workflow name"
        );

        require(

            bytes(workflowVersion).length > 0,

            "Invalid workflow version"
        );

        require(

            !registeredWorkflows[workflowHash],

            "Workflow already registered"
        );

        Workflow memory wf =

            Workflow(

                workflowHash,

                workflowName,

                workflowVersion,

                block.timestamp,

                true
            );

        workflows.push(wf);

        workflowRegistry[workflowHash] = wf;

        registeredWorkflows[workflowHash] = true;

        emit WorkflowRegistered(

            workflowHash,

            workflowName,

            workflowVersion,

            block.timestamp
        );
    }

    /*
     * Check workflow registration
     */
    function isWorkflowRegistered(

        string memory workflowHash
    )

        public

        view

        returns (bool)
    {

        return

            registeredWorkflows[workflowHash];
    }

    /*
     * Read workflow
     */
    function getWorkflow(

        string memory workflowHash
    )

        public

        view

        returns (

            string memory,

            string memory,

            string memory,

            uint256,

            bool
        )
    {

        Workflow memory wf =

            workflowRegistry[workflowHash];

        return (

            wf.workflowHash,

            wf.workflowName,

            wf.workflowVersion,

            wf.registeredAt,

            wf.active
        );
    }

    /*
     * Number of registered workflows
     */
    function getWorkflowCount()

        public

        view

        returns (uint256)
    {

        return workflows.length;
    }

    // ============================================================
    // VERIFICATION STORAGE
    // ============================================================

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
         * Workflow Registry enforcement
         */
        require(

            registeredWorkflows[workflowHash],

            "Workflow not registered"
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

    // ============================================================
    // AUDIT API
    // ============================================================

    function getRecordCount()

        public

        view

        returns (uint256)
    {

        return records.length;
    }

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

    // ============================================================
    // MERKLE AUDIT LAYER
    // ============================================================

    event MerkleProofVerified(

        bytes32 leaf,

        bytes32 root,

        bool valid
    );

    function verifyMerkleProof(

        bytes32 leaf,

        bytes32[] memory proof,

        bool[] memory directions,

        bytes32 root
    )

        public

        returns (bool)
    {

        require(

            proof.length == directions.length,

            "Invalid proof"
        );

        bytes32 current = leaf;

        for (

            uint256 i = 0;

            i < proof.length;

            i++
        ) {

            if (directions[i]) {

                current = sha256(

                    abi.encodePacked(

                        proof[i],

                        current
                    )
                );

            } else {

                current = sha256(

                    abi.encodePacked(

                        current,

                        proof[i]
                    )
                );
            }
        }

        bool valid =

            current == root;

        emit MerkleProofVerified(

            leaf,

            root,

            valid
        );

        return valid;
    }

    function verifyTraceMembership(

        string memory traceHash,

        bytes32 leaf,

        bytes32[] memory proof,

        bool[] memory directions,

        bytes32 root
    )

        public

        returns (bool)
    {

        require(

            registeredTraceHashes[traceHash],

            "Trace not registered"
        );

        return

            verifyMerkleProof(

                leaf,

                proof,

                directions,

                root
            );
    }

}