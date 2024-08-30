#include "LKH.h"

/*
 * After the candidate set has been created the FindTour function is called
 * a predetermined number of times (Runs).
 *
 * FindTour performs a number of trials, where in each trial it attempts
 * to improve a chosen initial tour using the modified Lin-Kernighan edge
 * exchange heuristics.
 *
 * Each time a better tour is found, the tour is recorded, and the candidates
 * are reorderded by the AdjustCandidateSet function. Precedence is given to
 * edges that are common to two currently best tours. The candidate set is
 * extended with those tour edges that are not present in the current set.
 * The original candidate set is re-established at exit from FindTour.
 */

long long FindTour()
{

    long long Cost;
    Node *t;
    int i;
    double EntryTime = GetTime();

    t = FirstNode;
    do
        t->OldPred = t->OldSuc = t->NextBestSuc = t->BestSuc = 0;
    while ((t = t->Suc) != FirstNode);
    BetterCost = LLONG_MAX;
    BetterPenalty = CurrentPenalty = LLONG_MAX;
    if (MaxTrials > 0) {
        if (HashingUsed)
            HashInitialize(HTable);
    } else {
        Trial = 1;
        ChooseInitialTour();
        CurrentPenalty = LLONG_MAX;
        CurrentPenalty = BetterPenalty = Penalty();
    }
    PrepareKicking();
    for (Trial = 1; Trial <= MaxTrials; Trial++) {
        if (Trial > 1 && GetTime() - StartTime >= TimeLimit) {
            if (TraceLevel >= 1)
                printff("*** Time limit exceeded ***\n");
            break;
        }
        /* Choose FirstNode at random */
        if (Dimension == DimensionSaved)
            FirstNode = &NodeSet[1 + Random() % Dimension];
        else
            for (i = Random() % Dimension; i > 0; i--)
                FirstNode = FirstNode->Suc;
        ChooseInitialTour();
        Cost = LinKernighan();
        if (CurrentPenalty < BetterPenalty ||
            (CurrentPenalty == BetterPenalty && Cost < BetterCost)) {
            if (TraceLevel >= 1) {
                printff("* %d: ", Trial);
                StatusReport(Cost, EntryTime, "");
            }
            BetterCost = Cost;
            BetterPenalty = CurrentPenalty;
            RecordBetterTour();
            AdjustCandidateSet();
            PrepareKicking();
            if (HashingUsed) {
                HashInitialize(HTable);
                HashInsert(HTable, Hash, Cost);
            }
        } else if (TraceLevel >= 2) {
            printff("  %d: ", Trial);
            StatusReport(Cost, EntryTime, "");
        }
    }
    t = FirstNode;
    if (Norm == 0 || MaxTrials == 0 || !t->BestSuc) {
        do
            t = t->BestSuc = t->Suc;
        while (t != FirstNode);
    }
    do
        (t->Suc = t->BestSuc)->Pred = t;
    while ((t = t->BestSuc) != FirstNode);
    if (HashingUsed) {
        Hash = 0;
        do
            Hash ^= Rand[t->Id] * Rand[t->Suc->Id];
        while ((t = t->BestSuc) != FirstNode);
    }
    if (Trial > MaxTrials)
        Trial = MaxTrials;
    CurrentPenalty = BetterPenalty;
    return BetterCost;
}
