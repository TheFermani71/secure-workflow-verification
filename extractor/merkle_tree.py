import hashlib
import json


class MerkleTree:

    # ==========================================
    # SHA256
    # ==========================================

    def sha256(self, data):

        return hashlib.sha256(
            data.encode()
        ).hexdigest()

    # ==========================================
    # ENTRY HASH
    # ==========================================

    def hash_entry(self, entry):

        serialized = json.dumps(

            entry,

            sort_keys=True

        )

        return self.sha256(
            serialized
        )

    # ==========================================
    # BUILD ROOT
    # ==========================================

    def build_root(self, entries):

        if len(entries) == 0:

            return self.sha256(
                "EMPTY_TRACE"
            )

        current_level = []

        for entry in entries:

            current_level.append(

                self.hash_entry(
                    entry
                )
            )

        while len(current_level) > 1:

            next_level = []

            for i in range(

                0,

                len(current_level),

                2
            ):

                left = current_level[i]

                if i + 1 < len(current_level):

                    right = current_level[i + 1]

                else:

                    right = left

                parent = self.sha256(

                    left + right
                )

                next_level.append(
                    parent
                )

            current_level = next_level

        return current_level[0]