#include "Segment.h"
#include "LKH.h"
#include "Heap.h"

/*      
 * The AllocateStructures function allocates all necessary 
 * structures except nodes and candidates.
 */

#define Free(s) { free(s); s = 0; }

void AllocateStructures()
{
    int i;

    Free(Heap);
    Free(BestTour);
    Free(BetterTour);
    Free(HTable);
    Free(Rand);
    Free(CacheSig);
    Free(CacheVal);
    Free(SwapStack);

    HeapMake(Dimension);
    BestTour = (int *) calloc(1 + Dimension, sizeof(int));
    BetterTour = (int *) calloc(1 + Dimension, sizeof(int));
    HTable = (HashTable *) malloc(sizeof(HashTable));
    HashInitialize((HashTable *) HTable);
    SRandom(Seed);
    Rand = (unsigned *) malloc((Dimension + 1) * sizeof(unsigned));
    for (i = 1; i <= Dimension; i++)
        Rand[i] = Random();
    SRandom(Seed);
    if (WeightType != EXPLICIT) {
        for (i = 0; (1 << i) < (Dimension << 1); i++);
        i = 1 << i;
        CacheSig = (int *) calloc(i, sizeof(int));
        CacheVal = (int *) calloc(i, sizeof(int));
        CacheMask = i - 1;
    }
    AllocateSegments();
    SwapStack = (SwapRecord *)
        malloc(6 * MoveType * sizeof(SwapRecord));
}

/*      
 * The AllocateSegments function allocates the segments of the two-level tree.
 */

void AllocateSegments()
{
    Segment *S = 0, *SPrev;
    SSegment *SS = 0, *SSPrev;
    int i;

    GroupSize = (int) sqrt((double) Dimension);
    Groups = 0;
    for (i = Dimension, SPrev = 0; i > 0; i -= GroupSize, SPrev = S) {
        S = (Segment *) malloc(sizeof(Segment));
        S->Rank = ++Groups;
        if (!SPrev)
            FirstSegment = S;
        else
            SLink(SPrev, S);
    }
    SLink(S, FirstSegment);
    SGroupSize = Dimension;
    SGroups = 0;
    for (i = Groups, SSPrev = 0; i > 0; i -= SGroupSize, SSPrev = SS) {
        SS = (SSegment *) malloc(sizeof(SSegment));
        SS->Rank = ++SGroups;
        if (!SSPrev)
            FirstSSegment = SS;
        else
            SLink(SSPrev, SS);
    }
    SLink(SS, FirstSSegment);
}

