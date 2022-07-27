/****************************************************************************/
/*                                                                          */
/*                             JPT Tour Scores                              */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include <float.h>
#include <unistd.h>
#include "JPTutil.h"

static char *tspname = (char *) NULL;
static char *Aname = (char *) NULL;
static char *Bname = (char *) NULL;

static int getTSP (char *fname, int *ncount, int ***M);
static int getTour (int ncount, char *fname, int *tour);

double get_score (int ac, char **av)
{
    int rval  = 0, i, ncount = 0;
    int **M = (int **) NULL, *Atour = (int *) NULL, *Btour = (int *) NULL;
    double score = 0.0;

    Aname = av[0];
    Bname = av[1];
    tspname = av[2];

    rval = getTSP (tspname, &ncount, &M);
    if (rval) { fprintf (stderr, "getTSP failed\n"); rval = 1; goto CLEANUP; }

    Atour = (int *) malloc(ncount*sizeof(int));
    if (!Atour) {
        fprintf (stderr, "out of memory for Atour\n"); rval = 0; goto CLEANUP;
    }
    Btour = (int *) malloc(ncount*sizeof(int));
    if (!Btour) {
        fprintf (stderr, "out of memory for Btour\n"); rval = 0; goto CLEANUP;
    }
    rval = getTour (ncount, Aname, Atour);
    if (rval) { fprintf (stderr, "getTour failed\n"); rval = 1; goto CLEANUP; }
    rval = getTour (ncount, Bname, Btour);
    if (rval) { fprintf (stderr, "getTour failed\n"); rval = 1; goto CLEANUP; }

    rval = JPTscore (ncount, Atour, Btour, M, &score);
    if (rval) { fprintf (stderr, "JPTscore failed\n"); rval = 1; goto CLEANUP; }

CLEANUP:

    if (M) {
        for (i = 0; i < ncount; i++) if (M[i]) free(M[i]);
        free (M);
    }
    if (Atour) free (Atour);
    if (Btour) free (Btour);
    return score;
}

static int getTSP (char *fname, int *ncount, int ***M)
{
    char buf[256], key[256], field[256], *p;
    FILE *in = (FILE *) NULL;
    int rval = 0, n = -1, **A = (int **) NULL;

    *ncount = -1;

    if ((in = fopen (fname, "r")) == (FILE *) NULL) {
        fprintf (stderr, "Unable to open %s for input\n", fname);
        rval = 1; goto CLEANUP;
    }

    while (fgets (buf, 254, in) != (char *) NULL) {
        p = buf;
        while (*p != '\0') {
            if (*p == ':')
                *p = ' ';
            p++;
        }
        p = buf;
        if (sscanf (p, "%s", key) != EOF) {
            p += strlen (key);
            while (*p == ' ')
                p++;
            if (!strcmp (key, "DIMENSION")) {
                if (sscanf (p, "%s", field) == EOF) {
                    fprintf (stderr, "ERROR in DIMENSION line\n");
                    return 1;
                }
                n = atoi (field);
                *ncount = n;
            } else if (!strcmp (key, "EDGE_WEIGHT_SECTION")) {
                int i, j;
                if (n == -1) {
                    fprintf (stderr, "ERROR: Dimension not specified\n");
                    rval = 1; goto CLEANUP;
                }
                A = (int **) malloc (n * sizeof(int *));
                if (!A) {
                    fprintf (stderr, "out of memory for A\n");
                    rval = 1; goto CLEANUP;
                }
                for (i = 0; i < n; i++) {
                    A[i] = (int *) malloc (n * sizeof(int));
                    if (!A[i]) {
                        fprintf (stderr, "out of memory for A[%d]\n", i);
                        rval = 1; goto CLEANUP;
                    }
                    for (j = 0; j < n; j++) fscanf (in, "%d", &(A[i][j]));
                }
                *M = A;
            }
        }
    }

CLEANUP:
    if (in) fclose (in);
    return rval;
}

static int getTour (int ncount, char *fname, int *tour)
{
    int rval = 0, icount = 0;
    FILE *in = (FILE *) NULL;
    char buf[256], key[256], field[256];
    char *p;

    in = fopen (fname, "r");
    if (in == (FILE *) NULL) {
        fprintf (stderr, "Unable to open %s for input\n", fname);
        rval = 1; goto CLEANUP;
    }

    while (fgets (buf, 254, in) != (char *) NULL) {
        p = buf;
        while (*p != '\0') {
            if (*p == ':') *p = ' ';
            p++;
        }
        p = buf;
        if (sscanf (p, "%s", key) != EOF) {
            p += strlen (key);
            while (*p == ' ') p++;
            if (!strcmp (key, "TYPE")) {
                if (sscanf (p, "%s", field) == EOF || strcmp (field, "TOUR")) {
                    fprintf (stderr, "Not a TOUR File\n");
                    rval = 1; goto CLEANUP;
                }
            } else if (!strcmp (key, "DIMENSION")) {
                if (sscanf (p, "%s", field) == EOF) {
                    fprintf (stderr, "ERROR in DIMENSION line\n");
                    rval = 1; goto CLEANUP;
                }
                icount = atoi (field);
                if (icount != ncount) {
                    fprintf (stderr, "Number of nodes does not agree\n");
                    rval = 1; goto CLEANUP;
                }
            } else if (!strcmp (key, "TOUR_SECTION")) {
                int i, k;
                if (icount <= 0) {
                    fprintf (stderr, "ERROR: Dimension not specified\n");
                    rval = 1; goto CLEANUP;
                }
                for (i = 0; i < icount; i++) {
                    fscanf (in, "%d", &k);
                    if (k < 1 && k > ncount) {
                        fprintf (stderr, "ERROR: Bad format in TSPLIB tour\n");
                        rval = 1; goto CLEANUP;
                    }
                    tour[i] = k - 1;
                }
                fscanf (in, "%d", &k);
                if (k != -1) {
                    fprintf (stderr, "Warning: tour not -1 terminated\n");
                }
                break;
            }
        }
    }

CLEANUP:
    if (in) fclose (in);
    return rval;
}

int file_select(const struct dirent *entry)
{
   char *s = strrchr(entry->d_name, '.');
   return s && !strcmp(s, ".ctsptw");
}

int main(int ac, char **av)
{
    char *directory;
    int files, i, count = 0;
    double score_sum = 0;
    struct dirent **file_names;
    char *name, delim[] = ".";
    char tsp_file_name[256], A_file_name[256], B_file_name[256];
    DIR *dir;

    if (ac == 1) {
        fprintf (stderr, "Usage: %s directory\n", av[0]);
        exit(1);
    }
    directory = av[1];
    if ((dir = opendir("ACTUAL_TOURS")) == 0) {
        printf("Directory ACTUAL_TOURS does not exist\n");
        exit(1);
    }
    closedir(dir);
    files = scandir(directory, &file_names, file_select, alphasort);
    for (i = 0; i < files; i++) {
        name = strtok(file_names[i]->d_name, delim);
        if (!name)
            continue;
        strcpy(A_file_name, "ACTUAL_TOURS/");
        strcat(A_file_name, name);
        strcat(A_file_name, ".tour");
        if (access(A_file_name, F_OK))
            continue;
        strcpy(B_file_name, "TOURS-");
        strcat(B_file_name, directory);
        strcat(B_file_name, "/");
        strcat(B_file_name, name);
        strcat(B_file_name, ".tour");
        if (access(B_file_name, F_OK))
            continue;
        strcpy(tsp_file_name, directory);
        strcat(tsp_file_name, "/");
        strcat(tsp_file_name, name);
        strcat(tsp_file_name, ".ctsptw");
        if (access(tsp_file_name, F_OK))
            continue;
        char *av[3] = { A_file_name, B_file_name, tsp_file_name };
        double score = get_score(3, av);
//        printf("%s: Score = %0.5f\n", name, score);
        score_sum += score;
        free(file_names[i]);
        count++;
    }
//    printf("Tours: %d\n", count);
    if (count > 0)
        printf("Score %s: %0.5f\n", directory, score_sum / count);
    else
        printf("No score\n");
}
