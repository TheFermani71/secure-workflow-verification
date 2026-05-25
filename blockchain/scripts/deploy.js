const hre = require("hardhat");

async function main() {

    console.log("\n========================================");
    console.log(" DEPLOY WORKFLOW VERIFIER CONTRACT");
    console.log("========================================\n");

    const WorkflowVerifier =
        await hre.ethers.getContractFactory(
            "WorkflowVerifier"
        );

    const verifier =
        await WorkflowVerifier.deploy();

    await verifier.deployed();

    console.log("Contract deployed successfully");

    console.log("\nContract address:");
    console.log(verifier.address);

    console.log("\n========================================");
}

main()
.then(() => process.exit(0))
.catch((error) => {

    console.error(error);

    process.exit(1);
});