#include "Segment.h"
#include "LKH.h"

/* The Improvement function is used to check whether a done move
 * has improved the current best tour. 
 * If the tour has been improved, the function computes the penalty gain
 * and returns 1. Otherwise, the move is undone, and the function returns 0.
 */

int Improvement(long long * Gain, Node * t1, Node * SUCt1)
{
    long long NewPenalty;

    CurrentGain = *Gain;
    NewPenalty = Penalty();
    if (NewPenalty <= CurrentPenalty) {
        if (NewPenalty < CurrentPenalty || CurrentGain > 0) {
            PenaltyGain = CurrentPenalty - NewPenalty;
            FlipUpdate();
            return 1;
        }
    }
    RestoreTour();
    if (SUC(t1) != SUCt1)
        Reversed ^= 1;
    *Gain = PenaltyGain = 0;
    return 0;
}
