# ============================================
# COMPLEXITY MODEL
#
# Future:
# - gap plausibility
# - behavioral overhead
# - branch estimation
# ============================================

def build_complexity(
        branches=0,
        loops=0,
        coroutines=0
):

    return {
        "branches": branches,
        "loops": loops,
        "coroutines": coroutines
    }