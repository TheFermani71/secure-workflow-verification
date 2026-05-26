public class TraceIntegrityValidator {

    /*
     * Validate entire hash chain
     */
    public boolean validate(
            ExecutionTrace trace
    ) {

        System.out.println();

        System.out.println(
                "========================================"
        );

        System.out.println(
                " TRACE INTEGRITY VALIDATION"
        );

        System.out.println(
                "========================================"
        );

        String previousHash =
                "GENESIS";

        for (TraceEntry entry :
                trace.entries) {

            /*
             * Check previous hash
             */
            if (
                    !entry.prevHash.equals(
                            previousHash
                    )
            ) {

                System.out.println();

                System.out.println(
                        "[INTEGRITY] INVALID PREVIOUS HASH"
                );

                System.out.println(
                        " seq : "
                                + entry.seq
                );

                return false;
            }

            /*
             * Recompute entry hash
             */
            String payload =
                    HashChainUtils.buildEntryPayload(
                            entry
                    );

            String computedHash =
                    HashChainUtils.sha256(
                            previousHash
                                    + payload
                    );

            /*
             * Compare hashes
             */
            if (
                    !computedHash.equals(
                            entry.entryHash
                    )
            ) {

                System.out.println();

                System.out.println(
                        "[INTEGRITY] HASH MISMATCH"
                );

                System.out.println(
                        " seq : "
                                + entry.seq
                );

                return false;
            }

            System.out.println(
                    "[OK] seq="
                            + entry.seq
            );

            previousHash =
                    computedHash;
        }

        System.out.println();

        System.out.println(
                "[INTEGRITY] VALID"
        );

        return true;
    }
}