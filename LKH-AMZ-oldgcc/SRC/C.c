#include "LKH.h"

/* 
 * Functions for computing the transformed distance of an edge (Na,Nb). 
 */

/* 
 * The C_EXPLICIT function returns the distance by looking it up in a table. 
 */

int C_EXPLICIT(Node * Na, Node * Nb)
{
    return Na->Id < Nb->Id ? Nb->C[Na->Id] : Na->C[Nb->Id];
}

int D_EXPLICIT(Node * Na, Node * Nb)
{
    return (Na->Id <
            Nb->Id ? Nb->C[Na->Id] : Na->C[Nb->Id]) + Na->Pi + Nb->Pi;
}

int C_FUNCTION(Node * Na, Node * Nb)
{
    Node *Nc;
    Candidate *Cand;
    int Index, i, j;

    if (CostMatrix)
        return D(Na, Nb);
    if (PredSucCostAvailable) {
        if (Na->Suc == Nb)
            return Na->SucCost;
        if (Na->Pred == Nb)
            return Na->PredCost;
    }
    if ((Cand = Na->CandidateSet))
        for (; (Nc = Cand->To); Cand++)
            if (Nc == Nb)
                return Cand->Cost;
    if ((Cand = Nb->CandidateSet))
        for (; (Nc = Cand->To); Cand++)
            if (Nc == Na)
                return Cand->Cost;
    if (CacheSig == 0)
        return D(Na, Nb);
    i = Na->Id;
    j = Nb->Id;
    if (i > j) {
        int k = i;
        i = j;
        j = k;
    }
    Index = ((i << 8) + i + j) & CacheMask;
    if (CacheSig[Index] == i)
        return CacheVal[Index];
    CacheSig[Index] = i;
    return (CacheVal[Index] = D(Na, Nb));
}

int D_FUNCTION(Node * Na, Node * Nb)
{
    return (Fixed(Na, Nb) ? 0 : Distance(Na, Nb) * Precision) + Na->Pi +
        Nb->Pi;
}
