#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char c;
    char buff1[20];
    char buff2[20];
    int flag1, flag2;

    int fd = open("alphabet.txt", O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "%s: %s\n", "open alphabet.txt", strerror(errno));
        exit(1);
    }

    FILE *fs1 = fdopen(fd, "r");
    if (!fs1)
    {
        fprintf(stderr, "first fdopen %d: %s\n", fd, strerror(errno));
        exit(1);
    }
    setvbuf(fs1, buff1, _IOFBF, 20);
    FILE *fs2 = fdopen(fd, "r");
    if (!fs2)
    {
        fprintf(stderr, "second fdopen %d: %s\n", fd, strerror(errno));
        exit(1);
    }
    setvbuf(fs2, buff2, _IOFBF, 20);

    flag1 = flag2 = 1;
    while(flag1 == 1 || flag2 == 1)
    {
        flag1 = fscanf(fs1, "%c", &c);
        if (flag1 == 1)
            fprintf(stdout, "%c", c);

        flag2 = fscanf(fs2, "%c", &c);
        if (flag2 == 1)
            fprintf(stdout, "%c", c);
    }

    close(fd);

    return 0;
}

