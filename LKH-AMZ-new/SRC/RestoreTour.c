#include "Segment.h"
#include "LKH.h"

/*
 * The RestoreTour function is used to undo a series of moves. The function 
 * restores the tour from SwapStack, the stack of 2-opt moves. A bad sequence 
 * of moves is undone by unstacking the 2-opt moves and making the inverse 
 * 2-opt moves in this reversed sequence.
 */

void RestoreTour()
{
    Node *t1, *t2, *t3;

    /* Loop as long as the stack is not empty */
    while (Swaps > 0) {
        /* Undo topmost 2-opt move */
        Swaps--;
        t1 = SwapStack[Swaps].t1;
        t2 = SwapStack[Swaps].t2;
        t3 = SwapStack[Swaps].t3;
        Swap1(t3, t2, t1);
        Swaps--;
    }
}
