# ============================================
# COROUTINE PARSER
#
# AceRoutine hooks
#
# Coroutine-aware extraction
# ============================================

COROUTINE_MACROS = [

    "COROUTINE_BEGIN",

    "COROUTINE_YIELD",

    "COROUTINE_DELAY",

    "COROUTINE_LOOP",

    "COROUTINE_END"
]


def is_coroutine_macro(line):

    for macro in COROUTINE_MACROS:

        if macro in line:
            return True

    return False


def extract_coroutine_macro(line):

    for macro in COROUTINE_MACROS:

        if macro in line:
            return macro

    return None