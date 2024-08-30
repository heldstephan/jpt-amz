#include "LKH.h"

void StatusReport(long long Cost, double EntryTime, char *Suffix)
{
    printff("Cost = %lld_%lld, Time = %0.2f sec. %s\n",
            CurrentPenalty, Cost, fabs(GetTime() - EntryTime), Suffix);
}
