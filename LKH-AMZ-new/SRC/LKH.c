#include "LKH.h"

/*
 * All global variables of the program.
 */

int AscentCandidates;   /* Number of candidate edges to be associated
                           with each node during the ascent */
long long BestCost;      /* Cost of the tour in BestTour */
long long BestPenalty;   /* Penalty of the tour in BestTour */
int *BestTour;          /* Table containing best tour found */
long long BetterCost;    /* Cost of the tour stored in BetterTour */
long long BetterPenalty; /* Penalty of the tour stored in BetterTour */
int *BetterTour;        /* Table containing the currently best tour 
                           in a run */
int CacheMask;  /* Mask for indexing the cache */
int *CacheVal;  /* Table of cached distances */
int *CacheSig;  /* Table of the signatures of cached 
                   distances */
int *CostMatrix;        /* Cost matrix */
long long CurrentGain;
long long CurrentPenalty;
Node *Depot;
int Dimension;          /* Number of nodes in the problem */
int DimensionSaved;     /* Saved value of Dimension */
int Dim;                /* DimensionSaved - Salesmen + 1 */
double Excess;          /* Maximum alpha-value allowed for any 
                           candidate edge is set to Excess times the 
                           absolute value of the lower bound of a 
                           solution tour */
Node *FirstActive, *LastActive; /* First and last node in the list 
                                   of "active" nodes */
Cluster *FirstCluster, *LastCluster;
SuperCluster *FirstSuperCluster, *LastSuperCluster;
SuperSuperCluster *FirstSuperSuperCluster, *LastSuperSuperCluster;
Node *FirstNode;        /* First node in the list of nodes */
int CTSPTransform; /* Specifies whether the CTSP transform is used */
int GTSPSets;   /* Specifies the number of clusters in a GTSP instance */
int SuperGTSPSets;   /* Specifies the number of super clusters */
int SuperSuperGTSPSets;   /* Specifies the number of super super clusters */
unsigned Hash;  /* Hash value corresponding to the current tour */
int HashingUsed; /* Specifies whether hashing is used */
Node **Heap;    /* Heap used for computing minimum spanning trees */
HashTable *HTable;      /* Hash table used for storing tours */
int InitialPeriod;      /* Length of the first period in the ascent */
int KickType;   /* Specifies K for a K-swap-kick */
char *LastLine; /* Last input line */
double LowerBound;      /* Lower bound found by the ascent */
int M;          /* The M-value is used when solving an ATSP-
                   instance by transforming it to a STSP-instance */
long long MM;    /* The MM-value is used when transforming a CTSP-
                   instance to an ATSP-instance */
int MaxCandidates;      /* Maximum number of candidate edges to be 
                           associated with each node */
int MaxMatrixDimension; /* Maximum dimension for an explicit cost matrix */
int MaxTrials;  /* Maximum number of trials in each run */
int MergingUsed;        /* Specifies whether merging is used */
int MoveType;   /* Specifies the sequantial move type to be used 
                   in local search. A value K >= 2 signifies 
                   that a k-opt moves are tried for k <= K */
int MoveTypeSpecial; /* A special (3- or 5-opt) move is used */
Node *NodeSet;  /* Array of all nodes */
int Norm;       /* Measure of a 1-tree's discrepancy from a tour */
long long PenaltyGain;
int PenaltyMultiplier;
int PenaltyUsed;
int Precision;  /* Internal precision in the representation of 
                   transformed distances */
unsigned *Rand; /* Table of random values */
short Reversed; /* Boolean used to indicate whether a tour has 
                   been reversed */
int Run;        /* Current run number */
int Runs;       /* Total number of runs */
unsigned Seed;  /* Initial seed for random number generation */
double ServiceTime;     /* Service time for a CVRP instance */
double StartTime;       /* Time when execution starts */
int Subgradient;        /* Specifies whether the Pi-values should be 
                           determined by subgradient optimization */
SwapRecord *SwapStack;  /* Stack of SwapRecords */
int Swaps;      /* Number of swaps made during a tentative move */
double TimeLimit;    /* The time limit in seconds */
int TimeWindowsUsed; /* Specifies whether time windows are used */
int TraceLevel; /* Specifies the level of detail of the output 
                   given during the solution process. 
                   The value 0 signifies a minimum amount of 
                   output. The higher the value is the more 
                   information is given */
int Trial;      /* Ordinal number of the current trial */

ZoneConstraint *FirstZoneNeighborConstraint;
ZoneConstraint *FirstZonePathConstraint;
ZoneConstraint *FirstZonePrecedenceConstraint;

ZoneConstraint *FirstSuperZoneNeighborConstraint;
ZoneConstraint *FirstSuperZonePathConstraint;
ZoneConstraint *FirstSuperZonePrecedenceConstraint;

ZoneConstraint *FirstSuperSuperZoneNeighborConstraint;
ZoneConstraint *FirstSuperSuperZonePathConstraint;
ZoneConstraint *FirstSuperSuperZonePrecedenceConstraint;

/* The following variables are read by the functions ReadParameters and
   ReadProblem: */

char *ParameterFileName, *ProblemFileName, *InitialTourFileName,
     *PiFileName, *TourFileName;
char *Name, *Type, *EdgeWeightType, *EdgeWeightFormat;
int CandidateSetSymmetric, MTSPDepot,
    ProblemType, WeightType, WeightFormat;

FILE *ParameterFile, *ProblemFile, *InitialTourFile;
CostFunction Distance, D, C, c, OldDistance;
