#include "LKH.h"
#include "Segment.h"

typedef int (*OK_Function) (ZoneConstraint *Z, int *Rank, int Count);
static int OK_Neighbor(ZoneConstraint *Z, int *Rank, int Count);
static int OK_Path(ZoneConstraint *Z, int *Rank, int Count);
static int OK_Precedence(ZoneConstraint *Z, int *Rank, int Count);
static long long Zone_Penalty(ZoneConstraint *Z, OK_Function OK,
                             int *Rank, int Count, int PDelta);
#define max(a, b) ((a) > (b) ? a : b)

long long Penalty()
{
    Node *N = Depot, *LastN = 0;
    long long P = 0;
    int Forward = SUCC(N)->Id != N->Id + DimensionSaved;
    int ClusterEntrances = 0, SuperClusterEntrances = 0,
        SuperSuperClusterEntrances = 0;
    int *ZoneRank = 0, *SuperZoneRank = 0, *SuperSuperZoneRank = 0;

    if (FirstCluster) {
        ZoneRank = (int *) malloc((1 + GTSPSets) * sizeof(int));
        ZoneRank[Depot->MyCluster->Id] = ++ClusterEntrances;
    }
    if (FirstSuperCluster) {
        SuperZoneRank = (int *) malloc((1 + SuperGTSPSets) * sizeof(int));
        SuperZoneRank[Depot->MyCluster->MySuperCluster->Id] =
            ++SuperClusterEntrances;
    }
    if (FirstSuperSuperCluster) {
        SuperSuperZoneRank = 
            (int *) malloc((1 + SuperSuperGTSPSets) * sizeof(int));
        SuperSuperZoneRank[Depot->MyCluster->MySuperCluster->
            MySuperSuperCluster->Id] =
            ++SuperSuperClusterEntrances;
    }
    do {
        if (LastN && N->MyCluster != LastN->MyCluster) {
            ZoneRank[N->MyCluster->Id] = ++ClusterEntrances;
            if (N->MyCluster->MySuperCluster !=
                LastN->MyCluster->MySuperCluster) {
                SuperZoneRank[N->MyCluster->MySuperCluster->Id] =
                    ++SuperClusterEntrances;
                if (N->MyCluster->MySuperCluster->MySuperSuperCluster !=
                    LastN->MyCluster->MySuperCluster->MySuperSuperCluster)
                    SuperSuperZoneRank[N->MyCluster->
                        MySuperCluster->MySuperSuperCluster->Id] =
                        ++SuperSuperClusterEntrances;
            }
            P = 10 * max(0, ClusterEntrances - GTSPSets) +
                10 * max(0, SuperClusterEntrances - SuperGTSPSets) +
                10 * max(0, SuperSuperClusterEntrances - SuperSuperGTSPSets);
            if (P > CurrentPenalty)
                goto End_Penalty;
        }
        LastN = N;
        N = Forward ? SUCC(SUCC(N)) : PREDD(PREDD(N));
    } while (N != Depot);

    /* ZONE_NEIGHBOR */
    P += Zone_Penalty(FirstZoneNeighborConstraint, OK_Neighbor,
                      ZoneRank, ClusterEntrances, 1);
    if (P > CurrentPenalty)
        goto End_Penalty;

    /* ZONE_PATH */
    P += Zone_Penalty(FirstZonePathConstraint, OK_Path,
                      ZoneRank, ClusterEntrances, 10 * 100);
    if (P > CurrentPenalty)
        goto End_Penalty;

    /* ZONE_PRECEDENCE */
    P += Zone_Penalty(FirstZonePrecedenceConstraint, OK_Precedence,
                      ZoneRank, ClusterEntrances, 1);
    if (P > CurrentPenalty)
        goto End_Penalty;

    /* SUPER_ZONE_NEIGHBOR */
    P += Zone_Penalty(FirstSuperZoneNeighborConstraint, OK_Neighbor,
                      SuperZoneRank, SuperClusterEntrances, 10 * 100);
    if (P > CurrentPenalty)
        goto End_Penalty;

    /* SUPER_ZONE_PATH */
    P += Zone_Penalty(FirstSuperZonePathConstraint, OK_Path,
                      SuperZoneRank, SuperClusterEntrances, 10 * 100);
    if (P > CurrentPenalty)
        goto End_Penalty;

    /* SUPER_ZONE_PRECEDENCE */
    P += Zone_Penalty(FirstSuperZonePrecedenceConstraint, OK_Precedence,
                      SuperZoneRank, SuperClusterEntrances, 10 * 100);
    if (P > CurrentPenalty)
        goto End_Penalty;
    
    /* SUPER_SUPER_ZONE_NEIGHBOR */
    P += Zone_Penalty(FirstSuperSuperZoneNeighborConstraint, OK_Neighbor,
                      SuperSuperZoneRank, SuperSuperClusterEntrances, 10 * 100);
    if (P > CurrentPenalty)
        goto End_Penalty;

    /* SUPER_SUPER_ZONE_PATH */
    P += Zone_Penalty(FirstSuperSuperZonePathConstraint, OK_Path,
                      SuperSuperZoneRank, SuperSuperClusterEntrances, 10 * 100);
    if (P > CurrentPenalty)
        goto End_Penalty;

    /* SUPER_SUPER_ZONE_PRECEDENCE */
    P += Zone_Penalty(FirstSuperSuperZonePrecedenceConstraint, OK_Precedence,
                      SuperSuperZoneRank, SuperSuperClusterEntrances, 10 * 100);

End_Penalty:
    free(ZoneRank);
    free(SuperZoneRank);
    free(SuperSuperZoneRank);
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

