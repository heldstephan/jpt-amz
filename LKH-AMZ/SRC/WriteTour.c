#include "LKH.h"

/*
 * The WriteTour function writes a tour to file. The tour
 * is written in TSPLIB format to file FileName.
 *
 * The tour is written in "normal form": starting at node 1,
 * and continuing in direction of its lowest numbered
 * neighbor.
 *
 * Nothing happens if FileName is 0.
 */

void WriteTour(char *FileName, int *Tour, long long Cost)
{
    FILE *TourFile;
    int i, j, n;
    char *FullFileName;
    time_t Now;
    long long tw_viol;
    int num_tw_viol;

    if (FileName == 0)
        return;
    FullFileName = FullName(FileName, Cost);
    Now = time(&Now);
    if (TraceLevel >= 1)
        printff("Writing%s: \"%s\" ... ",
                FileName == TourFileName ? " TOUR_FILE" : "",
                FullFileName);
    TourFile = fopen(FullFileName, "w");
    fprintf(TourFile, "NAME : %s.%lld_%lld.tour\n",
            Name, BestPenalty, Cost);
    fprintf(TourFile,
            "COMMENT : Cost = %lld_%lld\n", CurrentPenalty, Cost);
    fprintf(TourFile, "COMMENT : Found by LKH-AMZ [Keld Helsgaun] %s",
            ctime(&Now));
    tw_viol =  TotalTWViolation(&num_tw_viol);
    fprintf(TourFile, "COMMENT : TW viol.: %lld #: %d\n", tw_viol, num_tw_viol);
    fprintf(TourFile, "TYPE : TOUR\n");
    fprintf(TourFile, "DIMENSION : %d\n", DimensionSaved);
    fprintf(TourFile, "TOUR_SECTION\n");

    n = DimensionSaved;
    for (i = 1; Tour[i] != MTSPDepot; i++);
    for (j = 1; j <= n; j++) {
        fprintf(TourFile, "%d\n", Tour[i]);
        if (++i > n)
            i = 1;
    }
    fprintf(TourFile, "-1\nEOF\n");
    fclose(TourFile);
    free(FullFileName);
    if (TraceLevel >= 1)
        printff("done\n");
}

/*
 * The FullName function returns a copy of the string Name where all
 * occurrences of the character '$' have been replaced by Cost.
 */

char *FullName(char *Name, long long Cost)
{
    char *NewName = 0, *CostBuffer, *Pos;

    if (!(Pos = strstr(Name, "$"))) {
        NewName = (char *) calloc(strlen(Name) + 1, 1);
        strcpy(NewName, Name);
        return NewName;
    }
    CostBuffer = (char *) malloc(400);
    if (CurrentPenalty != 0)
        sprintf(CostBuffer, "%lld_%lld", CurrentPenalty, Cost);
    else
        sprintf(CostBuffer, "%lld", Cost);
    do {
        free(NewName);
        NewName = (char *) calloc(strlen(Name) + strlen(CostBuffer) + 1, 1);
        strncpy(NewName, Name, Pos - Name);
        strcat(NewName, CostBuffer);
        strcat(NewName, Pos + 1);
        Name = NewName;
    }
    while ((Pos = strstr(Name, "$")));
    free(CostBuffer);
    return NewName;
}
