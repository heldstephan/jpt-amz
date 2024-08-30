#include "LKH.h"

/*
 * The CandidateReport function prints the minimum, average and maximum
 * number of candidates associated with a node.
 */

void CandidateReport()
{
    int Min = INT_MAX, Max = 0, Fixed = 0, Count;
    long long Sum = 0, Cost = 0;
    Node *N;
    Candidate *NN;

    N = FirstNode;
    do {
        Count = 0;
        if (N->CandidateSet)
            for (NN = N->CandidateSet; NN->To; NN++)
                Count++;
        if (Count > Max)
            Max = Count;
        if (Count < Min)
            Min = Count;
        Sum += Count;
        if (N->FixedTo1 && N->Id < N->FixedTo1->Id) {
            Fixed++;
            Cost += Distance(N, N->FixedTo1);
        }
        if (N->FixedTo2 && N->Id < N->FixedTo2->Id) {
            Fixed++;
            Cost += Distance(N, N->FixedTo2);
        }
    }
    while ((N = N->Suc) != FirstNode);
    printff("Cand.min = %d, Cand.avg = %0.1f, Cand.max = %d\n",
            Min, (double) Sum / Dimension, Max);
    if (Fixed > 0)
        printff("Edges.fixed = %d [Cost = %lld]\n", Fixed, Cost);
}
