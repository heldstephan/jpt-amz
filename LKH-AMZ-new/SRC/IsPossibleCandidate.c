#include "LKH.h"

/* 
 * The IsPossibleCandidate function is used to test if an edge, (From,To),
 * may be a solution edge together with all fixed or common edges.
 *
 * If the edge is possible, the function returns 1; otherwise 0.
 */

int IsPossibleCandidate(Node * From, Node * To)
{
    if (Forbidden(From, To))
        return 0;
    if (Fixed(From, To))
        return 1;
    if (From->FixedTo2 || To->FixedTo2)
        return 0;
    if (!IsCandidate(From, To) &&
        (FixedCandidates(From) == 2 ||
         FixedCandidates(To) == 2))
        return 0;
    return 1;
}
