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
    Free(SwapStack);

    HeapMake(Dimension);
    BestTour = (int *) calloc(1 + Dimension, sizeof(int));
    BetterTour = (int *) calloc(1 + Dimension, sizeof(int));
    SRandom(Seed);
    if (HashingUsed) {
        HTable = (HashTable *) malloc(sizeof(HashTable));
        HashInitialize((HashTable *) HTable);
        Rand = (unsigned *) malloc((Dimension + 1) * sizeof(unsigned));
        for (i = 1; i <= Dimension; i++)
            Rand[i] = Random();
    }
    SRandom(Seed);
    SwapStack = (SwapRecord *)
        malloc(6 * MoveType * sizeof(SwapRecord));
}
