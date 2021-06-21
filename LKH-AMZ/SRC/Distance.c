#include "LKH.h"

/*
 * Functions for computing distances (see TSPLIB).
 *
 * The appropriate function is referenced by the function pointer Distance.
 */

int Distance_ATSP(Node * Na, Node * Nb)
{
    int n = DimensionSaved;
    if ((Na->Id <= n) == (Nb->Id <= n))
        return M;
    if (abs(Na->Id - Nb->Id) == n)
        return 0;
    return Na->Id <= n ? Na->C[Nb->Id - n] : Nb->C[Na->Id - n];
}

int Distance_EXPLICIT(Node * Na, Node * Nb)
{
    return Na->Id < Nb->Id ? Nb->C[Na->Id] : Na->C[Nb->Id];
}
