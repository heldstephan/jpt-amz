#include "Segment.h"
#include "LKH.h"

/* 
 * The StoreTour function is called each time the tour has been improved by 
 * the LinKernighan function.
 *
 * The function "activates" all nodes involved in the current sequence of moves.
 *
 * It sets OldPred to Pred and OldSuc to Suc for each of these nodes. In this
 * way it can always be determined whether an edge belongs to current starting
 * tour. This is used by the BestMove function to determine whether an edge is
 * excludable.
 */

void StoreTour()
{
    Node *t;
    int i;

    while (Swaps > 0) {
        Swaps--;
        for (i = 1; i <= 4; i++) {
            t = i == 1 ? SwapStack[Swaps].t1 :
                i == 2 ? SwapStack[Swaps].t2 :
                i == 3 ? SwapStack[Swaps].t3 : SwapStack[Swaps].t4;
            Activate(t);
            t->OldPred = t->Pred;
            t->OldSuc = t->Suc;
        }
    }
}
