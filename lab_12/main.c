#include <stdio.h>
#include <string.h>

#include "myftw.h"

int main(int argc, char **argv)
{
    int ret;

    if (argc != 2)
    {
        fprintf(stderr, "Usage:\n    %s <input directory>\n", argv[0]);
        return 1;
    }

    ret = myftw(argv[1], myfunc);

    ntot = nreg + ndir +  nblk + nchr +  nfifo + nslink + nsock;
    if (ntot == 0)
        ntot = 1;

    char hrule[256];
    memset(hrule, '-', sizeof(hrule));

    printf("%.*s\n", 35, hrule);
    printf("Regular files: %7ld, %5.2f%%\n", nreg,   nreg   * 100.0/ntot);
    printf("Directories:   %7ld, %5.2f%%\n", ndir,   ndir   * 100.0/ntot);
    printf("Block devices: %7ld, %5.2f%%\n", nblk,   nblk   * 100.0/ntot);
    printf("Char devices:  %7ld, %5.2f%%\n", nchr,   nchr   * 100.0/ntot);
    printf("FIFOs:         %7ld, %5.2f%%\n", nfifo,  nfifo  * 100.0/ntot);
    printf("Symlinks:      %7ld, %5.2f%%\n", nslink, nslink * 100.0/ntot);
    printf("Sockets:       %7ld, %5.2f%%\n", nsock,  nsock  * 100.0/ntot);
    printf("Total:         %7ld\n", ntot);

    return ret;
}
