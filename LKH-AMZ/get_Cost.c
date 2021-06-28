#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    FILE *in, *out;
    char line[255];
    int penalty = 0, length;

    in = fopen(argv[1], "r");
    fgets(line, 255, in);
    fgets(line, 255, in);
    if (sscanf(line, "COMMENT : Length = %d", &length) != 1)
        sscanf(line, "COMMENT : Cost = %d_%d", &penalty, &length);
    fclose(in);
    out = fopen("Penalty", "w");
    fprintf(out, "%d\n", penalty);
    fclose(out);
    out = fopen("Length", "w");
    fprintf(out, "%d\n", length);
    fclose(out);
}
