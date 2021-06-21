#include "LKH.h"

/* 
 * The FixedCandidates function returns the number of fixed
 * candidate edges emanating from a given node, N.
 */

int FixedCandidates(Node * N)
{
    int Count = 0;

    Count = N->FixedTo2 ? 2 : N->FixedTo1 ? 1 : 0;
    return Count;
}
