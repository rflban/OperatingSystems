#ifndef OSLAB02_MYFTW_H_
#define OSLAB02_MYFTW_H_

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define FTW_F   1 // файл, не являющийся каталогом
#define FTW_D   2 // каталог
#define FTW_DNR 3 // каталог, который не доступен для чтения
#define FTW_NS  4 // файл, о котором нельзя получить информацию

extern long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

typedef int MyFunc(const char *, const struct stat *, int, int);

static int myfunc(const char *pathname,
                  const struct stat *statptr,
                  int type, int len)
{
    for (int i = 0; i < len; i++)
            printf(" │   ");
    if (len >= 0) printf(" ├─── ");

    switch (type)
    {
        case FTW_F:
            printf("%s \n", pathname);

            switch (statptr->st_mode & S_IFMT)
            {
                case S_IFREG:
                    ++nreg;
                    break;

                case S_IFBLK:
                    ++nblk;
                    break;

                case S_IFCHR:
                    ++nchr;
                    break;

                case S_IFIFO:
                    ++nfifo;
                    break;

                case S_IFLNK:
                    ++nslink;
                    break;

                case S_IFSOCK:
                    ++nsock;
                    break;

                case S_IFDIR:
                    perror("The directory is of type FTW_F");
                    return -1;
            }

            break;

        case FTW_D:
            if (pathname[strlen(pathname) - 1] == '/')
                printf("%s\n", pathname);
            else
                printf("%s/\n", pathname);
            ++ndir;
            break;

        case FTW_DNR:
            perror("Blocked access to one of the directories");
            return -1;

        case FTW_NS:
            perror("Function error stat");
            return -1;

        default:
            perror("Unknown file type");
            return -1;
    }

    return 0;
}

int myftw(char *pathname, MyFunc *func);

#endif // OSLAB02_MYFTW_H_
