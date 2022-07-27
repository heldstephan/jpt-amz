/*****************************************************************************/
/*                                                                           */
/*                             JPT Score Function                            */
/*                                                                           */
/*  JPTscore (int ncount, int *Atour, int *Btour, int **Cost, double *score) */
/*    COMPUTES the Amazon/MIT route score(A,B)                               */
/*     -ncount is the number of nodes (including depot)                      */
/*     -Atour is an array of length ncount giving the order for tour A       */
/*     -Btour is an array of length ncount giving the order for tour B       */
/*     -Cost is an ncount by ncount matrix giving the travel costs           */
/*     -score returns the score                                              */
/*    NOTE: nodes numbers in the tour arrays are 0 to ncount-1               */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include "JPTutil.h"

static int erp_per_edit (int ncount, int *actual, int *sub, double **matrix,
    double *erp);
static void erp_per_edit_helper (int acount, int *actual, int scount, int *sub,
    double **matrix, double **totalMemo, int **countMemo, double *total,
    int *count);
static int seq_dev (int ncount, int *Atour, int *Btour, double *dev);
static int normalize_matrix (int ncount, int **M, double ***normM);

int JPTscore (int ncount, int *Atour, int *Btour, int **Cost, double *score)
{
    int rval  = 0, i, *A = (int *) NULL, *B = (int *) NULL;
    double **N = (double **) NULL, dev = 0.0, erp = 0.0;

    rval = seq_dev (ncount, Atour, Btour, &dev);
    if (rval) { fprintf(stderr, "seq_dev failed\n"); goto CLEANUP; }

    A = (int *) malloc((ncount+1)*sizeof(int));
    if (!A) {
        fprintf (stderr, "out of memory for A\n"); rval = 1; goto CLEANUP;
    }
    B = (int *) malloc((ncount+1)*sizeof(int));
    if (!B) {
        fprintf (stderr, "out of memory for B\n"); rval = 1; goto CLEANUP;
    }
    for (i = 0; i < ncount; i++) {
        A[i] = Atour[i];
        B[i] = Btour[i];
    }
    A[ncount] = Atour[0];
    B[ncount] = Btour[0];

    rval = normalize_matrix (ncount, Cost, &N);
    if (rval) { fprintf(stderr, "nomalize_matrix failed\n"); goto CLEANUP; }

    rval = erp_per_edit(ncount, A, B, N, &erp);
    if (rval) { fprintf(stderr, "erp_per_edit failed\n"); goto CLEANUP; }

    *score = dev*erp;

CLEANUP:
    if (N) {
        for (i = 0; i < ncount; i++) if (N[i]) free(N[i]);
        free (N);
    }
    if (A) free(A);
    if (B) free(B);
    return rval;
}

static int erp_per_edit (int ncount, int *actual, int *sub, double **matrix,
        double *erp)
{
    int rval = 0, count = 0, **countMemo = (int **) NULL, i, j, n = ncount+2;
    double total = 0.0, **totalMemo = (double **) NULL;

    countMemo = (int **) malloc (n * sizeof(int *));
    totalMemo = (double **) malloc (n * sizeof(double *));
    if (!countMemo || !totalMemo) {
        fprintf (stderr, "out of memory for memo matrices\n");
        rval = 1; goto CLEANUP;
    }
    for (i = 0; i < n; i++) {
        countMemo[i] = (int *) malloc (n * sizeof(int));
        totalMemo[i] = (double *) malloc (n * sizeof(double));
        if (!countMemo[i] || !totalMemo[i]) {
            fprintf (stderr, "out of memory for rows of memo matrices\n");
            rval = 1; goto CLEANUP;
        }
        for (j = 0; j < n; j++) {
            countMemo[i][j] = -1;
            totalMemo[i][j] = -1.0;
        }
    }

    erp_per_edit_helper (ncount+1, actual, ncount+1, sub, matrix,
                         totalMemo, countMemo, &total, &count);

    if (count == 0) *erp = 0.0;
    else *erp = total / (double) count;

CLEANUP:
    if (countMemo) {
        for (i = 0; i < n; i++) if (countMemo[i]) free(countMemo[i]);
        free (countMemo);
    }
    if (totalMemo) {
        for (i = 0; i < n; i++) if (totalMemo[i]) free(totalMemo[i]);
        free (totalMemo);
    }
    return rval;
}

static void erp_per_edit_helper (int acount, int *actual, int scount, int *sub,
        double **matrix, double **totalMemo, int **countMemo, double *total,
        int *count)
{
    int count1 = 0, count2 = 0, count3 = 0;
    double score1 = 0.0, score2 = 0.0, score3 = 0.0;
    double option1, option2, option3;

    if (scount > 1 && acount > 1 && countMemo[scount][acount] != -1) {
        *count = countMemo[scount][acount];
        *total = totalMemo[scount][acount];
        return;
    }

    if (scount == 0) {
        *count = acount;
        *total = 1000.0 * (double) acount;
    } else if (acount == 0) {
        *count = scount;
        *total = 1000.0 * (double) scount;
    } else {
        erp_per_edit_helper(acount-1, actual+1, scount-1, sub+1, matrix,
                            totalMemo, countMemo, &score1, &count1);
        erp_per_edit_helper(acount-1, actual+1, scount, sub, matrix,
                            totalMemo, countMemo, &score2, &count2);
        erp_per_edit_helper(acount, actual, scount-1, sub+1, matrix,
                            totalMemo, countMemo, &score3, &count3);
        option1 = score1 + matrix[actual[0]][sub[0]];
        option2 = score2 + 1000.0;
        option3 = score3 + 1000.0;
        if (option1 <= option2 && option1 <= option3) {
            *total = option1;
            if (actual[0] == sub[0]) *count = count1;
            else                     *count = count1+1;
        } else if (option2 <= option1 && option2 <= option3) {
            *total = option2;
            *count = count2+1;
        } else {
            *total = option3;
            *count = count3+1;
        }
    }

    if (scount > 1 && acount > 1) {
        countMemo[scount][acount] = *count;
        totalMemo[scount][acount] = *total;
    }
}

static int seq_dev (int ncount, int *Atour, int *Btour, double *dev)
{
    int rval = 0, i, *invperm = (int *) NULL, n = ncount-1;
    int comp_sum = 0, *comp_list = (int *) NULL;

    Atour = Atour+1;
    Btour = Btour+1;
    n = ncount-1;

    invperm = (int *) malloc(ncount*sizeof(int));
    if (!invperm) {
        fprintf (stderr, "out of memory for invperm\n"); rval=1; goto CLEANUP;
    }
    for (i = 0; i < n;  i++) invperm[Atour[i]] = i;

    comp_list = (int *) malloc(n*sizeof(int));
    if (!comp_list) {
        fprintf (stderr, "out of memory for comp_listn"); rval=1; goto CLEANUP;
    }

    for (i = 0; i < n;  i++) comp_list[i] = invperm[Btour[i]];
    for (i = 1; i < n;  i++) comp_sum += abs(comp_list[i] - comp_list[i-1]) - 1;

    *dev = (2.0/(double)(n*(n-1)))*(double)comp_sum;

CLEANUP:
    if (invperm) free(invperm);
    if (comp_list) free(comp_list);
    return rval;
}

static int normalize_matrix (int ncount, int **M, double ***normM)
{
    int rval = 0, i, j;
    double **N = (double **) NULL, sum = 0.0, mean = 0.0, std = 0.0, x = 0.0;
    double total = (double) (ncount*ncount);

    N = (double **) malloc (ncount * sizeof(double *));
    if (!N) {
        fprintf (stderr, "out of memory for N\n"); rval = 1; goto CLEANUP;
    }
    for (i = 0; i < ncount; i++) {
        N[i] = (double *) malloc (ncount * sizeof(double));
        if (!N[i]) {
            fprintf (stderr, "out of memory for N[%d]\n", i);
            rval = 1; goto CLEANUP;
        }
        for (j = 0; j < ncount; j++) {
            N[i][j] = (double) M[i][j];
            sum += N[i][j];
        }
    }

    mean = sum / total;
    for (i = 0; i < ncount; i++) {
        for (j = 0; j < ncount; j++) {
            x += pow(N[i][j] - mean, 2);
        }
    }
    std = sqrt(x/total);

    x = 1e30;
    for (i = 0; i < ncount; i++) {
        for (j = 0; j < ncount; j++) {
            N[i][j] = (N[i][j] - mean) / std;
            if (N[i][j] < x) x = N[i][j];
        }
    }

    for (i = 0; i < ncount; i++) {
        for (j = 0; j < ncount; j++) {
            N[i][j] = N[i][j] - x;
        }
    }

    *normM = N;

CLEANUP:
    return rval;
}
