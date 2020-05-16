#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char c;
    int flag1, flag2;

    int fd1 = open("alphabet.txt", O_RDONLY);
    if (fd1 == -1)
    {
        fprintf(stderr, "%s: %s\n", "first open alphabet.txt",
                strerror(errno));
        exit(1);
    }
    int fd2 = open("alphabet.txt", O_RDONLY);
    if (fd2 == -1)
    {
        fprintf(stderr, "%s: %s\n", "second open alphabet.txt",
                strerror(errno));
        exit(1);
    }

    do
    {
        if ((flag1 = read(fd1, &c, 1)))
            write(1, &c, 1);

        if ((flag2 = read(fd2, &c, 1)))
            write(1, &c, 1);
    }
    while (flag1 || flag2);

    close(fd1);
    close(fd2);

    return 0;
}

