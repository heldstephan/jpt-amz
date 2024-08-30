#include "LKH.h"

/*
 * The ReadParameters function reads the name of a parameter file from
 * standard input and reads the problem parameters from this file.
 *
 * All entries of the parameter file are of the form <keyword >= <value>
 * (or <keyword><whitespace><value>), where <keyword> denotes an alphanumeric
 * keyword and <value> denotes alphanumeric or numeric data. Keywords are not
 * case sensitive.
 *
 * The order of specifications in the file is arbitrary. The following
 * specification is mandatory.
 *
 * PROBLEM_FILE = <string>
 * Specifies the name of the problem file.
 *
 * Additional control information may be supplied in the following format:
 *
 * ASCENT_CANDIDATES = <integer>
 * The number of candidate edges to be associated with each node during the
 * ascent. The candidate set is complemented such that every candidate edge
 * is associated with both its two end nodes.
 * Default: 50
 *
 * COMMENT <string>
 * A comment.
 *
 * # <string>
 * A comment.
 *
 * CTSP_TRANSFORM = { YES | NO }
 * Specifies whether the CTSP transform is used.
 * Default: NO
 *
 * DEPOT = <integer>
 * Specifies the depot node.
 * Default: 1
 *
 * EOF
 * Terminates the input data. The entry is optional.
 *
 * EXCESS = <real>
 * The maximum alpha-value allowed for any candidate edge is set to
 * EXCESS times the absolute value of the lower bound of a solution
 * tour (determined by the ascent).
 * Default: 1.0/DIMENSION
 *
 * GAIN23 = { YES | NO }
 * Specifies whether the Gain23 function is used.
 * Default: NO
 *
 * INITIAL_PERIOD = <integer>
 * The length of the first period in the ascent.
 * Default: DIMENSION/2 (but at least 100)
 *
 * KICK_TYPE = <integer>
 * Specifies the value of k for a random k-swap kick (an extension of the
 * double-bridge move). If KICK_TYPE is zero, then the LKH's special kicking
 * strategy, WALK, is used.
 * Default: 4
 *
 * KICKS = <integer>
 * Specifies the number of times to "kick" a tour found by Lin-Kernighan.
 * Each kick is a random k-swap kick-move. However, if KICKS is zero, then
 * LKH's special kicking strategy, WALK, is used.
 * Default: 1
 *
 * MAX_CANDIDATES = <integer> [ SYMMETRIC ]
 * The maximum number of candidate edges to be associated with each node.
 * The integer may be followed by the keyword SYMMETRIC, signifying
 * that the candidate set is to be complemented such that every candidate
 * edge is associated with both its two end nodes.
 * Default: 5
 *
 * MAX_SWAPS = <integer>
 * Specifies the maximum number of swaps (flips) allowed in any search
 * for a tour improvement.
 * Default: DIMENSION
 *
 * MAX_TRIALS = <integer>
 * The maximum number of trials in each run.
 * Default: DIMENSION
 *
 * PENALTY_MULTIPLIER = <integer>
 * Default: 1500
 *
 * PRECISION = <integer>
 * The internal precision in the representation of transformed distances:
 *    d[i][j] = PRECISION*c[i][j] + pi[i] + pi[j],
 * where d[i][j], c[i][j], pi[i] and pi[j] are all integral.
 * Default: 100 (which corresponds to 2 decimal places)
 *
 * RUNS = <integer>
 * The total number of runs.
 * Default: 100000
 *
 * SEED = <integer>
 * Specifies the initial seed for random number generation. If zero, the
 * seed is derived from the system clock.
 * Default: 1
 *
 * SUBGRADIENT = { YES | NO }
 * Specifies whether the Pi-values should be determined by subgradient
 * optimization.
 * Default: YES
 *
 * TIME_LIMIT = <real>
 * Specifies a time limit in seconds for each run.
 * Default: DBL_MAX
 *
 * TOUR_FILE = <string>
 * Specifies the name of a file where the best tour is to be written.
 * When a run has produced a new best tour, the tour is written to this file.
 * The character $ in the name has a special meaning. All occurrences
 * are replaced by the cost of the tour.
 *
 * TRACE_LEVEL = <integer>
 * Specifies the level of detail of the output given during the solution
 * process. The value 0 signifies a minimum amount of output. The higher
 * the value is the more information is given.
 * Default: 1
 *
 * List of abbreviations
 * ---------------------
 *
 * A string value may be abbreviated to the first few letters of the string,
 * if that abbreviation is unambiguous.
 *
 *     Value        Abbreviation
 *     NO                N
 *     SPECIAL           S
 *     SYMMETRIC         S
 *     YES               Y
 */

static char Delimiters[] = "= \n\t\r\f\v\xef\xbb\xbf";
static char *GetFileName(char *Line);
static char *ReadYesOrNo(int *V);

void ReadParameters()
{
    char *Line, *Keyword, *Token;
    unsigned int i;

    ProblemFileName = TourFileName = 0;
    AscentCandidates = 50;
    CandidateSetSymmetric = 0;
    Excess = -1;
    InitialPeriod = -1;
    Kicks = 1;
    KickType = 4;
    MaxCandidates = 1000;
    MaxTrials = -1;
    MoveType = 3;
    MoveTypeSpecial = 1;
    MTSPDepot = 1;
    PenaltyMultiplier = 1500;
    Precision = 100;
    Runs = 100000;
    Seed = 1;
    Subgradient = 1;
    TimeLimit = DBL_MAX;
    TraceLevel = 0;

    if (ParameterFileName) {
        if (!(ParameterFile = fopen(ParameterFileName, "r")))
            eprintf("Cannot open PARAMETER_FILE: \"%s\"",
                    ParameterFileName);
    } else {
        while (1) {
            printff("PARAMETER_FILE = ");
            if (!(ParameterFileName = GetFileName(ReadLine(stdin)))) {
                do {
                    printff("PROBLEM_FILE = ");
                    ProblemFileName = GetFileName(ReadLine(stdin));
                } while (!ProblemFileName);
                return;
            } else if (!(ParameterFile = fopen(ParameterFileName, "r")))
                printff("Cannot open \"%s\". Please try again.\n",
                        ParameterFileName);
            else
                break;
        }
    }
    while ((Line = ReadLine(ParameterFile))) {
        if (!(Keyword = strtok(Line, Delimiters)))
            continue;
        if (Keyword[0] == '#')
            continue;
        for (i = 0; i < strlen(Keyword); i++)
            Keyword[i] = (char) toupper(Keyword[i]);
        if (!strcmp(Keyword, "ASCENT_CANDIDATES")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &AscentCandidates))
                eprintf("ASCENT_CANDIDATES: integer expected");
            if (AscentCandidates < 2)
                eprintf("ASCENT_CANDIDATES: >= 2 expected");
        } else if (!strcmp(Keyword, "COMMENT")) {
            continue;
        } else if (!strcmp(Keyword, "CTSP_TRANSFORM")) {
            if (!ReadYesOrNo(&CTSPTransform))
                eprintf("CTSP_TRANSFORM: YES or NO expected");
        } else if (!strcmp(Keyword, "DEPOT")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &MTSPDepot))
                eprintf("DEPOT: integer expected");
            if (MTSPDepot <= 0)
                eprintf("DEPOT: positive integer expected");
        } else if (!strcmp(Keyword, "EOF")) {
            break;
        } else if (!strcmp(Keyword, "EXCESS")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%lf", &Excess))
                eprintf("EXCESS: real expected");
            if (Excess < 0)
                eprintf("EXCESS: non-negeative real expected");
        } else if (!strcmp(Keyword, "INITIAL_PERIOD")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &InitialPeriod))
                eprintf("INITIAL_PERIOD: integer expected");
            if (InitialPeriod < 0)
                eprintf("INITIAL_PERIOD: non-negative integer expected");
        } else if (!strcmp(Keyword, "KICK_TYPE")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &KickType))
                eprintf("KICK_TYPE: integer expected");
            if (KickType != 0 && KickType < 4)
                eprintf("KICK_TYPE: integer >= 4 expected");
        } else if (!strcmp(Keyword, "KICKS")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &Kicks))
                eprintf("KICKS: integer expected");
            if (Kicks < 0)
                eprintf("KICKS: non-negative integer expected");
        } else if (!strcmp(Keyword, "MAX_CANDIDATES")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &MaxCandidates))
                eprintf("MAX_CANDIDATES: integer expected");
            if (MaxCandidates < 0)
                eprintf("MAX_CANDIDATES: non-negative integer expected");
            if ((Token = strtok(0, Delimiters))) {
                for (i = 0; i < strlen(Token); i++)
                    Token[i] = (char) toupper(Token[i]);
                if (!strncmp(Token, "SYMMETRIC", strlen(Token)))
                    CandidateSetSymmetric = 1;
                else
                    eprintf
                        ("(MAX_CANDIDATES) Illegal SYMMETRIC specification");
            }
        } else if (!strcmp(Keyword, "MAX_TRIALS")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &MaxTrials))
                eprintf("MAX_TRIALS: integer expected");
            if (MaxTrials < 0)
                eprintf("MAX_TRIALS: non-negative integer expected");
        } else if (!strcmp(Keyword, "MOVE_TYPE")) {
             if (!(Token = strtok(0, Delimiters)) ||
                 !sscanf(Token, "%d", &MoveType))
                 eprintf("MOVE_TYPE: integer expected");
         } else if (!strcmp(Keyword, "PENALTY_MULTIPLIER")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &PenaltyMultiplier))
                eprintf("PENALTY_MULTIPLIER: integer expected");
        } else if (!strcmp(Keyword, "PRECISION")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &Precision))
                eprintf("PRECISION: integer expected");
        } else if (!strcmp(Keyword, "PROBLEM_FILE")) {
            if (!(ProblemFileName = GetFileName(0)))
                eprintf("PROBLEM_FILE: string expected");
        } else if (!strcmp(Keyword, "RUNS")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &Runs))
                eprintf("RUNS: integer expected");
            if (Runs <= 0)
                eprintf("RUNS: positive integer expected");
        } else if (!strcmp(Keyword, "SEED")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%u", &Seed))
                eprintf("SEED: integer expected");
        } else if (!strcmp(Keyword, "SUBGRADIENT")) {
            if (!ReadYesOrNo(&Subgradient))
                eprintf("SUBGRADIENT: YES or NO expected");
        } else if (!strcmp(Keyword, "TIME_LIMIT")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%lf", &TimeLimit))
                eprintf("TIME_LIMIT: real expected");
            if (TimeLimit < 0)
                eprintf("TIME_LIMIT: >= 0 expected");
        } else if (!strcmp(Keyword, "TOUR_FILE")) {
            if (!(TourFileName = GetFileName(0)))
                eprintf("TOUR_FILE: string expected");
        } else if (!strcmp(Keyword, "TRACE_LEVEL")) {
            if (!(Token = strtok(0, Delimiters)) ||
                !sscanf(Token, "%d", &TraceLevel))
                eprintf("TRACE_LEVEL: integer expected");
        } else
            eprintf("Unknown keyword: %s", Keyword);
        if ((Token = strtok(0, Delimiters)) && Token[0] != '#')
            eprintf("Junk at end of line: %s", Token);
    }
    if (!ProblemFileName)
        eprintf("Problem file name is missing");
    fclose(ParameterFile);
    free(LastLine);
    LastLine = 0;
}

static char *GetFileName(char *Line)
{
    char *Rest = strtok(Line, "\n\t\r\f"), *t;

    if (!Rest)
        return 0;
    while (isspace(*Rest))
        Rest++;
    if (!Line) {
        if (*Rest == '=')
            Rest++;
    }
    while (isspace(*Rest))
        Rest++;
    for (t = Rest + strlen(Rest) - 1; isspace(*t); t--)
        *t = '\0';
    if (!strlen(Rest))
        return 0;
    t = (char *) malloc(strlen(Rest) + 1);
    strcpy(t, Rest);
    return t;
}

static char *ReadYesOrNo(int *V)
{
    char *Token = strtok(0, Delimiters);

    if (Token) {
        unsigned int i;
        for (i = 0; i < strlen(Token); i++)
            Token[i] = (char) toupper(Token[i]);
        if (!strncmp(Token, "YES", strlen(Token)))
            *V = 1;
        else if (!strncmp(Token, "NO", strlen(Token)))
            *V = 0;
        else
            Token = 0;
    }
    return Token;
}
