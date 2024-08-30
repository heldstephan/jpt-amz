#include "LKH.h"

/*
 * Specialized Flip function for asymmetric instances.
 *
 * The Flip function performs a 2-opt move. Edges (t1,t2) and (t3,t4)
 * are exchanged with edges (t2,t3) and (t4,t1). Node t4 is one of
 * t3's two neighbors on the tour; which one is uniquely determined
 * by the orientation of (t1,t2).
 *
 * Since asymmetric instances do not allow flips, this function apply
 * a 2-opt move without changing the orientation of the segments.
 *
 * The move is pushed onto a stack of 2-opt moves. The stack makes it
 * possible to undo moves (by the RestoreTour function).
 */

void Flip(Node *t1, Node *t2, Node *t3)
{
    Node *t4;

    if (t3->FixedTo1 == t3->Pred) {
        t4 = t3->Suc;
        t3->Suc = t2;
    } else {
        t4 = t3->Pred;
        t3->Pred = t2;
    }
    if (t2->FixedTo1 == t2->Pred)
        t2->Suc = t3;
    else
        t2->Pred = t3;
    if (t4->FixedTo1 == t4->Pred)
        t4->Suc = t1;
    else
        t4->Pred = t1;
    if (t1->FixedTo1 == t1->Pred)
        t1->Suc = t4;
    else
        t1->Pred = t4;
    SwapStack[Swaps].t1 = t1;
    SwapStack[Swaps].t2 = t2;
    SwapStack[Swaps].t3 = t3;
    SwapStack[Swaps].t4 = t4;
    Swaps++;
}

void FlipUpdate()
{
    Node *FirstN, *LastN, *N;
    int Rank, i;

    if (Swaps == 0)
        return;
    FirstN = LastN = SwapStack[0].t1;
    for (i = Swaps - 1; i >= 0; i--) {
        Node *t1 = SwapStack[i].t1;
        Node *t2 = SwapStack[i].t2;
        Node *t3 = SwapStack[i].t3;
        Node *t4 = SwapStack[i].t4;
        if (HashingUsed) {
            Hash ^= (Rand[t1->Id] * Rand[t2->Id]) ^
                    (Rand[t3->Id] * Rand[t4->Id]) ^
                    (Rand[t2->Id] * Rand[t3->Id]) ^
                    (Rand[t4->Id] * Rand[t1->Id]);
        }
        if (FirstN != FirstNode) {
            if (t1 == FirstNode ||
                t2 == FirstNode ||
                t3 == FirstNode ||
                t4 == FirstNode)
                FirstN = LastN = FirstNode;
            else {
                if (FirstN->Rank > t1->Rank)
                    FirstN = t1;
                else if (LastN->Rank < t1->Rank)
                    LastN = t1;
                if (FirstN->Rank > t2->Rank)
                    FirstN = t2;
                else if (LastN->Rank < t2->Rank)
                    LastN = t2;
                if (FirstN->Rank > t3->Rank)
                    FirstN = t3;
                else if (LastN->Rank < t3->Rank)
                    LastN = t3;
                if (FirstN->Rank > t4->Rank)
                    FirstN = t4;
                else if (LastN->Rank < t4->Rank)
                    LastN = t4;
            }
        }
    }
    N = FirstN;
    Rank = N->Rank;
    while ((N = N->Suc) != LastN)
        N->Rank = ++Rank;
}
