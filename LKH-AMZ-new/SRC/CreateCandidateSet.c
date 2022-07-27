#include "LKH.h"

/*
 * The CreateCandidateSet function determines for each node its set of incident
 * candidate edges.
 *
 * The Ascent function is called to determine a lower bound on the optimal tour 
 * using subgradient optimization. But only if the penalties (the Pi-values) is
 * not available on file. In the latter case, the penalties is read from the 
 * file, and the lower bound is computed from a minimum 1-tree.      
 *
 * The function GenerateCandidates is called to compute the Alpha-values and to 
 * associate to each node a set of incident candidate edges.  
 *
 * The CreateCandidateSet function itself is called from LKHmain.
 */

void CreateCandidateSet()
{
    long long Cost, MaxAlpha;
    Node *Na;
    int i;
    double EntryTime = GetTime();

    Norm = 9999;
    if (C == C_EXPLICIT) {
        Na = FirstNode;
        do {
            for (i = 1; i < Na->Id; i++)
                Na->C[i] *= Precision;
        }
        while ((Na = Na->Suc) != FirstNode);
    }
    if (TraceLevel >= 2)
        printff("Creating candidates ...\n");
    Na = FirstNode;
    do
        Na->Pi = 0;
    while ((Na = Na->Suc) != FirstNode);
    Cost = Ascent();
    if (MaxCandidates > 0) {
        if (TraceLevel >= 2)
            printff("Computing lower bound ... ");
        Cost = Minimum1TreeCost(0);
        if (TraceLevel >= 2)
            printff("done\n");
    } else {
        if (TraceLevel >= 2)
            printff("Computing lower bound ... ");
        Cost = Minimum1TreeCost(1);
        if (TraceLevel >= 2)
            printff("done\n");
    }
    LowerBound = (double) Cost / Precision;
    if (TraceLevel >= 1) {
        printff("Lower bound = %0.1f", LowerBound);
        printff(", Ascent time = %0.2f sec.",
                fabs(GetTime() - EntryTime));
        printff("\n");
    }
    MaxAlpha = (long long) fabs(Excess * Cost);
    GenerateCandidates(MaxCandidates, MaxAlpha, CandidateSetSymmetric);

    if (MaxTrials > 0) {
        Na = FirstNode;
        do {
            if (!Na->CandidateSet || !Na->CandidateSet[0].To) {
                if (MaxCandidates == 0)
                    eprintf
                        ("MAX_CANDIDATES = 0: Node %d has no candidates",
                         Na->Id);
                else
                    eprintf("Node %d has no candidates", Na->Id);
            }
        }
        while ((Na = Na->Suc) != FirstNode);
    }
    if (C == C_EXPLICIT) {
        Na = FirstNode;
        do
            for (i = 1; i < Na->Id; i++)
                Na->C[i] += Na->Pi + NodeSet[i].Pi;
        while ((Na = Na->Suc) != FirstNode);
    }
    if (TraceLevel >= 1) {
        CandidateReport();
        printff("Preprocessing time = %0.2f sec.\n",
                fabs(GetTime() - EntryTime));
    }
}
