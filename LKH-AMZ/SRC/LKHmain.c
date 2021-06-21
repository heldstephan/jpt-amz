#include "LKH.h"

/*
 * This file contains the main function of the program.
 */

int main(int argc, char *argv[])
{
    long long Cost;
    double Time, LastTime;

    /* Read the specification of the problem */
    if (argc >= 2)
        ParameterFileName = argv[1];
    ReadParameters();
    StartTime = LastTime = GetTime();
    MaxMatrixDimension = 20000;
    ReadProblem();
    if (CTSPTransform && GTSPSets > 1) {
        /* CTSP transform */
        int i, j;
        Node *From, *To;
        MM = INT_MAX / GTSPSets / Precision;
        for (i = 1; i <= DimensionSaved; i++) {
            From = &NodeSet[i];
            for (j = 1; j <= DimensionSaved; j++) {
                if (i == j)
                    continue;
                To = &NodeSet[j];
                if (From->MyCluster != To->MyCluster)
                    From->C[j] += MM;
            }
        }
    }
    AllocateStructures();
    CreateCandidateSet();
    InitializeStatistics();

    if (Norm != 0) {
        Norm = 9999;
        BestCost = LLONG_MAX;
        BestPenalty = CurrentPenalty = LLONG_MAX;
    } else {
        /* The ascent has solved the problem! */
        BestCost = LowerBound - GTSPSets * MM;
        UpdateStatistics(BestCost, GetTime() - LastTime);
        RecordBetterTour();
        RecordBestTour();
        CurrentPenalty = LLONG_MAX;
        BestPenalty = CurrentPenalty = Penalty();
        WriteTour(TourFileName, BestTour, BestCost);
        Runs = 0;
    }

    /* Find a specified number (Runs) of local optima */

    for (Run = 1; Run <= Runs; Run++) {
        LastTime = GetTime();
        if (LastTime - StartTime >= TimeLimit) {
            if (TraceLevel >= 1)
                printff("*** Time limit exceeded ***\n");
            break;
        }
        Cost = FindTour();    /* using the Lin-Kernighan heuristic */
        Cost -= GTSPSets * MM;
        CurrentPenalty = PenaltyMultiplier * CurrentPenalty + Cost;
        if (CurrentPenalty < BestPenalty ||
            (CurrentPenalty == BestPenalty && Cost < BestCost)) {
            BestPenalty = CurrentPenalty;
            BestCost = Cost;
            RecordBetterTour();
            RecordBestTour();
        }
        Time = fabs(GetTime() - LastTime);
        UpdateStatistics(Cost, Time);
        if (TraceLevel >= 1 && Cost != LLONG_MAX) {
            printff("Run %d: ", Run);
            StatusReport(Cost, LastTime, "");
            printff("\n");
        }
        SRandom(++Seed);
    }
    CurrentPenalty = BestPenalty;
    WriteTour(TourFileName, BestTour, BestCost);
    if (TraceLevel >= 1)
        PrintStatistics();
    if (1) {
        char *Token = strtok(ProblemFileName, "/");
        Token = strtok(0, ".");
        printff("%s: Cost = %lld_%lld, "
                "Runs = %d, Time = %0.2f sec.\n",
                Token, BestPenalty, BestCost, Run - 1,
                fabs(GetTime() - StartTime));
    }
    return EXIT_SUCCESS;
}
