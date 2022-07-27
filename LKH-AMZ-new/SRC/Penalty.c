#include "LKH.h"
#include "Segment.h"

typedef int (*OK_Function) (ZoneConstraint *Z, int *Rank, int Count);
static int OK_Neighbor(ZoneConstraint *Z, int *Rank, int Count);
static int OK_Path(ZoneConstraint *Z, int *Rank, int Count);
static int OK_Precedence(ZoneConstraint *Z, int *Rank, int Count);
static long long Zone_Penalty(ZoneConstraint *Z, OK_Function OK,
                             int *Rank, int Count, int PDelta);
static inline int max(int a, int b) {
    return a > b ? a : b;
}

static int *ZoneRank = 0, *SuperZoneRank = 0, *SuperSuperZoneRank = 0;

long long Penalty()
{
    if (!PenaltyUsed || GTSPSets == 0)
        return 0;
    Node *N = Depot, *NextN = 0;
    long long P = 0, Sum = 0;
    int Forward = (SUC(N)->Id != N->Id + DimensionSaved) == !Reversed;
    int ClusterEntrances = 0, SuperClusterEntrances = 0,
        SuperSuperClusterEntrances = 0;
    Cluster *LastCluster;

    if (!ZoneRank) {
        ZoneRank = (int *) malloc((1 + GTSPSets) * sizeof(int));
        SuperZoneRank = (int *) malloc((1 + SuperGTSPSets) * sizeof(int));
        SuperSuperZoneRank = 
            (int *) malloc((1 + SuperSuperGTSPSets) * sizeof(int));
    }
    LastCluster = N->MyCluster;
    ZoneRank[N->MyCluster->Id] = ++ClusterEntrances;
    if (SuperGTSPSets) {
        SuperZoneRank[N->MyCluster->MySuperCluster->Id] =
            ++SuperClusterEntrances;
        if (SuperSuperGTSPSets)
            SuperSuperZoneRank[N->MyCluster->MySuperCluster->
                MySuperSuperCluster->Id] = ++SuperSuperClusterEntrances;
    }
    N = Forward ? N->Suc->Suc : N->Pred->Pred;
    do {
        if (N->MyCluster != LastCluster) {
            ZoneRank[N->MyCluster->Id] = ++ClusterEntrances;
            if (N->MyCluster->MySuperCluster !=
                LastCluster->MySuperCluster) {
                SuperZoneRank[N->MyCluster->MySuperCluster->Id] =
                    ++SuperClusterEntrances;
                if (N->MyCluster->MySuperCluster->MySuperSuperCluster !=
                    LastCluster->MySuperCluster->MySuperSuperCluster)
                    SuperSuperZoneRank[N->MyCluster->
                        MySuperCluster->MySuperSuperCluster->Id] =
                        ++SuperSuperClusterEntrances;
            }
            P = 10 * (max(0, ClusterEntrances - GTSPSets) +
                      max(0, SuperClusterEntrances - SuperGTSPSets) +
                      max(0, SuperSuperClusterEntrances - SuperSuperGTSPSets));
            if (P > CurrentPenalty)
                return P;
            LastCluster = N->MyCluster;
        }
        N = Forward ? N->Suc->Suc : N->Pred->Pred;
    } while (N != Depot);

    if (TimeWindowsUsed) {
        LastCluster = N->MyCluster;
        do {
            if (N->Id <= DimensionSaved) {
                if (N->MyCluster != LastCluster) {
                    LastCluster = N->MyCluster;
                    Sum -= MM;
                }
                if (Sum < N->Earliest)
                    Sum = N->Earliest;
                else if (Sum > N->Latest &&
                         (P += Sum - N->Latest) > CurrentPenalty)
                    return P;
               Sum += N->ServiceTime;
            }
            NextN = Forward ? N->Suc : N->Pred;
            Sum += (C(N, NextN) - N->Pi - NextN->Pi) / Precision;
            N = Forward ? NextN->Suc : NextN->Pred;
        } while (N != Depot);
        if (Sum > Depot->Latest) {
            P += Sum - Depot->Latest;
            if (P > CurrentPenalty)
                return P;
        }
    }

    /* ZONE_NEIGHBOR */
    P += Zone_Penalty(FirstZoneNeighborConstraint, OK_Neighbor,
                      ZoneRank, ClusterEntrances, 1);
    if (P > CurrentPenalty)
        return P;

    /* ZONE_PATH */
    P += Zone_Penalty(FirstZonePathConstraint, OK_Path,
                      ZoneRank, ClusterEntrances, 1);
    if (P > CurrentPenalty)
        return P;

    /* ZONE_PRECEDENCE */
    P += Zone_Penalty(FirstZonePrecedenceConstraint, OK_Precedence,
                      ZoneRank, ClusterEntrances, 1);
    if (P > CurrentPenalty)
        return P;

    /* SUPER_ZONE_NEIGHBOR */
    P += Zone_Penalty(FirstSuperZoneNeighborConstraint, OK_Neighbor,
                      SuperZoneRank, SuperClusterEntrances, 1000);
    if (P > CurrentPenalty)
        return P;

    /* SUPER_ZONE_PATH */
    P += Zone_Penalty(FirstSuperZonePathConstraint, OK_Path,
                      SuperZoneRank, SuperClusterEntrances, 1000);
    if (P > CurrentPenalty)
        return P;

    /* SUPER_ZONE_PRECEDENCE */
    P += Zone_Penalty(FirstSuperZonePrecedenceConstraint, OK_Precedence,
                      SuperZoneRank, SuperClusterEntrances, 1000);
    if (P > CurrentPenalty)
        return P;
    
    /* SUPER_SUPER_ZONE_NEIGHBOR */
    P += Zone_Penalty(FirstSuperSuperZoneNeighborConstraint, OK_Neighbor,
                      SuperSuperZoneRank, SuperSuperClusterEntrances, 1000);
    if (P > CurrentPenalty)
        return P;

    /* SUPER_SUPER_ZONE_PATH */
    P += Zone_Penalty(FirstSuperSuperZonePathConstraint, OK_Path,
                      SuperSuperZoneRank, SuperSuperClusterEntrances, 1000);
    if (P > CurrentPenalty)
        return P;

    /* SUPER_SUPER_ZONE_PRECEDENCE */
    P += Zone_Penalty(FirstSuperSuperZonePrecedenceConstraint, OK_Precedence,
                      SuperSuperZoneRank, SuperSuperClusterEntrances, 1000);
    return P;
}

#define InOrder(A, B, Rank, Count)\
    (Rank[A] < Count ? Rank[B] == Rank[A] + 1 : Rank[B] == 1)

static int OK_Neighbor(ZoneConstraint *Z, int *Rank, int Count)
{
    return InOrder(Z->A, Z->B, Rank, Count) ||
           InOrder(Z->B, Z->A, Rank, Count);
}

static int OK_Path(ZoneConstraint *Z, int *Rank, int Count)
{
    return InOrder(Z->A, Z->B, Rank, Count);
}

static int OK_Precedence(ZoneConstraint *Z, int *Rank, int Count)
{
    return Rank[Z->B] > Rank[Z->A];
}

static long long Zone_Penalty(ZoneConstraint *Z, OK_Function OK,
                              int *Rank, int Count, int PDelta)
{
    long long P = 0, PDelta1, PDelta2;

    while (Z) {
        if (Z->Type == OR) {
            PDelta1 = !OK(Z, Rank, Count) ? PDelta : 0;
            Z = Z->Next;
            while (Z) {
                PDelta2 = !OK(Z, Rank, Count) ? PDelta : 0;
                if (PDelta2 < PDelta1)
                    PDelta1 = PDelta2;
                if (Z->Type == OR)
                    Z = Z->Next;
                else
                    break;
            }
            P += PDelta1;
        } else if (!OK(Z, Rank, Count))
            P += PDelta;
        Z = Z->Next;
    }
    return P;
}

long long TotalTWViolation(int *num_violation)
{
  Node *N = Depot, *NextN = 0;
  long long P = 0, Sum = 0;
    int Forward = (SUC(N)->Id != N->Id + DimensionSaved) == !Reversed;

    Cluster *LastCluster;

    LastCluster = N->MyCluster;
    do {
        if (N->Id <= DimensionSaved) {
            if (N->MyCluster != LastCluster) {
                LastCluster = N->MyCluster;
                Sum -= MM;
            }
            if (Sum < N->Earliest) {
              Sum = N->Earliest ;
            }
            else if (Sum > N->Latest) {
              P += Sum - N->Latest ;
              (*num_violation)++;
            }
            Sum += N->ServiceTime;
        }
        NextN = Forward ? N->Suc : N->Pred;
        Sum += (C(N, NextN) - N->Pi - NextN->Pi) / Precision;
        N = Forward ? NextN->Suc : NextN->Pred;
    } while (N != Depot);
    if (Sum > Depot->Latest) {
        P += Sum - Depot->Latest;
        if (P > CurrentPenalty) {
            (*num_violation)++;
            return P;
        }
    }
    return P;
}

