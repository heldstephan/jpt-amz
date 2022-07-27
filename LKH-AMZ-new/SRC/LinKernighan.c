#include "Segment.h"
#include "LKH.h"
#include "Hashing.h"

/*
 * The LinKernighan function seeks to improve a tour by sequential
 * and non-sequential edge exchanges.
 *
 * The function returns the cost of the resulting tour.
 */

long long LinKernighan()
{
    long long Cost, Gain, G0;
    int X2, i, it = 0;
    Node *t1, *t2, *SUCt1;
    double EntryTime = GetTime();

    Cost = 0;
    Reversed = 0;

    FirstActive = LastActive = 0;
    Swaps = 0;

    /* Compute the cost of the initial tour, Cost.
       Compute the corresponding hash value, Hash.
       Initialize the segment list.
       Make all nodes "active" (so that they can be used as t1). */

    Cost = 0;
    Hash = 0;
    i = 0;
    t1 = FirstNode;
    do {
        t2 = t1->OldSuc = t1->Suc;
        t1->OldPred = t1->Pred;
        t1->Rank = ++i;
        Cost += C(t1, t2) - t1->Pi - t2->Pi;
        if (HashingUsed)
            Hash ^= Rand[t1->Id] * Rand[t2->Id];
        t1->Next = 0;
        if (KickType == 0 || Trial == 1 ||
            !InBestTour(t1, t1->Pred) || !InBestTour(t1, t1->Suc))
            Activate(t1);
    }
    while ((t1 = t1->Suc) != FirstNode);
    Cost /= Precision;
    CurrentPenalty = LLONG_MAX;
    CurrentPenalty = Penalty();
    if (TraceLevel >= 3 ||
        (TraceLevel == 2 &&
         (CurrentPenalty < BetterPenalty ||
          (CurrentPenalty == BetterPenalty && Cost < BetterCost))))
        StatusReport(Cost, EntryTime, "");

    /* Choose t1 as the first "active" node */
    while ((t1 = RemoveFirstActive())) {
        /* t1 is now "passive" */
        SUCt1 = SUC(t1);
        if ((TraceLevel >= 3 || (TraceLevel == 2 && Trial == 1)) &&
            ++it % (Dimension >= 100000 ? 10000 :
                    Dimension >= 10000 ? 1000 : 100) == 0)
            printff("#%d: Time = %0.2f sec.\n",
                    it, fabs(GetTime() - EntryTime));
        /* Choose t2 as one of t1's two neighbors on the tour */
        for (X2 = 1; X2 <= 2; X2++) {
            t2 = X2 == 1 ? PRED(t1) : SUCt1;
            if (Fixed(t1, t2) ||
                (Near(t1, t2) &&
                 (Trial == 1 || KickType == 0)))
                continue;
            G0 = C(t1, t2);
            Swaps = 0;
            PenaltyGain = Gain = 0;
            /* Try to find a tour-improving move */
            SpecialMove(t1, t2, &G0, &Gain);
            if (PenaltyGain > 0 || Gain > 0) {
                /* An improvement has been found */
                assert(Gain % Precision == 0);
                Cost -= Gain / Precision;
                CurrentPenalty -= PenaltyGain;
                StoreTour();
                if (TraceLevel >= 3 ||
                    (TraceLevel == 2 &&
                     (CurrentPenalty < BetterPenalty ||
                      (CurrentPenalty == BetterPenalty &&
                       Cost < BetterCost))))
                    StatusReport(Cost, EntryTime, "");
                if (HashingUsed && HashSearch(HTable, Hash, Cost))
                    goto End_LinKernighan;
                /* Make t1 "active" again */
                Activate(t1);
                break;
            }
            RestoreTour();
        }
    }
  End_LinKernighan:
    NormalizeNodeList();
    Reversed = 0;
    return Cost;
}
