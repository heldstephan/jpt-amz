#include "LKH.h"

static int TrialsMin, TrialsMax, TrialSum, Successes;
static long long CostMin, CostMax, CostSum;
static long long PenaltyMin, PenaltyMax, PenaltySum;
static double TimeMin, TimeMax, TimeSum;

void InitializeStatistics()
{
    TrialSum = Successes = 0;
    CostSum = 0;
    TimeSum = 0.0;
    TrialsMin = INT_MAX;
    TrialsMax = 0;
    TimeMin = DBL_MAX;
    TimeMax = 0;
    CostMin = LLONG_MAX;
    CostMax = LLONG_MIN;
    PenaltySum = 0;
    PenaltyMin = LLONG_MAX;
    PenaltyMax = LLONG_MIN;
}

void UpdateStatistics(long long Cost, double Time)
{
    if (Trial < TrialsMin)
        TrialsMin = Trial;
    if (Trial > TrialsMax)
        TrialsMax = Trial;
    TrialSum += Trial;
    if (Cost < CostMin)
        CostMin = Cost;
    if (Cost > CostMax)
        CostMax = Cost;
    CostSum += Cost;
    if (CurrentPenalty < PenaltyMin)
        PenaltyMin = CurrentPenalty;
    if (CurrentPenalty > PenaltyMax)
        PenaltyMax = CurrentPenalty;
    PenaltySum += CurrentPenalty;
    if (Time < TimeMin)
        TimeMin = Time;
    if (Time > TimeMax)
        TimeMax = Time;
    TimeSum += Time;
}

void PrintStatistics()
{
    int _Runs = Run - 1, _TrialsMin = TrialsMin;
    double _TimeMin = TimeMin;

    printff("Successes/Runs = %d/%d \n", Successes, Runs);
    if (_Runs == 0)
        _Runs = 1;
    if (_TrialsMin > TrialsMax)
        _TrialsMin = 0;
    if (_TimeMin > TimeMax)
        _TimeMin = 0;
    if (CostMin <= CostMax && CostMin != LLONG_MAX) {
        printff("Cost.min = %lld, Cost.avg = %0.2f, "
                "Cost.max = %lld\n",
                CostMin, (double) CostSum / _Runs, CostMax);
        if (PenaltyMin != LLONG_MAX)
            printff("Penalty.min = %lld, Penalty.avg = %0.2f, "
                    "Penalty.max = %lld\n",
                    PenaltyMin, (double) PenaltySum / _Runs, PenaltyMax);
    }
    printff("Trials.min = %d, Trials.avg = %0.1f, Trials.max = %d\n",
            _TrialsMin, 1.0 * TrialSum / _Runs, TrialsMax);
    printff
        ("Time.min = %0.2f sec., Time.avg = %0.2f sec., "
         "Time.max = %0.2f sec.\n",
         fabs(_TimeMin), fabs(TimeSum) / _Runs, fabs(TimeMax));
    printff("Time.total = %0.2f sec.\n", GetTime() - StartTime);
}
