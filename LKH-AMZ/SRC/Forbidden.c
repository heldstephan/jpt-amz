#include "LKH.h"

/*
 * The Forbidden function is used to test if an edge, (Na,Nb),
 * is not allowed to belong to a tour.
 * If the edge is forbidden, the function returns 1; otherwise 0.
 */

int Forbidden(Node * Na, Node * Nb)
{
    return (Na->Id <= DimensionSaved) == (Nb->Id <= DimensionSaved);
}
