#ifndef _LKH_H
#define _LKH_H

/*
 * This header is used by almost all functions of the program. It defines 
 * macros and specifies data structures and function prototypes.
 */

#undef NDEBUG
#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "Hashing.h"

/* Macro definitions */

#define Fixed(a, b) ((a)->FixedTo1 == (b) || (a)->FixedTo2 == (b))
#define InBestTour(a, b) ((a)->BestSuc == (b) || (b)->BestSuc == (a))
#define InNextBestTour(a, b)\
    ((a)->NextBestSuc == (b) || (b)->NextBestSuc == (a))
#define Near(a, b)\
    ((a)->BestSuc ? InBestTour(a, b) : (a)->Dad == (b) || (b)->Dad == (a))
#define Link(a, b) { ((a)->Suc = (b))->Pred = (a); }
#define Follow(b, a)\
    { Link((b)->Pred, (b)->Suc); Link(b, b); Link(b, (a)->Suc); Link(a, b); }
#define Precede(a, b)\
    { Link((a)->Pred, (a)->Suc); Link(a, a); Link((b)->Pred, a); Link(a, b); }
#define SLink(a, b) { (a)->Suc = (b); (b)->Pred = (a); }

enum Types { TSPTW };
enum EdgeWeightTypes { EXPLICIT };
enum EdgeWeightFormats { FULL_MATRIX };
enum CandidateSetTypes { ALPHA };
enum InitialTourAlgorithms { WALK };
enum ConstraintType { AND, OR };

typedef struct Node Node;
typedef struct Candidate Candidate;
typedef struct Cluster Cluster;
typedef struct Segment Segment;
typedef struct SSegment SSegment;
typedef struct SuperCluster SuperCluster;
typedef struct SuperSuperCluster SuperSuperCluster;
typedef struct SwapRecord SwapRecord;
typedef struct ZoneConstraint ZoneConstraint;
typedef Node *(*MoveFunction) (Node * t1, Node * t2, long long * G0,
                               long long * Gain);
typedef int (*CostFunction) (Node * Na, Node * Nb);

/* The Node structure is used to represent nodes (cities) of the problem */

struct Node {
    int Id;     /* Number of the node (1...Dimension) */
    int Loc;    /* Location of the node in the heap 
                   (zero, if the node is not in the heap) */
    int Rank;   /* During the ascent, the priority of the node.
                   Otherwise, the ordinal number of the node in 
                   the tour */
    int V;      /* During the ascent the degree of the node minus 2.
                   Otherwise, the variable is used to mark nodes */
    int LastV;  /* Last value of V during the ascent */
    int Cost;   /* "Best" cost of an edge emanating from the node */
    int NextCost;  /* During the ascent, the next best cost of an edge
                      emanating from the node */
    int PredCost,  /* The costs of the neighbor edges on the current tour */ 
        SucCost; 
    int SavedCost;
    int Pi;     /* Pi-value of the node */
    int BestPi; /* Currently best pi-value found during the ascent */
    int Beta;   /* Beta-value (used for computing alpha-values) */
    int Sons;   /* Number of sons in the minimum spanning tree */
    int *C;     /* A row in the cost matrix */
    int Seq;           /* Sequence number in the current tour */
    int Degree; /* The degree of the node */
    int KickRank; /* Ranks used in for KSwapKick */
    int KickV;    /* V used in KSwapKick */
    Node *Pred, *Suc;  /* Predecessor and successor node in 
                          the two-way list of nodes */
    Node *OldPred, *OldSuc; /* Previous values of Pred and Suc */
    Node *BestSuc,     /* Best and next best successor node in the */
         *NextBestSuc; /* currently best tour */
    Node *Dad;         /* Father of the node in the minimum 1-tree */
    Node *Nearest;     /* Nearest node (used in the greedy heuristics) */
    Node *Next; /* Auxiliary pointer, usually to the next node in a list
                   of nodes (e.g., the list of "active" nodes) */
    Node *Prev; /* Auxiliary pointer, usually to the previous node 
                   in a list of nodes */
    Node *Mark; /* Visited mark */
    Node *FixedTo1,    /* Pointers to the opposite end nodes of fixed edges. */
         *FixedTo2;    /* A maximum of two fixed edges can be incident
                          to a node */
    Node *FixedTo1Saved, /* Saved values of FixedTo1 and FixedTo2 */
         *FixedTo2Saved;
    Node *SucSaved;             /* Saved pointer to successor node */
    Candidate *CandidateSet;    /* Candidate array */
    Segment *Parent;   /* Parent segment of a node when the two-level
                          tree representation is used */
    double ServiceTime;
    int DepotId;     /* Equal to Id if the node is a depot; otherwize 0 */
    double Earliest, Latest;
    Cluster *MyCluster;
    Node *NextInCluster;
    char OldPredExcluded, OldSucExcluded;  /* Booleans used for indicating 
                                              whether one (or both) of the 
                                              adjoining nodes on the old tour 
                                              has been excluded */
};

/* The Candidate structure is used to represent candidate edges */

struct Candidate {
    Node *To;   /* The end node of the edge */
    int Cost;   /* Cost (distance) of the edge */
    int Alpha;  /* Its alpha-value */
};

/* The Segment structure is used to represent the segments in the two-level 
   representation of tours */

struct Segment {
    char Reversed;       /* Reversal bit */
    Node *First, *Last;  /* First and last node in the segment */
    Segment *Pred, *Suc; /* Predecessor and successor in the two-way 
                            list of segments */
    int Rank;   /* Ordinal number of the segment in the list */
    int Size;   /* Number of nodes in the segment */
    SSegment *Parent;    /* The parent super segment */
};

struct SSegment {
    char Reversed;         /* Reversal bit */
    Segment *First, *Last; /* The first and last node in the segment */
    SSegment *Pred, *Suc;  /* The predecessor and successor in the
                              two-way list of super segments */
    int Rank;   /* The ordinal number of the segment in the list */
    int Size;   /* The number of nodes in the segment */
};

struct SuperSuperCluster {
    SuperSuperCluster *Next; 
    SuperCluster *First;
    int Id, Size;
};

struct SuperCluster {
    SuperSuperCluster *MySuperSuperCluster;
    SuperCluster *Next, *NextInSuperSuperCluster; 
    Cluster *First;
    int Id, Size, V;
};

struct Cluster {
    SuperCluster *MySuperCluster;
    Cluster *Next, *NextInSuperCluster;
    Node *First;
    int Id, Size, V;
};

struct ZoneConstraint {
    int A, B;
    ZoneConstraint *Next;
    int Type;
};

/* The SwapRecord structure is used to record 2-opt moves (swaps) */

struct SwapRecord {
    Node *t1, *t2, *t3, *t4;    /* The 4 nodes involved in a 2-opt move */
};

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
Segment *FirstSegment;  /* A pointer to the first segment in the cyclic 
                           list of segments */
SSegment *FirstSSegment;        /* A pointer to the first super segment in
                                   the cyclic list of segments */
int CTSPTransform; /* Specifies whether the CTSP transform is used */
int GTSPSets;   /* Specifies the number of clusters in a GTSP instance */
int SuperGTSPSets;   /* Specifies the number of super clusters */
int SuperSuperGTSPSets;   /* Specifies the number of super super clusters */
int GroupSize;  /* Desired initial size of each segment */
int SGroupSize; /* Desired initial size of each super segment */
int Groups;     /* Current number of segments */
int SGroups;    /* Current number of super segments */
unsigned Hash;  /* Hash value corresponding to the current tour */
Node **Heap;    /* Heap used for computing minimum spanning trees */
HashTable *HTable;      /* Hash table used for storing tours */
int InitialPeriod;      /* Length of the first period in the ascent */
int Kicks;      /* Specifies the number of K-swap-kicks */
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
int MoveType;   /* Specifies the sequantial move type to be used 
                   in local search. A value K >= 2 signifies 
                   that a k-opt moves are tried for k <= K */
int MoveTypeSpecial; /* A special (3- or 5-opt) move is used */
Node *NodeSet;  /* Array of all nodes */
int Norm;       /* Measure of a 1-tree's discrepancy from a tour */
long long PenaltyGain;
int PenaltyMultiplier;
int Precision;  /* Internal precision in the representation of 
                   transformed distances */
int PredSucCostAvailable; /* PredCost and SucCost are available */
unsigned *Rand;           /* Table of random values */
short Reversed; /* Boolean used to indicate whether a tour has 
                   been reversed */
int Run;        /* Current run number */
int Runs;       /* Total number of runs */
double ServiceTime; /* Service time for a CVRP instance */
int Serial;
unsigned Seed;  /* Initial seed for random number generation */
double StartTime;       /* Time when execution starts */
int Subgradient;        /* Specifies whether the Pi-values should be 
                           determined by subgradient optimization */
int SubproblemSize;     /* Number of nodes in a subproblem */
SwapRecord *SwapStack;  /* Stack of SwapRecords */
int Swaps;      /* Number of swaps made during a tentative move */
double TimeLimit;    /* The time limit in seconds */
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

/* Function prototypes: */

int Distance_ATSP(Node * Na, Node * Nb);
int Distance_EXPLICIT(Node * Na, Node * Nb);

int D_EXPLICIT(Node * Na, Node * Nb);
int D_FUNCTION(Node * Na, Node * Nb);

int C_EXPLICIT(Node * Na, Node * Nb);
int C_FUNCTION(Node * Na, Node * Nb);

void Activate(Node * t);
int AddCandidate(Node * From, Node * To, int Cost, int Alpha);
void AddTourCandidates(void);
void AdjustCandidateSet(void);
void AllocateSegments(void);
void AllocateStructures(void);
long long Ascent(void);
int Between(const Node * ta, const Node * tb, const Node * tc);
int Between_SL(const Node * ta, const Node * tb, const Node * tc);
int Between_SSL(const Node * ta, const Node * tb, const Node * tc);
void ChooseInitialTour(void);
void Connect(Node * N1, int Max, int Sparse);
void CandidateReport(void);
void CreateCandidateSet(void);
void eprintf(const char *fmt, ...);
int Excludable(Node * ta, Node * tb);
void Exclude(Node * ta, Node * tb);
int FixedCandidates(Node * N);
long long FindTour(void);
void Flip(Node * t1, Node * t2, Node * t3);
void Flip_SL(Node * t1, Node * t2, Node * t3);
void Flip_SSL(Node * t1, Node * t2, Node * t3);
int Forbidden(Node * Na, Node * Nb);
char *FullName(char * Name, long long Cost);
int fscanint(FILE *f, int *v);
void GenerateCandidates(int MaxCandidates, long long MaxAlpha, int Symmetric);
double GetTime(void);
long long GreedyTour(void);
int Improvement(long long  * Gain, Node * t1, Node * SUCt1);
void InitializeStatistics(void);
int IsCandidate(const Node * ta, const Node * tb);
int IsCommonEdge(const Node * ta, const Node * tb);
int IsPossibleCandidate(Node * From, Node * To);
void KSwapKick(int K);
long long LinKernighan(void);
void Make2OptMove(Node * t1, Node * t2, Node * t3, Node * t4);
void Make3OptMove(Node * t1, Node * t2, Node * t3, Node * t4, 
                  Node * t5, Node * t6, int Case);
void Make4OptMove(Node * t1, Node * t2, Node * t3, Node * t4, 
                  Node * t5, Node * t6, Node * t7, Node * t8, 
                  int Case);
void Make5OptMove(Node * t1, Node * t2, Node * t3, Node * t4, 
                  Node * t5, Node * t6, Node * t7, Node * t8,
                  Node * t9, Node * t10, int Case);
void MakeKOptMove(int K);
long long Minimum1TreeCost(int Sparse);
void MinimumSpanningTree(int Sparse);
void NormalizeNodeList(void);
void NormalizeSegmentList(void);
void OrderCandidateSet(int MaxCandidates, 
                       long long MaxAlpha, int Symmetric);
long long Penalty(void);
void PrepareKicking(void);
void printff(const char * fmt, ...);
void PrintParameters(void);
void PrintStatistics(void);
unsigned Random(void);
int ReadEdges(int MaxCandidates);
char *ReadLine(FILE * InputFile);
void ReadParameters(void);
void ReadProblem(void);
void ReadTour(char * FileName, FILE ** File);
void RecordBestTour(void);
void RecordBetterTour(void);
Node *RemoveFirstActive(void);
void ResetCandidateSet(void);
void RestoreTour(void);
int SegmentSize(Node * ta, Node * tb);
void SpecialMove(Node * t1, Node * t2, long long * G0, long long * Gain);
void StatusReport(long long Cost, double EntryTime, char * Suffix);
void StoreTour(void);
void SRandom(unsigned seed);
void SymmetrizeCandidateSet(void);
void TSPTW_Reduce(void);
void TrimCandidateSet(int MaxCandidates);
void UpdateStatistics(long long Cost, double Time);
void WriteTour(char * FileName, int * Tour, long long Cost);

#endif
