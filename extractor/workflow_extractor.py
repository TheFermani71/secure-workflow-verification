from coroutine_parser import CoroutineParser

class WorkflowExtractor:

    def __init__(self, source_file, mode):

        self.source_file = source_file

        self.mode = mode

        with open(source_file, "r") as f:

            self.lines = f.readlines()

    # ==========================================
    # MAIN
    # ==========================================

    def extract_sequence(self):

        if self.mode == "coroutine":

            return self.extract_coroutine_sequence()

        return self.extract_sequential_sequence()

    # ==========================================
    # SEQUENTIAL
    # ==========================================

    def extract_sequential_sequence(self):

        sequence = []

        for line in self.lines:

            api = self.extract_api(line)

            if api:

                sequence.append(api)

        return sequence

    # ==========================================
    # COROUTINE
    # ==========================================

    def extract_coroutine_sequence(self):

        parser = CoroutineParser()

        return parser.parse(self.lines)

    # ==========================================
    # API DETECTION
    # ==========================================

    def extract_api(self, line):

        sv_apis = [

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

        upper = line.upper()

        for api in sv_apis:

            if api in upper:

                return api

        return None