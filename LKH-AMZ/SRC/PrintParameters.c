#include "LKH.h"

/*
 * The PrintParameters function prints the problem parameters to
 * standard output.
*/

void PrintParameters()
{
    printff("ASCENT_CANDIDATES = %d\n", AscentCandidates);
    printff("CTSP_TRANSFORM = %s\n", CTSPTransform ? "YES" : "NO");
    printff("DEPOT = %d\n", MTSPDepot);
    if (Excess >= 0)
        printff("EXCESS = %g\n", Excess);
    else
        printff("# EXCESS =\n");
    if (InitialPeriod >= 0)
        printff("INITIAL_PERIOD = %d\n", InitialPeriod);
    else
        printff("# INITIAL_PERIOD =\n");
    printff("KICK_TYPE = %d\n", KickType);
    printff("KICKS = %d\n", Kicks);
    printff("MAX_CANDIDATES = %d %s\n",
            MaxCandidates, CandidateSetSymmetric ? "SYMMETRIC" : "");
    if (MaxTrials >= 0)
        printff("MAX_TRIALS = %d\n", MaxTrials);
    else
        printff("# MAX_TRIALS =\n");
    printff("PENALTY_MULTIPLIER = %d\n", PenaltyMultiplier);
    printff("PRECISION = %d\n", Precision);
    printff("%sPROBLEM_FILE = %s\n",
            ProblemFileName ? "" : "# ",
            ProblemFileName ? ProblemFileName : "");
    printff("RUNS = %d\n", Runs);
    printff("SEED = %u\n", Seed);
    printff("SUBGRADIENT = %s\n", Subgradient ? "YES" : "NO");
    if (TimeLimit == DBL_MAX)
        printff("# TIME_LIMIT =\n");
    else
        printff("TIME_LIMIT = %0.1f\n", TimeLimit);
    printff("%sTOUR_FILE = %s\n",
            TourFileName ? "" : "# ", TourFileName ? TourFileName : "");
    printff("TRACE_LEVEL = %d\n", TraceLevel);
}
