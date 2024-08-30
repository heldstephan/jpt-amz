#include "LKH.h"

/*
 * The AddTourCandidates function extends the candidate set with 
 * fixed edges.
 *   
 * The function is called from GenerateCandidateSet.  
*/

void AddTourCandidates()
{
    /* Add fixed edges */
    Node *Na = FirstNode;
    do {
        if (Na->FixedTo1)
            AddCandidate(Na, Na->FixedTo1, D(Na, Na->FixedTo1), 0);
        if (Na->FixedTo2)
            AddCandidate(Na, Na->FixedTo2, D(Na, Na->FixedTo2), 0);
    }
    while ((Na = Na->Suc) != FirstNode);
}

