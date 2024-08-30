#include "LKH.h"

void PrepareKicking()
{
    Node *N = FirstNode;
    Candidate *NN;
    int Count = 0;

    do {
        N->Cost = INT_MAX;
        N->Degree = 0;
        for (NN = N->CandidateSet; NN->To; NN++) {
            if (NN->To != N->Pred && NN->To != N->Suc && NN->Cost < N->Cost)
                N->Cost = NN->Cost;
            N->Degree++;
        }
        N->KickRank = ++Count;
        N->KickV = 0;
    } while ((N = N->Suc) != FirstNode);
}
