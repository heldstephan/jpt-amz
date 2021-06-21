#include "LKH.h"

/*
 * The KSwapKick function makes a random walk K-swap kick, K>=4
 * (an extension of the double-bridge kick).
 *
 * The algorithm is inspired by the thesis
 *
 *    D. Richter,
 *    Toleranzen in Helsgauns Lin-Kernighan.Heuristik fur das TSP,
 *    Diplomarbeit, Martin–Luther–Universitat Halle–Wittenberg, 2006.
 */

static Node *RandomNode();
static int compare(const void *Na, const void *Nb);

static Node *RandomWalkNode(Node *From);
static Node *LongEdgeNode();

#define WALK_STEPS 50
#define HUNT_COUNT (10 + Dimension / 1000)

#define V KickV
#define Rank KickRank

void KSwapKick(int K)
{
    Node **s, *N;
    int i;

    s = (Node **) malloc(K * sizeof(Node *));
    N = FirstNode = s[0] = LongEdgeNode();
    if (!N)
        goto End_KSwapKick;
    N->V = 1;
    for (i = 1; i < K; i++) {
        N = s[i] = RandomWalkNode(s[i - 1]);
        if (!N)
            K = i;
        else
            N->V = 1;
    }
    if (K < 4)
        goto End_KSwapKick;
    qsort(s, K, sizeof(Node *), compare);
    for (i = 0; i < K; i++)
        s[i]->OldSuc = s[i]->Suc;
    for (i = 0; i < K; i++)
        Link(s[(i + 2) % K], s[i]->OldSuc);
  End_KSwapKick:
    for (i = 0; i < K; i++)
        s[i]->V = 0;
    free(s);
}

static Node *RandomNode()
{
    Node *N;
    int Count;

    N = &NodeSet[1 + Random() % Dimension];
    Count = 0;
    while ((N->V || Fixed(N, N->Suc)) && Count < Dimension) {
        N = N->Suc;
        Count++;
    }
    return Count < Dimension ? N : 0;
}

static int compare(const void *Na, const void *Nb)
{
    return (*(Node **) Na)->Rank - (*(Node **) Nb)->Rank;
}

static Node *LongEdgeNode()
{
    Node *N, *Best = 0;
    int MaxG = INT_MIN, i, G;

    for (i = HUNT_COUNT; i > 0; i--) {
        N = RandomNode();
        if (!Fixed(N, N->Suc)) {
            if ((G = C(N, N->Suc) - N->Cost) > MaxG) {
                MaxG = G;
                Best = N;
            }
        }
    }
    return Best ? Best : RandomNode();
}

static Node *RandomWalkNode(Node *From)
{
    Node *N, *Best = 0, *Last = 0;
    int i;
    Candidate *NFrom;

    for (i = WALK_STEPS; i > 0; i--) {
        N = From->CandidateSet[Random() % From->Degree].To;
        if (N == Last)
            for (NFrom = From->CandidateSet; (N = NFrom->To); NFrom++)
                if (N != Last)
                    break;
        if (N && !N->V) {
            Last = From;
            From = N;
            if (!Fixed(N, N->Suc))
                Best = N;
        }
    }
    return Best ? Best : RandomNode();
}
