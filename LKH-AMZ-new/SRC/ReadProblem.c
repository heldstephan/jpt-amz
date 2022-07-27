#include "LKH.h"
#include "Heap.h"

/*
 * The ReadProblem function reads the problem data in TSPLIB format from the
 * file specified in the parameter file (PROBLEM_FILE).
 *
 * The following description of the file format is extracted from the TSPLIB
 * documentation.
 *
 * The file consists of a specification part and a data part. The specification
 * part contains information on the file format and on its contents. The data
 * part contains explicit data.
 *
 * (1) The specification part
 *
 * All entries in this section are of the form <keyword> : <value>, where
 * <keyword> denotes an alphanumerical keyword and <value> denotes
 * alphanumerical or numerical data. The terms <string>, <integer> and <real>
 * denote character string, integer or real data, respectively. The order of
 * specification of the keywords in the data file is arbitrary (in principle),
 * but must be consistent, i.e., whenever a keyword is specified, all
 * necessary information for the correct interpretation of the keyword has to
 * be known.
 *
 * Below is given a list of all available keywords.
 *
 * NAME : <string>e
 * Identifies the data file.
 *
 * TYPE : <string>
 * Specifies the type of data. The only possible type is
 * TSPTW        Data for a TSP instance with time windows
 *
 * COMMENT : <string>
 * Additional comments (usually the name of the contributor or the creator of
 * the problem instance is given here).
 *
 * DIMENSION : < integer>
 * The number of nodes.
 *
 * EDGE_WEIGHT_TYPE : <string>
 * Specifies how the edge weights (or distances) are given. The only value is:
 * EXPLICIT     Weights are listed explicitly in the corresponding section
 *
 * EDGE-WEIGHT_FORMAT : <string>
 * Describes the format of the edge weights if they are given explicitly.
 * The only value is
 * FUNCTION         Weights are given by a function (see above)
 * FULL_MATRIX      Weights are given by a full matrix
 *                      (column-wise including diagonal entries)
 * GTSP_SETS : <integer>
 * Specifies the number of clusters in this instance.
 *
 * SUPER_GTSP_SETS : <integer>
 * Specifies the number of super clusters in this instance.
 *
 * SERVICE_TIME_SECTION :
 * The service times of all nodes of a CVRP are given in the form (per line)
 *
 *    <integer> <real>
 *
 * The integer specifies a node number, the real its service time.
 * The depot node must also occur in this section. Its service time is 0.
 *
 * TIME_WINDOW_SECTION :
 * Time windows are given in this section. Each line is of the form
 *
 *      <integer> <real> <realr>
 *
 * The first integer specifies a node number. The two reals specify
 * earliest and latest arrival time for the node, respectively.
 *
 * EOF
 * Terminates input data. The entry is optional.
 *
 * (2) The data part
 *
 * Depending on the choice of specifications some additional data may be
 * required. These data are given corresponding data sections following the
 * specification part. Each data section begins with the corresponding
 * keyword. The length of the sectionis either explicitly known form the
 * format specification, or the section is terminated by an appropriate
 * end-of-section identifier.
 *
 * FIXED_EDGES_SECTION :
 * In this section, edges are listed that are required to appear in each
 * solution to the problem. The edges to be fixed are given in the form
 * (per line)
 *
 *      <integer> <integer>
 *
 * meaning that the edge (arc) from the first node to the second node has
 * to be contained in a solution. This section is terminated by a -1.
 *
 * DEPOT_SECTION :
 * Contains a list of possible alternate depot nodes. This list is terminated
 * by a -1. The current implementation allows only one depot.
 *
 * GTSP_SET_SECTION :
 * This section defines which nodes belong to which clusters. This section
 * contains exactly M entries, where M is the number of clusters.
 * Each entry has the following format:
 * m v1 v2 ... vk(m) -1, where m is the cluster number (clusters are numbered
 * from 1 to M), and v1 v2 ... vk(m) are vertices comprising cluster m
 * (vertices are numbered from 1 to N).
 *
 * SUPER_GTSP_SET_SECTION :
 * This section defines which clustters belong to which super clusters.
 * This section contains exactly M entries, where M is the number of 
 * super clusters.
 * Each entry has the following format:
 * m v1 v2 ... vk(m) -1, where m is the super cluster number
 * (clusters are numbered from 1 to M), and v1 v2 ... vk(m) are the 
 * clusters comprising subser cluster m (Clusters are numbered from 1 to 
 * GTSP_SETS).
 */

static const char Delimiters[] = " :=\n\t\r\f\v\xef\xbb\xbf";
static void CheckSpecificationPart(void);
static char *Copy(char *S);
static void CreateNodes(void);
static int FixEdge(Node * Na, Node * Nb);
static void Read_DEPOT_SECTION(void);
static void Read_DIMENSION(void);
static void Read_EDGE_WEIGHT_FORMAT(void);
static void Read_EDGE_WEIGHT_SECTION(void);
static void Read_EDGE_WEIGHT_TYPE(void);
static void Read_FIXED_EDGES_SECTION(void);
static void Read_GTSP_SETS(void);
static void Read_GTSP_SET_SECTION(void);
static void Read_NAME(void);
static void Read_SERVICE_TIME(void);
static void Read_SERVICE_TIME_SECTION(void);
static void Read_SUPER_GTSP_SETS(void);
static void Read_SUPER_GTSP_SET_SECTION(void);
static void Read_SUPER_ZONE_NEIGHBOR_SECTION(void);
static void Read_SUPER_ZONE_PATH_SECTION(void);
static void Read_SUPER_ZONE_PRECEDENCE_SECTION(void);
static void Read_SUPER_SUPER_GTSP_SETS(void);
static void Read_SUPER_SUPER_GTSP_SET_SECTION(void);
static void Read_SUPER_SUPER_ZONE_NEIGHBOR_SECTION(void);
static void Read_SUPER_SUPER_ZONE_PATH_SECTION(void);
static void Read_SUPER_SUPER_ZONE_PRECEDENCE_SECTION(void);
static void Read_TIME_WINDOW_SECTION(void);
static void Read_TYPE(void);
static void Read_ZONE_NEIGHBOR_SECTION(void);
static void Read_ZONE_PATH_SECTION(void);
static void Read_ZONE_PRECEDENCE_SECTION(void);
static void Read_ZONE_SECTION(ZoneConstraint ** First, char * SectionName);

void ReadProblem()
{
    int i, j;
    char *Line, *Keyword;

    if (!(ProblemFile = fopen(ProblemFileName, "r")))
        eprintf("Cannot open PROBLEM_FILE: \"%s\"", ProblemFileName);
    if (TraceLevel >= 1)
        printff("Reading PROBLEM_FILE: \"%s\" ... ", ProblemFileName);
    FirstNode = 0;
    WeightType = WeightFormat = ProblemType = -1;
    Name = Copy("Unnamed");
    Type = EdgeWeightType = EdgeWeightFormat = 0;
    Distance = 0;
    C = 0;
    c = 0;
    while ((Line = ReadLine(ProblemFile))) {
        if (!(Keyword = strtok(Line, Delimiters)))
            continue;
        for (i = 0; i < (int) strlen(Keyword); i++)
            Keyword[i] = (char) toupper(Keyword[i]);
        if (!strcmp(Keyword, "COMMENT"));
        else if (!strcmp(Keyword, "DEPOT_SECTION"))
            Read_DEPOT_SECTION();
        else if (!strcmp(Keyword, "DIMENSION"))
            Read_DIMENSION();
        else if (!strcmp(Keyword, "EDGE_WEIGHT_FORMAT"))
            Read_EDGE_WEIGHT_FORMAT();
        else if (!strcmp(Keyword, "EDGE_WEIGHT_SECTION"))
            Read_EDGE_WEIGHT_SECTION();
        else if (!strcmp(Keyword, "EDGE_WEIGHT_TYPE"))
            Read_EDGE_WEIGHT_TYPE();
        else if (!strcmp(Keyword, "EOF"))
            break;
        else if (!strcmp(Keyword, "FIXED_EDGES_SECTION"))
            Read_FIXED_EDGES_SECTION();
        else if (!strcmp(Keyword, "GTSP_SETS"))
            Read_GTSP_SETS();
        else if (!strcmp(Keyword, "GTSP_SET_SECTION"))
            Read_GTSP_SET_SECTION();
        else if (!strcmp(Keyword, "NAME"))
            Read_NAME();
        else if (!strcmp(Keyword, "SERVICE_TIME"))
            Read_SERVICE_TIME();
        else if (!strcmp(Keyword, "SERVICE_TIME_SECTION"))
            Read_SERVICE_TIME_SECTION();
        else if (!strcmp(Keyword, "SUPER_GTSP_SETS"))
            Read_SUPER_GTSP_SETS();
        else if (!strcmp(Keyword, "SUPER_GTSP_SET_SECTION"))
            Read_SUPER_GTSP_SET_SECTION();
        else if (!strcmp(Keyword, "SUPER_SUPER_GTSP_SETS"))
            Read_SUPER_SUPER_GTSP_SETS();
        else if (!strcmp(Keyword, "SUPER_SUPER_GTSP_SET_SECTION"))
            Read_SUPER_SUPER_GTSP_SET_SECTION();
        else if (!strcmp(Keyword, "TIME_WINDOW_SECTION"))
            Read_TIME_WINDOW_SECTION();
        else if (!strcmp(Keyword, "TYPE"))
            Read_TYPE();
        else if (!strcmp(Keyword, "ZONE_NEIGHBOR_SECTION"))
            Read_ZONE_NEIGHBOR_SECTION();
        else if (!strcmp(Keyword, "ZONE_PATH_SECTION"))
            Read_ZONE_PATH_SECTION();
        else if (!strcmp(Keyword, "ZONE_PRECEDENCE_SECTION"))
            Read_ZONE_PRECEDENCE_SECTION();
        else if (!strcmp(Keyword, "SUPER_ZONE_NEIGHBOR_SECTION"))
            Read_SUPER_ZONE_NEIGHBOR_SECTION();
        else if (!strcmp(Keyword, "SUPER_ZONE_PATH_SECTION"))
            Read_SUPER_ZONE_PATH_SECTION();
        else if (!strcmp(Keyword, "SUPER_ZONE_PRECEDENCE_SECTION"))
            Read_SUPER_ZONE_PRECEDENCE_SECTION();
        else if (!strcmp(Keyword, "SUPER_SUPER_ZONE_NEIGHBOR_SECTION"))
            Read_SUPER_SUPER_ZONE_NEIGHBOR_SECTION();
        else if (!strcmp(Keyword, "SUPER_SUPER_ZONE_PATH_SECTION"))
            Read_SUPER_SUPER_ZONE_PATH_SECTION();
        else if (!strcmp(Keyword, "SUPER_SUPER_ZONE_PRECEDENCE_SECTION"))
            Read_SUPER_SUPER_ZONE_PRECEDENCE_SECTION();
        else
            eprintf("Unknown keyword: %s", Keyword);
    }
    Swaps = 0;

    /* Adjust parameters */
    if (Seed == 0)
        Seed = (unsigned) time(0);
    if (Precision == 0)
        Precision = 100;
    if (KickType > Dimension / 2)
        KickType = Dimension / 2;
    if (Runs == 0)
        Runs = 10;
    if (MaxCandidates > Dimension - 1)
        MaxCandidates = Dimension - 1;
    if (AscentCandidates > Dimension - 1)
        AscentCandidates = Dimension - 1;
    if (InitialPeriod < 0) {
        InitialPeriod = Dimension / 2;
        if (InitialPeriod < 100)
            InitialPeriod = 100;
    }
    if (Excess < 0)
        Excess = 1.0 / DimensionSaved;
    if (MaxTrials == -1)
        MaxTrials = 8 * DimensionSaved;
    HeapMake(Dimension);
    Depot = &NodeSet[MTSPDepot];
    Depot->DepotId = 1;
    for (i = Dim + 1; i <= DimensionSaved; i++)
        NodeSet[i].DepotId = i - Dim + 1;
    if (Dimension != DimensionSaved) {
        NodeSet[Depot->Id + DimensionSaved].DepotId = 1;
        for (i = Dim + 1; i <= DimensionSaved; i++)
            NodeSet[i + DimensionSaved].DepotId = i - Dim + 1;
    }
    if (CostMatrix == 0 && Dimension <= MaxMatrixDimension &&
        Distance != 0 && Distance != Distance_EXPLICIT &&
        Distance != Distance_ATSP) {
        Node *Ni, *Nj;
        CostMatrix = (int *) calloc((size_t) Dim * (Dim - 1) / 2, sizeof(int));
        Ni = FirstNode->Suc;
        do {
            Ni->C =
                &CostMatrix[(size_t) (Ni->Id - 1) * (Ni->Id - 2) / 2] - 1;
            if (Ni->Id <= Dim)
                for (Nj = FirstNode; Nj != Ni; Nj = Nj->Suc)
                    Ni->C[Nj->Id] = Fixed(Ni, Nj) ? 0 : Distance(Ni, Nj);
            else
                for (Nj = FirstNode; Nj != Ni; Nj = Nj->Suc)
                    Ni->C[Nj->Id] = 0;
        }
        while ((Ni = Ni->Suc) != FirstNode);
        c = 0;
        WeightType = EXPLICIT;
    }
    C = WeightType == EXPLICIT ? C_EXPLICIT : C_FUNCTION;
    D = WeightType == EXPLICIT ? D_EXPLICIT : D_FUNCTION;
    if (Precision > 1 && CostMatrix) {
        for (i = 2; i <= Dim; i++) {
            Node *N = &NodeSet[i];
            for (j = 1; j < i; j++)
                if (N->C[j] * Precision / Precision != N->C[j])
                    eprintf("PRECISION (= %d) is too large", Precision);
        }
    }
    if (TraceLevel >= 1) {
        printff("done\n");
        PrintParameters();
    }
    fclose(ProblemFile);
    if (InitialTourFileName)
        ReadTour(InitialTourFileName, &InitialTourFile);
    free(LastLine);
    LastLine = 0;
}

static void CheckSpecificationPart()
{
    if (ProblemType == -1)
        eprintf("TYPE is missing");
    if (Dimension < 3)
        eprintf("DIMENSION < 3 or not specified");
    if (WeightType == EXPLICIT && WeightFormat == -1 && !EdgeWeightFormat)
        eprintf("EDGE_WEIGHT_FORMAT is missing");
}

static char *Copy(char *S)
{
    char *Buffer;

    if (!S || strlen(S) == 0)
        return 0;
    Buffer = (char *) malloc(strlen(S) + 1);
    strcpy(Buffer, S);
    return Buffer;
}

static void CreateNodes()
{
    Node *Prev = 0, *N = 0;
    int i;

    if (Dimension <= 0)
        eprintf("DIMENSION is not positive (or not specified)");
    Dim = DimensionSaved;
    DimensionSaved = Dimension;
    Dimension = 2 * DimensionSaved;
    NodeSet = (Node *) calloc(Dimension + 1, sizeof(Node));
    for (i = 1; i <= Dimension; i++, Prev = N) {
        N = &NodeSet[i];
        if (i == 1)
            FirstNode = N;
        else
            Link(Prev, N);
        N->Id = i;
    }
    Link(N, FirstNode);
}

static int FixEdge(Node * Na, Node * Nb)
{
    if (!Na->FixedTo1 || Na->FixedTo1 == Nb)
        Na->FixedTo1 = Nb;
    else if (!Na->FixedTo2 || Na->FixedTo2 == Nb)
        Na->FixedTo2 = Nb;
    else
        return 0;
    if (!Nb->FixedTo1 || Nb->FixedTo1 == Na)
        Nb->FixedTo1 = Na;
    else if (!Nb->FixedTo2 || Nb->FixedTo1 == Na)
        Nb->FixedTo2 = Na;
    else
        return 0;
    return 1;
}

static void Read_NAME()
{
    if (!(Name = Copy(strtok(0, Delimiters))))
        eprintf("NAME: string expected");
}

static void Read_DEPOT_SECTION()
{
    int i;
    if (!fscanint(ProblemFile, &MTSPDepot))
        eprintf("DEPOT_SECTION: Integer expected");
    else if (MTSPDepot <= 0)
        eprintf("DEPOT_SECTION: Positive value expected");
    if (fscanint(ProblemFile, &i) && i != -1)
        eprintf("DEPOT_SECTION: Only one depot allowed");
}

static void Read_DIMENSION()
{
    char *Token = strtok(0, Delimiters);

    if (!Token || !sscanf(Token, "%d", &Dimension))
        eprintf("DIMENSION: Integer expected");
    if (Dimension < 0)
        eprintf("DIMENSION: < 0");
    DimensionSaved = Dim = Dimension;
}

static void Read_EDGE_WEIGHT_FORMAT()
{
    unsigned int i;

    if (!(EdgeWeightFormat = Copy(strtok(0, Delimiters))))
        eprintf("EDGE_WEIGHT_FORMAT: string expected");
    for (i = 0; i < strlen(EdgeWeightFormat); i++)
        EdgeWeightFormat[i] = (char) toupper(EdgeWeightFormat[i]);
    if (!strcmp(EdgeWeightFormat, "FULL_MATRIX"))
        WeightFormat = FULL_MATRIX;
    else
        eprintf("Unknown EDGE_WEIGHT_FORMAT: %s", EdgeWeightFormat);
}

static void Read_EDGE_WEIGHT_SECTION()
{
    Node *Ni;
    int i, j, n, W;

    n = Dimension;
    CheckSpecificationPart();
    if (!FirstNode)
        CreateNodes();
    n = Dimension / 2;
    CostMatrix = (int *) calloc((size_t) n * n, sizeof(int));
    for (Ni = FirstNode; Ni->Id <= n; Ni = Ni->Suc)
        Ni->C = &CostMatrix[(size_t) (Ni->Id - 1) * n] - 1;
    switch (WeightFormat) {
    case FULL_MATRIX:
        for (i = 1; i <= Dim; i++) {
            Ni = &NodeSet[i];
            for (j = 1; j <= Dim; j++) {
                if (!fscanf(ProblemFile, "%d", &W))
                    eprintf("EDGE_WEIGHT_SECTION: Missing weight");
                Ni->C[j] = W;
                if (j != i && W > M)
                    M = W;
            }
        }
        break;
    }
    for (i = 1; i <= DimensionSaved; i++)
        FixEdge(&NodeSet[i], &NodeSet[i + DimensionSaved]);
    Distance = Distance_ATSP;
    WeightType = -1;
}

static void Read_EDGE_WEIGHT_TYPE()
{
    unsigned int i;

    if (!(EdgeWeightType = Copy(strtok(0, Delimiters))))
        eprintf("EDGE_WEIGHT_TYPE: string expected");
    for (i = 0; i < strlen(EdgeWeightType); i++)
        EdgeWeightType[i] = (char) toupper(EdgeWeightType[i]);
    if (!strcmp(EdgeWeightType, "EXPLICIT")) {
        WeightType = EXPLICIT;
        Distance = Distance_EXPLICIT;
    } else
        eprintf("Unknown EDGE_WEIGHT_TYPE: %s", EdgeWeightType);
}

static void Read_FIXED_EDGES_SECTION()
{
    Node *Ni, *Nj, *N, *NPrev = 0, *NNext;
    int i, j, Count = 0;

    CheckSpecificationPart();
    if (!FirstNode)
        CreateNodes();
    if (!fscanint(ProblemFile, &i))
        i = -1;
    while (i != -1) {
        if (i <= 0 || i > Dimension / 2)
            eprintf("FIXED_EDGES_SECTION: Node number out of range: %d",
                    i);
        fscanint(ProblemFile, &j);
        if (j <= 0 || j > Dimension / 2)
            eprintf("FIXED_EDGES_SECTION: Node number out of range: %d",
                    j);
        if (i == j)
            eprintf("FIXED_EDGES_SECTION: Illegal edge: %d to %d", i, j);
        Ni = &NodeSet[i];
        Nj = &NodeSet[j + Dimension / 2];
        if (!FixEdge(Ni, Nj))
            eprintf("FIXED_EDGES_SECTION: Illegal fix: %d to %d", i, j);
        /* Cycle check */
        N = Ni;
        Count = 0;
        do {
            NNext = N->FixedTo1 != NPrev ? N->FixedTo1 : N->FixedTo2;
            NPrev = N;
            Count++;
        } while ((N = NNext) && N != Ni);
        if (N == Ni && Count != Dimension)
            eprintf("FIXED_EDGES_SECTION: Illegal fix: %d to %d", i, j);
        if (!fscanint(ProblemFile, &i))
            i = -1;
    }
}

static void Read_ZONE_SECTION(ZoneConstraint ** First, char * SectionName)
{
    int A, B;
    ZoneConstraint *Z = 0, *Last = 0;
    char *Token, *Line;
    int State = AND;

    while ((Line = ReadLine(ProblemFile))) {
        Token = strtok(Line, " ");
        if (Token[0] == '|')  {
            State = OR;
            continue;
        }
        if (Z)
            Z->Type = State;
        sscanf(Token, "%d", &A);
        if (A == -1)
            break;
        if (A < 1)
            eprintf("%s: Set number less than 1", Name);
        Token = strtok(0, " ");
        if (!sscanf(Token, "%d", &B))
            eprintf("%s: Missing set number", SectionName);
        if (B < 1)
            eprintf("%s: Set number less than 1", SectionName);
        Z = (ZoneConstraint *) malloc(sizeof(ZoneConstraint));
        Z->A = A;
        Z->B = B;
        Z->Type = State;
        Z->Next = 0;
        if (!Last)
            *First = Last = Z;
        else {
            Last->Next = Z;
            Last = Z;
        }
        State = AND;
    }
}

static void Read_ZONE_NEIGHBOR_SECTION()
{
    Read_ZONE_SECTION(&FirstZoneNeighborConstraint, "ZONE_NEIGHBOR_SECTION");
}

static void Read_ZONE_PATH_SECTION()
{
    Read_ZONE_SECTION(&FirstZonePathConstraint, "ZONE_PATH_SECTION");
}

static void Read_ZONE_PRECEDENCE_SECTION()
{
    Read_ZONE_SECTION(&FirstZonePrecedenceConstraint,
                      "ZONE_PRECEDENCE_SECTION");
}

static void Read_GTSP_SETS()
{
    char *Token = strtok(0, Delimiters);

    if (!Token || !sscanf(Token, "%d", &GTSPSets))
        eprintf("GTSP_SETS: integer expected");
    if (GTSPSets <= 0)
        eprintf("GTSP_SETS: not positive");
}

static void Read_GTSP_SET_SECTION()
{
    int Clusters = 0, n, Id;
    Cluster *Cl;
    Node *N, *Last = 0;
    char *Used;

    if (GTSPSets == 0)
        eprintf("Missing specification of GTSP_SETS");
    N = FirstNode;
    do
        N->V = 0;
    while ((N = N->Suc) != FirstNode);
    Used = (char *) calloc(GTSPSets + 1, sizeof(char));
    while (fscanf(ProblemFile, "%d", &Id) > 0) {
        if (Id < 1 || Id > GTSPSets)
            eprintf("(GTSP_SET_SECTION) Set number %d of of range", Id);
        if (Used[Id])
            eprintf("(GTSP_SET_SECTION) Set %d specified twice", Id);
        Used[Id] = 1;
        Cl = (Cluster *) calloc(1, sizeof(Cluster));
        Cl->Id = Id;
        Clusters++;
        Last = 0;
        for (;;) {
            if (fscanf(ProblemFile, "%d", &n))
                ;
            if (n == -1)
                break;
            N = &NodeSet[n];
            if (N->Id < 1 || N->Id > DimensionSaved)
                eprintf("GTSP_SET %d: Node %d outside range", n, N->Id);
            if (N->V)
                eprintf("(GTSP_SET_SECTION) Node %d occurs in two sets", N->Id);
            N->NextInCluster = 0;
            N->MyCluster = (N + DimensionSaved)->MyCluster = Cl;
            N->V = Id;
            Cl->Size++;
            if (!Cl->First)
                Cl->First = N;
            if (Last)
                Last->NextInCluster = N;
            Last = N;
        }
        if (LastCluster)
            LastCluster->Next = Cl;
        else
            FirstCluster = Cl;
        LastCluster = Cl;
        Last->NextInCluster = Cl->First;
    }
    for (Id = 1; Id <= DimensionSaved; Id++) {
        N = &NodeSet[Id];
        if (!N->V)
            eprintf("(GTSP_SET_SECTION) Node %d does not occur in any set",
                    N->Id);
    }
    if (Clusters != GTSPSets)
        eprintf("(GTSP_SET_SECTION) Missing sets");
    free(Used);
}

static void Read_SUPER_GTSP_SETS()
{
    char *Token = strtok(0, Delimiters);

    if (!Token || !sscanf(Token, "%d", &SuperGTSPSets))
        eprintf("SUPER_GTSP_SETS: integer expected");
    if (SuperGTSPSets <= 0)
        eprintf("SUPER_GTSP_SETS: not positive");
}

static void Read_SUPER_SUPER_GTSP_SETS()
{
    char *Token = strtok(0, Delimiters);

    if (!Token || !sscanf(Token, "%d", &SuperSuperGTSPSets))
        eprintf("SUPER_SUPER_GTSP_SETS: integer expected");
    if (SuperSuperGTSPSets <= 0)
        eprintf("SUPER_SUPER_GTSP_SETS: not positive");
}

static void Read_SUPER_GTSP_SET_SECTION()
{
    int SuperClusters = 0, n, Id;
    Cluster *Cl, *Last = 0;
    SuperCluster *SCl;
    char *Used;
    
    if (GTSPSets == 0)
        eprintf("Missing specification of GTSP_SETS");
    if (SuperGTSPSets == 0)
        eprintf("Missing specification of SUPER_GTSP_SETS");
    Cl = FirstCluster;
    do
        Cl->V = 0;
    while ((Cl = Cl->Next));
    Used = (char *) calloc(SuperGTSPSets + 1, sizeof(char));
    while (fscanf(ProblemFile, "%d", &Id) > 0) {
        if (Id < 1 || Id > SuperGTSPSets)
            eprintf("(SUPER_GTSP_SET_SECTION) Set number %d of of range", Id);
        if (Used[Id])
            eprintf("(SUPER_GTSP_SET_SECTION) Set %d specified twice", Id);
        Used[Id] = 1; 
        SCl = (SuperCluster *) calloc(1, sizeof(SuperCluster));
        SCl->Id = Id;
        SuperClusters++;
        Last = 0;
        for (;;) {
            if (fscanf(ProblemFile, "%d", &n))
                ;
            if (n == -1)
                break;
            Cl = FirstCluster;
            do
                if (Cl->Id == n)
                    break;
            while ((Cl = Cl->Next));
            if (!Cl)
                eprintf("SUPER_GTSP_SET_SECTION %d: "
                        "Cluster %d outside range", n);
            if (Cl->V)
                eprintf("(SUPER_GTSP_SET_SECTION) "
                        "Cluster %d occurs in two sets", Cl->Id);
            Cl->NextInSuperCluster = 0;
            Cl->MySuperCluster = SCl;
            Cl->V = Id;
            SCl->Size++;
            if (!SCl->First) 
                SCl->First = Cl;
            if (Last)
                Last->NextInSuperCluster = Cl;
            Last = Cl;
        }
        if (LastSuperCluster)
            LastSuperCluster->Next = SCl;
        else
            FirstSuperCluster = SCl;
        LastSuperCluster = SCl;
        Cl->NextInSuperCluster = SCl->First;
    }
    Cl = FirstCluster;
    do {
        if (!Cl->V)
            eprintf("(SUPER_GTSP_SET_SECTION) "
                   "Cluster %d does not occur in any set",
                    Cl->Id);
    } while ((Cl = Cl->Next));
    if (SuperClusters != SuperGTSPSets)
        eprintf("(SUPER_GTSP_SET_SECTION) Missing sets");
    free(Used);
}

static void Read_SUPER_SUPER_GTSP_SET_SECTION()
{   
    int SuperSuperClusters = 0, n, Id;
    SuperCluster *Cl, *Last = 0;
    SuperSuperCluster *SCl;
    char *Used;
    
    if (GTSPSets == 0)
        eprintf("Missing specification of GTSP_SETS");
    if (SuperGTSPSets == 0)
        eprintf("Missing specification of SUPER_GTSP_SETS");
    if (SuperSuperGTSPSets == 0)
        eprintf("Missing specification of SUPER_SUPER_GTSP_SETS");
    Cl = FirstSuperCluster;
    do  
        Cl->V = 0;
    while ((Cl = Cl->Next));
    Used = (char *) calloc(SuperSuperGTSPSets + 1, sizeof(char));
    while (fscanf(ProblemFile, "%d", &Id) > 0) {
        if (Id < 1 || Id > SuperSuperGTSPSets)
            eprintf("(SUPER_SUPER_GTSP_SET_SECTION) "
                    "Set number %d of of range", Id);
        if (Used[Id])
            eprintf("(SUPER_SUPER_GTSP_SET_SECTION) "
                    "Set %d specified twice", Id);
        Used[Id] = 1; 
        SCl = (SuperSuperCluster *) calloc(1, sizeof(SuperSuperCluster));
        SCl->Id = Id;
        SuperSuperClusters++;
        Last = 0;
        for (;;) {
            if (fscanf(ProblemFile, "%d", &n))
                ; 
            if (n == -1)
                break;
            Cl = FirstSuperCluster;
            do  
                if (Cl->Id == n)
                    break;
            while ((Cl = Cl->Next));
            if (!Cl)
                eprintf("SUPER_SUPER_GTSP_SET %d: Cluster %d outside range", n);
            if (Cl->V)
                eprintf("(SUPER_SUPER_GTSP_SET_SECTION)"
                       " Super cluster %d occurs in two sets", Cl->Id);
            Cl->NextInSuperSuperCluster = 0;
            Cl->MySuperSuperCluster = SCl;
            Cl->V = Id;
            SCl->Size++;
            if (!SCl->First)
                SCl->First = Cl;
            if (Last)
                Last->NextInSuperSuperCluster = Cl;
            Last = Cl;
        }
        if (LastSuperSuperCluster)
            LastSuperSuperCluster->Next = SCl;
        else
            FirstSuperSuperCluster = SCl;
        LastSuperSuperCluster = SCl;
        Cl->NextInSuperSuperCluster = SCl->First;
    }
    Cl = FirstSuperCluster;
    do {
        if (!Cl->V)
            eprintf("(SUPER_SUPER_GTSP_SET_SECTION)"
                   " Cluster %d does not occur in any set",
                    Cl->Id);
    } while ((Cl = Cl->Next));
    if (SuperSuperClusters != SuperSuperGTSPSets)
        eprintf("(iSUPER_SUPER_GTSP_SET_SECTION) Missing sets");
    free(Used);
}

static void Read_SUPER_ZONE_NEIGHBOR_SECTION()
{
    Read_ZONE_SECTION(&FirstSuperZoneNeighborConstraint,
                      "SUPER_ZONE_NEIGHBOR_SECTION");
}

static void Read_SUPER_ZONE_PATH_SECTION()
{
    Read_ZONE_SECTION(&FirstSuperZonePathConstraint,
                      "SUPER_ZONE_PATH_SECTION");
}

static void Read_SUPER_ZONE_PRECEDENCE_SECTION()
{
    Read_ZONE_SECTION(&FirstSuperZonePrecedenceConstraint,
                      "SUPER_ZONE_PRECEDENCE_SECTION");
}

static void Read_SUPER_SUPER_ZONE_NEIGHBOR_SECTION()
{
    Read_ZONE_SECTION(&FirstSuperSuperZoneNeighborConstraint,
                      "SUPER_SUPER_ZONE_NEIGHBOR_SECTION");
}

static void Read_SUPER_SUPER_ZONE_PATH_SECTION()
{
    Read_ZONE_SECTION(&FirstSuperSuperZonePathConstraint,
                      "SUPER_SUPER_ZONE_PATH_SECTION");
}

static void Read_SUPER_SUPER_ZONE_PRECEDENCE_SECTION()
{
    Read_ZONE_SECTION(&FirstSuperSuperZonePrecedenceConstraint,
                      "SUPER_SUPER_ZONE_PRECEDENCE_SECTION");
}

static void Read_SERVICE_TIME()
{
    char *Token = strtok(0, Delimiters);

    if (!Token || !sscanf(Token, "%lf", &ServiceTime))
        eprintf("SERVICE_TIME: Real expected");
    if (ServiceTime < 0)
        eprintf("SERVICE_TIME: < 0");
}

static void Read_SERVICE_TIME_SECTION()
{
    int Id, i;
    Node *N;

    for (i = 1; i <= Dim; i++) {
        fscanint(ProblemFile, &Id);
        if (Id <= 0 || Id > Dim)
            eprintf("SERVICE_TIME_SECTION: Node number out of range: %d",
                    Id);
        N = &NodeSet[Id];
        if (!fscanf(ProblemFile, "%lf", &N->ServiceTime))
            eprintf("SERVICE_TIME_SECTION: "
                    "Missing service time for node %d", Id);
    }
}

static void Read_TIME_WINDOW_SECTION()
{
    int Id, i;
    Node *N = FirstNode;
    do
        N->V = 0;
    while ((N = N->Suc) != FirstNode);
    for (i = 1; i <= Dim; i++) {
        if (!fscanint(ProblemFile, &Id))
            eprintf("TIME_WINDOW_SECTION: Missing nodes");
        if (Id <= 0 || Id > Dim)
            eprintf("TIME_WINDOW_SECTION: Node number out of range: %d",
                    Id);
        N = &NodeSet[Id];
        if (N->V == 1)
            eprintf("TIME_WINDOW_SECTION: Node number occurs twice: %d",
                    N->Id);
        N->V = 1;
        if (!fscanf(ProblemFile, "%lf", &N->Earliest))
            eprintf("TIME_WINDOW_SECTION: Missing earliest time");
        if (!fscanf(ProblemFile, "%lf", &N->Latest))
            eprintf("TIME_WINDOW_SECTION: Missing latest time");
        if (N->Latest == 1000000) N->Latest = INT_MAX;
        if (N->Earliest > N->Latest)
            eprintf("TIME_WINDOW_SECTION: Earliest > Latest for node %d",
                    N->Id);
    }
    N = FirstNode;
    do
        if (!N->V && N->Id <= Dim)
            break;
    while ((N = N->Suc) != FirstNode);
    if (!N->V)
        eprintf("TIME_WINDOW_SECTION: No time window given for node %d",
                N->Id);
}

static void Read_TYPE()
{
    unsigned int i;

    if (!(Type = Copy(strtok(0, Delimiters))))
        eprintf("TYPE: string expected");
    for (i = 0; i < strlen(Type); i++)
        Type[i] = (char) toupper(Type[i]);
    if (!strcmp(Type, "TSPTW"))
        ProblemType = TSPTW;
    else
        eprintf("Unknown TYPE: %s", Type);
}

/*
 The ReadTour function reads a tour from a file.
 
 The format is as follows:
 
 OPTIMUM = <real>
 Known optimal tour length. A run will be terminated as soon as a tour
 length less than or equal to optimum is achieved.
 Default: MINUS_INFINITY.
 
 TOUR_SECTION :
 A tour is specified in this section. The tour is given by a list of integers
 giving the sequence in which the nodes are visited in the tour. The tour is
 terminated by a -1.
 
 EOF
 Terminates the input data. The entry is optional.
 
 Other keywords in TSPLIB format may be included in the file, but they are
 ignored.
 */

void ReadTour(char *FileName, FILE ** File)
{
    char *Line, *Keyword, *Token;
    unsigned int i;
    int Done = 0;

    if (!(*File = fopen(FileName, "r")))
        eprintf("Cannot open tour file: \"%s\"", FileName);
    while ((Line = ReadLine(*File))) {
        if (!(Keyword = strtok(Line, Delimiters)))
            continue;
        for (i = 0; i < strlen(Keyword); i++)
            Keyword[i] = (char) toupper(Keyword[i]);
        if (!strcmp(Keyword, "COMMENT") ||
            !strcmp(Keyword, "DEMAND_SECTION") ||
            !strcmp(Keyword, "DEPOT_SECTION") ||
            !strcmp(Keyword, "EDGE_WEIGHT_FORMAT") ||
            !strcmp(Keyword, "EDGE_WEIGHT_SECTION") ||
            !strcmp(Keyword, "EDGE_WEIGHT_TYPE") ||
            !strcmp(Keyword, "FIXED_EDGES_SECTION") ||
            !strcmp(Keyword, "NAME") ||
            !strcmp(Keyword, "NODE_COORD_SECTION") ||
            !strcmp(Keyword, "NODE_COORD_TYPE") ||
            !strcmp(Keyword, "TYPE"));
        else if (strcmp(Keyword, "DIMENSION") == 0) {
            int Dim = 0;
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &Dim))
                eprintf("[%s] (DIMENSION): Integer expected", FileName);
            if (Dim != DimensionSaved && Dim != Dimension) {
                printff("Dim = %d, DimensionSaved = %d, Dimension = %d\n",
                        Dim, DimensionSaved, Dimension);
                eprintf
                    ("[%s] (DIMENSION): does not match problem dimension",
                     FileName);
            }
        } else if (!strcmp(Keyword, "EOF"))
            break;
        else
            eprintf("[%s] Unknown Keyword: %s", FileName, Keyword);
    }
    if (!Done)
        eprintf("Missing TOUR_SECTION in tour file: \"%s\"", FileName);
    fclose(*File);
}
