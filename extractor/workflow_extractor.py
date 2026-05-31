from coroutine_parser import CoroutineParser


class WorkflowExtractor:

    def __init__(
            self,
            source_file,
            mode
    ):

        self.source_file = source_file
        self.mode = mode

        with open(
                source_file,
                "r"
        ) as f:

            self.lines = f.readlines()

    # ==========================================
    # MAIN
    # ==========================================

    def extract_sequence(self):

        if self.mode == "coroutine":

            return self.extract_coroutine_sequence()

        return self.extract_sequential_sequence()

    # ==========================================
    # SEQUENTIAL WORKFLOW
    # ==========================================

    def extract_sequential_sequence(self):

        body = self.extract_function_body(
            "demo_pipeline_avgN"
        )

        sequence = []

        init_count = 0

        for line in body:

            #
            # INIT
            #
            if "init(" in line:

                init_count += 1

                #
                # La trace contiene soltanto
                # i primi due INIT.
                #
                if init_count <= 2:

                    sequence.append(
                        "INIT"
                    )

                continue

            #
            # AVG_N
            #
            if "f_avg_lastN_temp_P3(" in line:

                sequence.append(
                    "AVG_N"
                )

            #
            # CONST
            #
            elif "f_const_f32(" in line:

                sequence.append(
                    "CONST"
                )

            #
            # GT
            #
            elif "f_gt(" in line:

                sequence.append(
                    "GT"
                )

            #
            # WRITE
            #
            elif "f_write_fan_P12(" in line:

                sequence.append(
                    "WRITE"
                )

        print()
        print(
            "[WorkflowExtractor] "
            "Sequential workflow extracted"
        )
        print(
            "Operations :",
            len(sequence)
        )
        print()

        return sequence

    # ==========================================
    # FUNCTION BODY EXTRACTION
    # ==========================================

    def extract_function_body(
            self,
            function_name
    ):

        body = []

        inside = False

        brace_level = 0

        signature = (
            f"static void {function_name}(void)"
        )

        for line in self.lines:

            #
            # Search function
            #
            if not inside:

                if signature in line:

                    inside = True

                else:

                    continue

            #
            # Track braces
            #
            brace_level += line.count("{")

            brace_level -= line.count("}")

            body.append(line)

            #
            # End of function
            #
            if (
                    inside
                    and brace_level == 0
                    and len(body) > 1
            ):

                break

        return body

    # ==========================================
    # COROUTINE
    # ==========================================

    def extract_coroutine_sequence(self):

        parser = CoroutineParser()

        return parser.parse(
            self.lines
        )