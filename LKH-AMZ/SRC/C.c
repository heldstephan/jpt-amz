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
    return D(Na, Nb);
}

int D_FUNCTION(Node * Na, Node * Nb)
{
    return (Fixed(Na, Nb) ? 0 : Distance(Na, Nb) * Precision) + Na->Pi +
        Nb->Pi;
}
