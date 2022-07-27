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
    int SavedCost;
    int Pi;     /* Pi-value of the node */
    int BestPi; /* Currently best pi-value found during the ascent */
    int Beta;   /* Beta-value (used for computing alpha-values) */
    int Sons;   /* Number of sons in the minimum spanning tree */
    int *C;     /* A row in the cost matrix */
    int Seq;    /* Sequence number in the current tour */
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
    double ServiceTime;
    int DepotId;     /* Equal to Id if the node is a depot; otherwize 0 */
    double Earliest, Latest;
    Cluster *MyCluster;
    Node *NextInCluster;
};

/* The Candidate structure is used to represent candidate edges */

struct Candidate {
    Node *To;   /* The end node of the edge */
    int Cost;   /* Cost (distance) of the edge */
    int Alpha;  /* Its alpha-value */
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

extern int AscentCandidates;   /* Number of candidate edges to be associated
                                  with each node during the ascent */
extern long long BestCost;      /* Cost of the tour in BestTour */
extern long long BestPenalty;   /* Penalty of the tour in BestTour */
extern int *BestTour;           /* Table containing best tour found */
extern long long BetterCost;    /* Cost of the tour stored in BetterTour */
extern long long BetterPenalty; /* Penalty of the tour stored in BetterTour */
extern int *BetterTour;         /* Table containing the currently best tour 
                                   in a run */
extern int *CostMatrix;         /* Cost matrix */
extern long long CurrentGain;
extern long long CurrentPenalty;
extern Node *Depot;
extern int Dimension;          /* Number of nodes in the problem */
extern int DimensionSaved;     /* Saved value of Dimension */
extern int Dim;                /* DimensionSaved - Salesmen + 1 */
extern double Excess;          /* Maximum alpha-value allowed for any 
                                  candidate edge is set to Excess times the 
                                  absolute value of the lower bound of a 
                                  solution tour */
extern Node *FirstActive, *LastActive; /* First and last node in the list 
                                          of "active" nodes */
extern Cluster *FirstCluster, *LastCluster;
extern SuperCluster *FirstSuperCluster, *LastSuperCluster;
extern SuperSuperCluster *FirstSuperSuperCluster, *LastSuperSuperCluster;
extern Node *FirstNode;        /* First node in the list of nodes */
extern int CTSPTransform; /* Specifies whether the CTSP transform is used */
extern int GTSPSets;   /* Specifies the number of clusters in a GTSP instance */
extern int SuperGTSPSets;   /* Specifies the number of super clusters */
extern int SuperSuperGTSPSets; /* Specifies the number of
                                  super super clusters */
extern int GroupSize;  /* Desired initial size of each segment */
extern int SGroupSize; /* Desired initial size of each super segment */
extern int Groups;     /* Current number of segments */
extern int SGroups;    /* Current number of super segments */
extern unsigned Hash;  /* Hash value corresponding to the current tour */
extern Node **Heap;    /* Heap used for computing minimum spanning trees */
extern HashTable *HTable;      /* Hash table used for storing tours */
extern int HashingUsed;        /* Specifies whether hashing is used */
extern int InitialPeriod;      /* Length of the first period in the ascent */
extern int KickType;   /* Specifies K for a K-swap-kick */
extern char *LastLine; /* Last input line */
extern double LowerBound;      /* Lower bound found by the ascent */
extern int M;          /* The M-value is used when solving an ATSP-
                           instance by transforming it to a STSP-instance */
extern long long MM;    /* The MM-value is used when transforming a CTSP-
                           instance to an ATSP-instance */
extern int MaxCandidates;      /* Maximum number of candidate edges to be 
                                  associated with each node */
extern int MaxMatrixDimension; /* Maximum dimension for an explicit
                                  cost matrix */
extern int MaxTrials;  /* Maximum number of trials in each run */
extern int MergingUsed;        /* Specifies whether merging is used */
extern int MoveType;   /* Specifies the sequantial move type to be used 
                          in local search. A value K >= 2 signifies 
                          that a k-opt moves are tried for k <= K */
extern int MoveTypeSpecial; /* A special (3- or 5-opt) move is used */
extern Node *NodeSet;  /* Array of all nodes */
extern int Norm;       /* Measure of a 1-tree's discrepancy from a tour */
extern long long PenaltyGain;
extern int PenaltyMultiplier;
extern int PenaltyUsed;
extern int Precision;  /* Internal precision in the representation of 
                          transformed distances */
extern unsigned *Rand; /* Table of random values */
extern short Reversed; /* Boolean used to indicate whether a tour has 
                          been reversed */
extern int Run;        /* Current run number */
extern int Runs;       /* Total number of runs */
extern unsigned Seed;  /* Initial seed for random number generation */
extern double ServiceTime;     /* Service time for a CVRP instance */
extern double StartTime;       /* Time when execution starts */
extern int Subgradient;        /* Specifies whether the Pi-values should be 
                                  determined by subgradient optimization */
extern SwapRecord *SwapStack;  /* Stack of SwapRecords */
extern int Swaps;      /* Number of swaps made during a tentative move */
extern double TimeLimit;    /* The time limit in seconds */
extern int TimeWindowsUsed;  /* Specifies whether time window are used */
extern int TraceLevel; /* Specifies the level of detail of the output 
                          given during the solution process. 
                          The value 0 signifies a minimum amount of 
                          output. The higher the value is the more 
                          information is given */
extern int Trial;      /* Ordinal number of the current trial */

extern ZoneConstraint *FirstZoneNeighborConstraint;
extern ZoneConstraint *FirstZonePathConstraint;
extern ZoneConstraint *FirstZonePrecedenceConstraint;

extern ZoneConstraint *FirstSuperZoneNeighborConstraint;
extern ZoneConstraint *FirstSuperZonePathConstraint;
extern ZoneConstraint *FirstSuperZonePrecedenceConstraint;

extern ZoneConstraint *FirstSuperSuperZoneNeighborConstraint;
extern ZoneConstraint *FirstSuperSuperZonePathConstraint;
extern ZoneConstraint *FirstSuperSuperZonePrecedenceConstraint;

/* The following variables are read by the functions ReadParameters and 
   ReadProblem: */

extern char *ParameterFileName, *ProblemFileName, *InitialTourFileName,
            *TourFileName;
extern char *Name, *Type, *EdgeWeightType, *EdgeWeightFormat;
extern int CandidateSetSymmetric, MTSPDepot,
           ProblemType, WeightType, WeightFormat;

extern FILE *ParameterFile, *ProblemFile, *InitialTourFile;
extern CostFunction Distance, D, C, c, OldDistance;

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
void AllocateStructures(void);
long long Ascent(void);
int Between(const Node * ta, const Node * tb, const Node * tc);
int Between_SL(const Node * ta, const Node * tb, const Node * tc);
void ChooseInitialTour(void);
void Connect(Node * N1, int Max, int Sparse);
void CandidateReport(void);
void CreateCandidateSet(void);
void eprintf(const char *fmt, ...);
int FixedCandidates(Node * N);
long long FindTour(void);
void Flip(Node * t1, Node * t2, Node * t3);
void FlipUpdate(void);
int Forbidden(Node * Na, Node * Nb);
char *FullName(char * Name, long long Cost);
int fscanint(FILE *f, int *v);
void GenerateCandidates(int MaxCandidates, long long MaxAlpha, int Symmetric);
double GetTime(void);
int Improvement(long long  * Gain, Node * t1, Node * SUCt1);
void InitializeStatistics(void);
int IsCandidate(const Node * ta, const Node * tb);
int IsCommonEdge(const Node * ta, const Node * tb);
int IsPossibleCandidate(Node * From, Node * To);
void KSwapKick(int K);
long long LinKernighan(void);
long long MergeTourWithBestTour(void);
long long MergeWithTour(void);
long long Minimum1TreeCost(int Sparse);
void MinimumSpanningTree(int Sparse);
void NormalizeNodeList(void);
long long Penalty(void);
long long TotalTWViolation(int *num_violation);
void PrepareKicking(void);
void printff(const char * fmt, ...);
void PrintParameters(void);
void PrintStatistics(void);
unsigned Random(void);
char *ReadLine(FILE * InputFile);
void ReadParameters(void);
void ReadProblem(void);
void ReadTour(char * FileName, FILE ** File);
void RecordBestTour(void);
void RecordBetterTour(void);
Node *RemoveFirstActive(void);
void RestoreTour(void);
void SpecialMove(Node * t1, Node * t2, long long * G0, long long * Gain);
void StatusReport(long long Cost, double EntryTime, char * Suffix);
void StoreTour(void);
void SRandom(unsigned seed);
void SymmetrizeCandidateSet(void);
void UpdateStatistics(long long Cost, double Time);
void WriteTour(char * FileName, int * Tour, long long Cost);

#endif
