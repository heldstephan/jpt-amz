#include "LKH.h"

/* 
 * The IsPossibleCandidate function is used to test if an edge, (From,To),
 * may be a solution edge together with all fixed or common edges.
 *
 * If the edge is possible, the function returns 1; otherwise 0.
 */

int IsPossibleCandidate(Node * From, Node * To)
{
    ZoneConstraint *Z1, *Z2;

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
    Z1 = FirstZoneNeighborConstraint;
    while (Z1) {
        Z2 = Z1->Next;
        while (Z2) {
            int X = 0;
            if (Z1->A == Z2->A || Z1->A == Z2->B)
                X = Z1->A;
            if (Z1->B == Z2->A || Z1->B == Z2->B)
                X = Z1->B;
            if (From->MyCluster->Id == X &&
                To->MyCluster->Id != Z1->A &&
                To->MyCluster->Id != Z1->B &&
                To->MyCluster->Id != Z2->A &&
                To->MyCluster->Id != Z2->B)
                return 0;
            if (To->MyCluster->Id == X &&
                From->MyCluster->Id != Z1->A &&
                From->MyCluster->Id != Z1->B &&
                From->MyCluster->Id != Z2->A &&
                From->MyCluster->Id != Z2->B)
                return 0;
            Z2 = Z2->Next;
       }
       Z1 = Z1->Next;
    }
    Z1 = FirstSuperZoneNeighborConstraint;
    while (Z1) {
        Z2 = Z1->Next;
        while (Z2) {
            int X = 0;
            if (Z1->A == Z2->A || Z1->A == Z2->B)
                X = Z1->A;
            if (Z1->B == Z2->A || Z1->B == Z2->B)
                X = Z1->B;
            if (From->MyCluster->MySuperCluster->Id == X &&
                To->MyCluster->MySuperCluster->Id != Z1->A &&
                To->MyCluster->MySuperCluster->Id != Z1->B &&
                To->MyCluster->MySuperCluster->Id != Z2->A &&
                To->MyCluster->MySuperCluster->Id != Z2->B)
                return 0;
            if (To->MyCluster->MySuperCluster->Id == X &&
                From->MyCluster->MySuperCluster->Id != Z1->A &&
                From->MyCluster->MySuperCluster->Id != Z1->B &&
                From->MyCluster->MySuperCluster->Id != Z2->A &&
                From->MyCluster->MySuperCluster->Id != Z2->B)
                return 0;
            Z2 = Z2->Next;
       }
       Z1 = Z1->Next;
    }
    Z1 = FirstSuperSuperZoneNeighborConstraint;
    while (Z1) {
        Z2 = Z1->Next;
        while (Z2) {
            int X = 0;
            if (Z1->A == Z2->A || Z1->A == Z2->B)
                X = Z1->A;
            if (Z1->B == Z2->A || Z1->B == Z2->B)
                X = Z1->B;
            if (From->MyCluster->MySuperCluster->
                    MySuperSuperCluster->Id == X &&
                To->MyCluster->MySuperCluster->
                    MySuperSuperCluster->Id != Z1->A &&
                To->MyCluster->MySuperCluster->
                    MySuperSuperCluster->Id != Z1->B &&
                To->MyCluster->MySuperCluster->
                    MySuperSuperCluster->Id != Z2->A &&
                To->MyCluster->MySuperCluster->
                    MySuperSuperCluster->Id != Z2->B)
                return 0;
            if (To->MyCluster->MySuperCluster->
                    MySuperSuperCluster->Id == X &&
                From->MyCluster->MySuperCluster->
                    MySuperSuperCluster->Id != Z1->A &&
                From->MyCluster->MySuperCluster->
                    MySuperSuperCluster->Id != Z1->B &&
                From->MyCluster->MySuperCluster->
                    MySuperSuperCluster->Id != Z2->A &&
                From->MyCluster->MySuperCluster->
                    MySuperSuperCluster->Id != Z2->B)
                return 0;
            Z2 = Z2->Next;
       }
       Z1 = Z1->Next;
    }
    return 1;
}
