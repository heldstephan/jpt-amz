#include "Segment.h"
#include "LKH.h"

/*
 * The SpecialMove function makes sequential as well as non-sequential
 * edge exchanges. If possible, it makes a sequential 3-opt move,
 * or a non-sequential 4-move, that improves the tour.
 *
 * The function is called from the LinKernighan function.
 */

static void SelectForward(Node ** a, Node ** b, Node * from, Node * to);
static void SelectBackward(Node ** a, Node ** b, Node * from, Node * to);

void SpecialMove(Node * t1, Node * t2, long long * G0, long long * Gain)
{
    Node *t3, *t4, *t5 = 0, *t6 = 0, *t7 = 0, *t8 = 0, *t6Old = 0, *t7Old = 0;
    Candidate *Nt2, *Nt4;
    long long G1, G2, G3, G4;
    int Case56, Case78;

    if (t2 != SUC(t1))
        Reversed ^= 1;

    /* Choose (t2,t3) as a candidate edge emanating from t2 */
    for (Nt2 = t2->CandidateSet; (t3 = Nt2->To); Nt2++) {
        if (t3 == t2->Pred || t3 == t2->Suc ||
            (G1 = *G0 - Nt2->Cost) <= 0)
            continue;
        t4 = SUC(t3);
        if (Fixed(t3, t4))
            continue;
        G2 = G1 + C(t3, t4);
        /* Try 3-opt move */
        for (Nt4 = t4->CandidateSet; (t5 = Nt4->To); Nt4++) {
            if (t5 == t4->Pred || t5 == t4->Suc ||
                (G3 = G2 - Nt4->Cost) <= 0 || !BETWEEN(t2, t5, t3))
                continue;
            t6 = SUC(t5);
            if (t6 == t1 || Fixed(t5, t6) || Forbidden(t6, t1))
                continue;
            *Gain = G3 + C(t5, t6) - C(t6, t1);
            if (*Gain > 0) {
                Make3OptMove(t1, t2, t3, t4, t5, t6, 5);
                if (Improvement(Gain, t1, t2))
                    return;
            }
        }
        /* Try special 4-opt */
        if (!Forbidden(t4, t1)) {
            G3 = G2 - C(t4, t1);
            for (Case56 = 1; Case56 <= 2; Case56++) {
                if (Case56 == 1) {
                    SelectBackward(&t6, &t5, t3, t2);
                    if (t6 == t2)
                        break;
                    t6Old = t6;
                } else {
                    SelectForward(&t5, &t6, t2, t3);
                    if (t5 == t3 || t6 == t6Old)
                        break;
                }
                for (Case78 = 1; Case78 <= 2; Case78++) {
                    if (Case78 == 1) {
                        SelectBackward(&t8, &t7, t1, t4);
                        if (t8 == t4)
                            break;
                        t7Old = t7;
                    } else {
                        SelectForward(&t7, &t8, t4, t1);
                        if (t7 == t1 || t7 == t7Old)
                            break;
                    }
                    if (!Forbidden(t6, t7) && !Forbidden(t8, t5)) {
                        G4 = G3 + C(t5, t6) + C(t7, t8);
                        *Gain = G4 - C(t6, t7) - C(t8, t5);
                        if (*Gain > 0) {
                            Swap3(t1, t2, t4, t7, t8, t5, t1, t3, t2);
                            if (Improvement(Gain, t1, t2))
                                return;
                        }
                    }
                }
            }
        }
    }
    *Gain = PenaltyGain = 0;
}

static void SelectForward(Node ** a, Node ** b, Node * from, Node * to)
{
    *a = from;
    *b = SUC(*a);
    while (*a != to && Fixed(*a, *b)) {
        *a = *b;
        *b = SUC(*a);
    }
}

static void SelectBackward(Node ** a, Node ** b, Node * from, Node * to)
{
    *a = from;
    *b = PRED(*a);
    while (*a != to && Fixed(*a, *b)) {
        *a = *b;
        *b = PRED(*a);
    }
}

