class CoroutineParser:

    def __init__(self):

        self.coroutine_macros = [

            "COROUTINE_LOOP",
            "COROUTINE_DELAY",
            "COROUTINE_YIELD"

        ]

    # ==========================================
    # MAIN
    # ==========================================

    def parse(self, lines):

        sequence = []

        coroutine_names = [

            "acqGpsTask",
            "processingTask",
            "exportTask"

        ]

        for coroutine_name in coroutine_names:

            body = self.extract_coroutine_body(
                lines,
                coroutine_name
            )

            sequence.extend(
                self.extract_operations(
                    body
                )
            )

        print()
        print(
            "[CoroutineParser] "
            "Coroutine workflow extracted"
        )
        print(
            "Operations :",
            len(sequence)
        )
        print(
            "Sequence :",
            sequence
        )
        print()

        return sequence

    # ==========================================
    # BODY EXTRACTION
    # ==========================================

    def extract_coroutine_body(
            self,
            lines,
            coroutine_name
    ):

        body = []

        inside = False

        brace_level = 0

        signature = (
            f"COROUTINE({coroutine_name})"
        )

        for line in lines:

            if not inside:

                if signature in line:

                    inside = True

                else:

                    continue

            brace_level += line.count("{")

            brace_level -= line.count("}")

            body.append(line)

            if (
                    inside
                    and brace_level == 0
                    and len(body) > 1
            ):

                break

        return body

    # ==========================================
    # OPERATION EXTRACTION
    # ==========================================

    def extract_operations(
            self,
            body
    ):

        sequence = []

        for line in body:

            upper = line.upper()

            #
            # Coroutine macros
            #
            if "COROUTINE_LOOP" in upper:

                sequence.append(
                    "COROUTINE_LOOP"
                )

            elif "COROUTINE_DELAY" in upper:

                sequence.append(
                    "COROUTINE_DELAY"
                )

            elif "COROUTINE_YIELD" in upper:

                sequence.append(
                    "COROUTINE_YIELD"
                )

            #
            # Application operations
            #
            elif "F_POLL_BUTTON_P4(" in upper:

                sequence.append(
                    "READ"
                )

            elif "F_AVG_LASTN_TEMP_P3(" in upper:

                sequence.append(
                    "AVG_N"
                )

            elif "F_CONST_F32(" in upper:

                sequence.append(
                    "CONST"
                )

            elif "F_GT(" in upper:

                sequence.append(
                    "GT"
                )

            elif "F_WRITE_FAN_P12(" in upper:

                sequence.append(
                    "WRITE"
                )

        return sequence