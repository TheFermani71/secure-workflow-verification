const { ethers } = require("hardhat");

async function main() {

    const CONTRACT_ADDRESS =
        "0x5FC8d32690cc91D4c39d9d3abcBD16989F875707";

    const contract =
        await ethers.getContractAt(
            "WorkflowVerifier",
            CONTRACT_ADDRESS
        );

    console.log();
    console.log("========================================");
    console.log(" MERKLE VERIFICATION TEST");
    console.log("========================================");
    console.log();

    /*
     * Build simple Merkle tree:
     *
     * leafA ----\
     *            > root
     * leafB ----/
     */

    const leafA =
        ethers.utils.sha256(
            ethers.utils.toUtf8Bytes("A")
        );

    const leafB =
        ethers.utils.sha256(
            ethers.utils.toUtf8Bytes("B")
        );

    const root =
        ethers.utils.sha256(
            ethers.utils.concat([
                leafA,
                leafB
            ])
        );

    const proof = [
        leafB
    ];

    const directions = [
        false
    ];

    const valid =
        await contract.callStatic.verifyMerkleProof(
            leafA,
            proof,
            directions,
            root
        );

    console.log(
        "Valid proof:",
        valid
    );

    const invalid =
        await contract.callStatic.verifyMerkleProof(
            leafA,
            [
                ethers.constants.HashZero
            ],
            directions,
            root
        );

    console.log(
        "Invalid proof:",
        invalid
    );

    console.log();
}

main()
    .then(() => process.exit(0))
    .catch((err) => {
        console.error(err);
        process.exit(1);
    });