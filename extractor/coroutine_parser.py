class CoroutineParser:

    def __init__(self):

        self.coroutine_macros = [

            "COROUTINE_BEGIN",
            "COROUTINE_YIELD",
            "COROUTINE_DELAY",
            "COROUTINE_END"

        ]

        self.sv_apis = [

            "INIT",
            "ASSIGN",
            "CONST",
            "READ",
            "AVG_N",
            "ADD",
            "SUB",
            "MUL",
            "DIV",
            "GT",
            "LT",
            "EQ",
            "WRITE"

        ]

    # ==========================================
    # MAIN PARSER
    # ==========================================

    def parse(self, lines):

        sequence = []

        for line in lines:

            upper = line.upper()

            # ----------------------------------
            # Coroutine macros
            # ----------------------------------

            for macro in self.coroutine_macros:

                if macro in upper:

                    sequence.append(macro)

            # ----------------------------------
            # Secure APIs
            # ----------------------------------

            for api in self.sv_apis:

                if api in upper:

                    sequence.append(api)

        return sequence