#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    FILE *fss[2];
    const char alphabet[] = "Abcdefghijklmnopqrstuvwxyz";

    fss[0] = fopen("out.txt", "wr");
    if (!fss[0])
    {
        fprintf(stderr, "first fopen: %s\n", strerror(errno));
        exit(1);
    }
    fss[1] = fopen("out.txt", "wr");
    if (!fss[1])
    {
        fprintf(stderr, "second fopen: %s\n", strerror(errno));
        exit(1);
    }

    for (int i = 0; i < sizeof(alphabet)-1; ++i)
        fprintf(fss[i % 2], "%c", alphabet[i]);

    fclose(fss[0]);
    fclose(fss[1]);

    return 0;
}

